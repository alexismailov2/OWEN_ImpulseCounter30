#pragma once

#include <cstdint>
#include <string>
#include <vector>

class SerialPort;

class ModBus
{
public:
  ModBus(SerialPort& serialPort, uint8_t deviceAddress = 0x10);

  auto Function_0x01(uint16_t startRegisterAddress, uint16_t count, uint16_t timeoutMs = 1000) -> std::vector<bool>
  {
    return ReadCoilStatus(startRegisterAddress, count, timeoutMs);
  }

  auto ReadCoilStatus(uint16_t startRegisterAddress, uint16_t count, uint16_t timeoutMs = 1000) -> std::vector<bool>;

  auto Function_0x02(uint16_t startRegisterAddress, uint16_t count, uint16_t timeoutMs = 1000) -> std::vector<bool>
  {
    return ReadInputStatus(startRegisterAddress, count, timeoutMs);
  }

  auto ReadInputStatus(uint16_t startRegisterAddress, uint16_t count, uint16_t timeoutMs = 1000) -> std::vector<bool>;

  auto Function_0x03(uint16_t startRegisterAddress, uint16_t count, uint16_t timeoutMs = 1000) -> std::vector<uint16_t>
  {
    return ReadHoldingRegisters(startRegisterAddress, count, timeoutMs);
  }

  auto ReadHoldingRegisters(uint16_t startRegisterAddress, uint16_t count, uint16_t timeoutMs = 1000) -> std::vector<uint16_t>;

  auto Function_0x04(uint16_t startRegisterAddress, uint16_t count, uint8_t timeoutMs = 1000) -> std::vector<uint16_t>
  {
    return ReadInputRegisters(startRegisterAddress, count, timeoutMs);
  }

  auto ReadInputRegisters(uint16_t startRegisterAddress, uint16_t count, uint16_t timeoutMs = 1000) -> std::vector<uint16_t>;

  bool Function_0x05(uint16_t startRegisterAddress, uint16_t count, uint16_t timeoutMs = 1000)
  {
     return ForceSingleCoil(startRegisterAddress, count, timeoutMs);
  }

  bool ForceSingleCoil(uint16_t registerAddress, bool isOn, uint16_t timeoutMs = 1000);

  bool Function_0x06(uint16_t registerAddress, uint16_t value, uint16_t timeoutMs = 1000)
  {
    return WriteSingleHoldingRegister(registerAddress, value, timeoutMs);
  }

  bool WriteSingleHoldingRegister(uint16_t registerAddress, uint16_t value, uint16_t timeoutMs = 1000);

  bool Function_0x10(uint16_t startRegisterAddress, std::vector<uint16_t> values, uint16_t timeoutMs = 1000)
  {
    return WriteMultipleHoldingRegister(startRegisterAddress, values, timeoutMs);
  }

  bool WriteMultipleHoldingRegister(uint16_t startRegisterAddress, std::vector<uint16_t> values, uint16_t timeoutMs = 1000);
private:
  SerialPort& _serialPort;
  uint8_t _deviceAddress{};
};
