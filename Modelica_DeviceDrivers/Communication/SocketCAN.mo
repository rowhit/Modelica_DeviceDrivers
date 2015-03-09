within Modelica_DeviceDrivers.Communication;
class SocketCAN
  "ALPHA Feature. Support for the Linux Controller Area Network Protocol Family (aka Socket CAN)"
extends ExternalObject;
encapsulated function constructor "Open Socket / Create external object"
    import Modelica_DeviceDrivers.Communication.SocketCAN;
input String ifrName;
output SocketCAN softingCAN;

  external "C" softingCAN = MDD_socketCANConstructor(ifrName)
  annotation (IncludeDirectory="modelica://Modelica_DeviceDrivers/Resources/Include",
    Include="#include \"MDDSocketCAN.h\"",
              Library={"pthread"});
end constructor;

encapsulated function destructor "Destroy object, free resources"
import Modelica_DeviceDrivers.Communication.SocketCAN;
input SocketCAN socketCAN;

  external "C" MDD_socketCANDestructor(socketCAN)
  annotation (IncludeDirectory="modelica://Modelica_DeviceDrivers/Resources/Include",
              Include="#include \"MDDSocketCAN.h\"",
              Library={"pthread"});
end destructor;

end SocketCAN;
