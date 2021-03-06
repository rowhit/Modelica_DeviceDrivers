/** MAC address utility functions (header-only library).
 *
 * @file
 * @author      tbeu
 * @since       2018-08-29
 * @copyright see Modelica_DeviceDrivers project's License.txt file
 *
 * Little handy functions offered in the Utilities package.
 */

#ifndef MDDUTILITIESMAC_H_
#define MDDUTILITIESMAC_H_

#if !defined(ITI_COMP_SIM)

#if defined(_MSC_VER) || defined(__MINGW32__)

#ifndef WINVER
#define WINVER 0x0501
#endif
#include <winsock2.h>
#include <IPHlpApi.h>

#pragma comment( lib, "IPHlpApi.lib" )

#elif defined(__linux__) || defined(__CYGWIN__)

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netdb.h>
#include <errno.h>

#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/include/CompatibilityDefs.h"
#include "ModelicaUtilities.h"

#if defined(_MSC_VER) || defined(__MINGW32__)

DllExport const char* MDD_utilitiesGetMACAddress(int idx) {
    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 0;
    int i = 0;
    DWORD rc;

    do {
        pAddresses = (IP_ADAPTER_ADDRESSES *) malloc(outBufLen);
        if (NULL == pAddresses) {
            ModelicaError("MDDUtilitiesMAC.h: Memory allocation failed for IP_ADAPTER_ADDRESSES struct\n");
        }
        rc = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &outBufLen);
        if (ERROR_BUFFER_OVERFLOW == rc) {
            free(pAddresses);
            pAddresses = NULL;
        }
        else {
            break;
        }
        i++;
    } while (ERROR_BUFFER_OVERFLOW == rc && i < 3);

    if (NO_ERROR == rc) {
        PIP_ADAPTER_ADDRESSES pAddress = pAddresses;
        i = 1;
        while (i < idx && NULL != pAddress->Next) {
            pAddress = pAddress->Next;
            i++;
        }
        if (i == idx) {
            if (0 != pAddress->PhysicalAddressLength) {
                char* ret = ModelicaAllocateStringWithErrorReturn(3*pAddress->PhysicalAddressLength);
                if (NULL != ret) {
                    ret[0] = '\0';
                    for (i = 0; i < (int) pAddress->PhysicalAddressLength; ++i) {
                        char buf[4];
                        if (i == pAddress->PhysicalAddressLength - 1) {
                            sprintf(buf, "%.2X", (unsigned char) pAddress->PhysicalAddress[i]);
                        }
                        else {
                            sprintf(buf, "%.2X-", (unsigned char) pAddress->PhysicalAddress[i]);
                        }
                        strncat(ret, buf, 3);
                    }
                    free(pAddresses);
                    return (const char*)ret;
                }
                else {
                    free(pAddresses);
                    ModelicaError("MDDUtilitiesMAC.h: ModelicaAllocateString failed\n");
                    return "";
                }
            }
        }
        free(pAddresses);
    }
    else {
        if (pAddresses) {
            free(pAddresses);
        }
        ModelicaFormatError("MDDUtilitiesMAC.h: Call to GetAdaptersAddresses failed with error code: %lu\n", rc);
    }

    return "";
}

#elif defined(__linux__) || defined(__CYGWIN__)

const char* MDD_utilitiesGetMACAddress(int idx) {
    struct ifconf c;
    char cbuf[1024];
    int sock, rc;

    /* Create a SOCK_DGRAM socket. */
    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (-1 == sock) {
        ModelicaFormatError("MDDUtilitiesMAC.h: socket(..) failed (%s)\n", strerror(errno));
        return "";
    }

    c.ifc_len = sizeof(cbuf);
    c.ifc_buf = cbuf;
    rc = ioctl(sock, SIOCGIFCONF, &c);
    if (0 == rc) {
        struct ifreq s;
        struct ifreq* it = c.ifc_req;
        const struct ifreq* const end = it + (c.ifc_len / sizeof(struct ifreq));
        int i = 1;
        int found_idx = 0;
        for (it = c.ifc_req; it != end; ++it) {
            strcpy(s.ifr_name, it->ifr_name);
            rc = ioctl(sock, SIOCGIFFLAGS, &s);
            if (0 == rc) {
                if (!(s.ifr_flags & IFF_LOOPBACK)) { /* ignore loopback */
                    rc = ioctl(sock, SIOCGIFHWADDR, &s);
                    if (0 == rc) {
                        if (i == idx) {
                            found_idx = 1;
                            break;
                        }
                        else {
                            i++;
                        }
                    }
                }
            }
            else {
                close(sock);
                ModelicaFormatError("MDDUtilitiesMAC.h: ioctl(..) failed (%s)\n", strerror(errno));
                return "";
            }
        }
        if (1 == found_idx) {
            char* ret = ModelicaAllocateStringWithErrorReturn(180);
            if (NULL != ret) {
                ret[0] = '\0';
                for (i = 0; i < 6; ++i) {
                    char buf[4];
                    if (i == 5) {
                        snprintf(buf, 4, "%.2X", (unsigned char) s.ifr_addr.sa_data[i]);
                    }
                    else {
                        snprintf(buf, 4, "%.2X-", (unsigned char) s.ifr_addr.sa_data[i]);
                    }
                    strncat(ret, buf, 3);
                }
                close(sock);
                return (const char*)ret;
            }
            else {
                close(sock);
                ModelicaError("MDDUtilitiesMAC.h: ModelicaAllocateString failed\n");
                return "";
            }
        }
        else {
            close(sock);
        }
    }
    else {
        close(sock);
        ModelicaFormatError("MDDUtilitiesMAC.h: ioctl(..) failed (%s)\n", strerror(errno));
        return "";
    }

    return "";
}

#else

#error "Modelica_DeviceDrivers: No support of MAC address retrieval for your platform"

#endif

#endif /* !defined(ITI_COMP_SIM) */

#endif /* MDDUTILITIESMAC_H_ */
