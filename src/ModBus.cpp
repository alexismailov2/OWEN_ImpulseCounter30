#include "ModBus.hpp"

#include "SerialPort.hpp"
#include "crc16.hpp"

#ifndef DEBUG_INFO
#define DEBUG_INFO 0
#endif

#if DEBUG_INFO
#include <iostream>
#endif

ModBus::ModBus(SerialPort& serialPort, uint8_t deviceAddress)
  : _deviceAddress{deviceAddress}
  , _serialPort{serialPort}
{
}

auto ModBus::ReadCoilStatus(uint16_t startRegisterAddress, uint16_t count, uint16_t timeoutMs) -> std::vector<bool>
{
  // TODO: Same as 02 Should be moved to dedicated function
  std::string request{static_cast<char>(_deviceAddress),
                      0x01,
                      static_cast<char>(startRegisterAddress >> 8),
                      static_cast<char>(startRegisterAddress & 0xFF),
                      static_cast<char>(count >> 8),
                      static_cast<char>(count & 0xFF)};
  auto crc16 = Crc16(reinterpret_cast<uint8_t const*>(request.data()), request.size());
  request += static_cast<char>(crc16 >> 8);
  request += static_cast<char>(crc16 & 0xFF);
#if DEBUG_INFO
  for (auto byte : request)
  {
    std::cout << std::hex << " 0x" << static_cast<uint32_t>(byte);
  }
  std::cout << std::endl;
#endif

  std::vector<bool> requestedInputStatus{};
  _serialPort.SendCommand(request, [&](std::string const& response, bool error) {
#if DEBUG_INFO
    for (auto byte : response)
    {
      std::cout << std::hex << " 0x" << static_cast<uint32_t>(byte);
    }
    std::cout << std::endl;
#endif
    if (error ||
        (response.size() < (5 + count/8)) ||
        (response[0] != static_cast<char>(_deviceAddress)) ||
        (response[1] != 0x01) ||
        (response[2] != count/8) ||
        (Crc16(reinterpret_cast<uint8_t const*>(response.data()), response.size() - 2) != ((((uint8_t)*std::prev(response.cend(), 2)) << 8) | ((uint8_t)response.back()))))
    {
      return;
    }
    requestedInputStatus.reserve(count);
    auto currentRegIt = std::next(response.cbegin(), 3);
    auto endRegIt = std::prev(response.cend(), 2);
    while ((currentRegIt != endRegIt) && (count != 0))
    {
      auto bits = 0;
      while ((count-- != 0) && (bits++ < 16))
      {
        requestedInputStatus.emplace_back(*currentRegIt & (1 << bits));
      }
      currentRegIt += 2;
    }
  }, timeoutMs);
  return requestedInputStatus;
}

auto ModBus::ReadInputStatus(uint16_t startRegisterAddress, uint16_t count, uint16_t timeoutMs) -> std::vector<bool>
{
  // TODO: Same as 01 Should be moved to dedicated function
  std::string request{static_cast<char>(_deviceAddress),
                      0x02,
                      static_cast<char>(startRegisterAddress >> 8),
                      static_cast<char>(startRegisterAddress & 0xFF),
                      static_cast<char>(count >> 8),
                      static_cast<char>(count & 0xFF)};
  auto crc16 = Crc16(reinterpret_cast<uint8_t const*>(request.data()), request.size());
  request += static_cast<char>(crc16 >> 8);
  request += static_cast<char>(crc16 & 0xFF);
#if DEBUG_INFO
  for (auto byte : request)
  {
    std::cout << std::hex << " 0x" << static_cast<uint32_t>(byte);
  }
  std::cout << std::endl;
#endif

  std::vector<bool> requestedInputStatus{};
  _serialPort.SendCommand(request, [&](std::string const& response, bool error) {
#if DEBUG_INFO
    for (auto byte : response)
    {
      std::cout << std::hex << " 0x" << static_cast<uint32_t>(byte);
    }
    std::cout << std::endl;
#endif
    if (error ||
        (response.size() < (5 + count/8)) ||
        (response[0] != static_cast<char>(_deviceAddress)) ||
        (response[1] != 0x02) ||
        (response[2] != count/8) ||
        (Crc16(reinterpret_cast<uint8_t const*>(response.data()), response.size() - 2) != ((((uint8_t)*std::prev(response.cend(), 2)) << 8) | ((uint8_t)response.back()))))
    {
      return;
    }
    requestedInputStatus.reserve(count);
    auto currentRegIt = std::next(response.cbegin(), 3);
    auto endRegIt = std::prev(response.cend(), 2);
    while ((currentRegIt != endRegIt) && (count != 0))
    {
      auto bits = 0;
      while ((count-- != 0) && (bits++ < 16))
      {
        requestedInputStatus.emplace_back(*currentRegIt & (1 << bits));
      }
      currentRegIt += 2;
    }
  }, timeoutMs);
  return requestedInputStatus;
}

auto ModBus::ReadHoldingRegisters(uint16_t startRegisterAddress, uint16_t count, uint16_t timeoutMs) -> std::vector<uint16_t>
{
  std::string request{static_cast<char>(_deviceAddress),
                      0x03,
                      static_cast<char>(startRegisterAddress >> 8),
                      static_cast<char>(startRegisterAddress & 0xFF),
                      static_cast<char>(count >> 8),
                      static_cast<char>(count & 0xFF)};
  auto crc16 = Crc16(reinterpret_cast<uint8_t const*>(request.data()), request.size());
  request += static_cast<char>(crc16 >> 8);
  request += static_cast<char>(crc16 & 0xFF);
#if DEBUG_INFO
  for (auto byte : request)
  {
    std::cout << std::hex << " 0x" << static_cast<uint32_t>(byte);
  }
  std::cout << std::endl;
#endif

  std::vector<uint16_t> requestedRegisters{};
  _serialPort.SendCommand(request, [&](std::string const& response, bool error) {
#if DEBUG_INFO
    for (auto byte : response)
    {
      std::cout << std::hex << " 0x" << static_cast<uint32_t>(byte);
    }
    std::cout << std::endl;
#endif
    auto const responseLen = response.size();
    if (responseLen <= 2)
    {
        return;
    }
    auto const crc16calculated = Crc16(reinterpret_cast<uint8_t const*>(response.data()), responseLen - 2);
    uint8_t const crc16hi = *std::prev(response.cend(), 2);
    uint8_t const crc16lo = response.back();
    auto const crc16response = crc16hi << 8 | crc16lo;
    if (error ||
        (responseLen < (3 + 2 + count*2)) ||
        (response[0] != static_cast<char>(_deviceAddress)) ||
        (response[1] != 0x03) ||
        (response[2] != count*2) ||
        (crc16calculated != crc16response))
    {
      return;
    }
    requestedRegisters.reserve(count);
    auto currentRegIt = std::next(response.cbegin(), 3);
    auto endRegIt = std::prev(response.cend(), 2);
    while(currentRegIt != endRegIt)
    {
      requestedRegisters.emplace_back((*currentRegIt << 8) | *(currentRegIt + 1));
      currentRegIt += 2;
    }
  }, timeoutMs);
  return requestedRegisters;
}

bool ModBus::ForceSingleCoil(uint16_t registerAddress, bool isOn, uint16_t timeoutMs)
{
  std::string request{static_cast<char>(_deviceAddress),
                      0x05,
                      static_cast<char>(registerAddress >> 8),
                      static_cast<char>(registerAddress & 0xFF),
                      static_cast<char>(isOn ? 0xFF : 0x00),
                      static_cast<char>(0x00)};
  auto crc16 = Crc16(reinterpret_cast<uint8_t const*>(request.data()), request.size());
  request += static_cast<char>(crc16 >> 8);
  request += static_cast<char>(crc16 & 0xFF);
#if DEBUG_INFO
  for (auto byte : request)
  {
    std::cout << std::hex << " 0x" << static_cast<uint32_t>(byte);
  }
  std::cout << std::endl;
#endif
  bool result{};
  _serialPort.SendCommand(request, [&](std::string const& response, bool error) {
#if DEBUG_INFO
    for (auto byte : response)
    {
      std::cout << std::hex << " 0x" << static_cast<uint32_t>(byte);
    }
    std::cout << std::endl;
#endif
    result = !error && (response == request);
  }, timeoutMs);
  return result;
}

bool ModBus::WriteSingleHoldingRegister(uint16_t registerAddress, uint16_t value, uint16_t timeoutMs)
{
  std::string request{static_cast<char>(_deviceAddress),
                      0x06,
                      static_cast<char>(registerAddress >> 8),
                      static_cast<char>(registerAddress & 0xFF),
                      static_cast<char>(value >> 8),
                      static_cast<char>(value & 0xFF)};
  auto crc16 = Crc16(reinterpret_cast<uint8_t const*>(request.data()), request.size());
  request += static_cast<char>(crc16 >> 8);
  request += static_cast<char>(crc16 & 0xFF);
#if DEBUG_INFO
  for (auto byte : request)
  {
    std::cout << std::hex << " 0x" << static_cast<uint32_t>(byte);
  }
  std::cout << std::endl;
#endif
  bool result{};
  _serialPort.SendCommand(request, [&](std::string const& response, bool error) {
#if DEBUG_INFO
    for (auto byte : response)
    {
      std::cout << std::hex << " 0x" << static_cast<uint32_t>(byte);
    }
    std::cout << std::endl;
#endif
    result = !error && (response == request);
  }, timeoutMs);
  return result;
}

bool ModBus::WriteMultipleHoldingRegister(uint16_t startRegisterAddress, std::vector<uint16_t> values, uint16_t timeoutMs)
{
  if (values.empty())
  {
    return false;
  }
  std::string request{static_cast<char>(_deviceAddress),
                      0x10,
                      static_cast<char>(startRegisterAddress >> 8),
                      static_cast<char>(startRegisterAddress & 0xFF),
                      static_cast<char>(values.size() >> 8),
                      static_cast<char>(values.size() & 0xFF),
                      static_cast<char>(values.size() * 2)};
  for (auto const& value : values) {
    request += static_cast<char>(value >> 8);
    request += static_cast<char>(value & 0xFF);
  }
  auto crc16 = Crc16(reinterpret_cast<uint8_t const*>(request.data()), request.size());
  request += static_cast<char>(crc16 >> 8);
  request += static_cast<char>(crc16 & 0xFF);
#if DEBUG_INFO
  for (auto byte : request)
  {
    std::cout << std::hex << " 0x" << static_cast<uint32_t>(byte);
  }
  std::cout << std::endl;
#endif
  bool result{};
  _serialPort.SendCommand(request, [&](std::string const& response, bool error) {
#if DEBUG_INFO
    for (auto byte : response)
    {
      std::cout << std::hex << " 0x" << static_cast<uint32_t>(byte);
    }
    std::cout << std::endl;
#endif
    if (error ||
        (response.size() != 8) ||
        (response[0] != static_cast<char>(_deviceAddress)) ||
        (response[1] != 0x10) ||
        (((response[2] << 8) | (response[3] & 0xFF)) != startRegisterAddress) ||
        (((response[4] << 8) | (response[5] & 0xFF)) != values.size()) ||
        (Crc16(reinterpret_cast<uint8_t const*>(response.data()), response.size() - 2) != ((((uint8_t)*std::prev(response.cend(), 2)) << 8) | ((uint8_t)response.back()))))
    {
      return;
    }
    result = true;
  }, timeoutMs);
  return result;
}

auto ModBus::ReadInputRegisters(uint16_t startRegisterAddress, uint16_t count, uint16_t timeoutMs) -> std::vector<uint16_t>
{
  std::string request{static_cast<char>(_deviceAddress),
                      0x04,
                      static_cast<char>(startRegisterAddress >> 8),
                      static_cast<char>(startRegisterAddress & 0xFF),
                      static_cast<char>(count >> 8),
                      static_cast<char>(count & 0xFF)};
  auto crc16 = Crc16(reinterpret_cast<uint8_t const*>(request.data()), request.size());
  request += static_cast<char>(crc16 >> 8);
  request += static_cast<char>(crc16 & 0xFF);
#if DEBUG_INFO
  for (auto byte : request)
  {
    std::cout << std::hex << " 0x" << static_cast<uint32_t>(byte);
  }
  std::cout << std::endl;
#endif

  std::vector<uint16_t> requestedRegisters{};
  _serialPort.SendCommand(request, [&](std::string const& response, bool error) {
#if DEBUG_INFO
    for (auto byte : response)
    {
      std::cout << std::hex << " 0x" << static_cast<uint32_t>(byte);
    }
    std::cout << std::endl;
#endif
    if (error ||
        (response.size() < (3 + 2 + count*2)) ||
        (response[0] != static_cast<char>(_deviceAddress)) ||
        (response[1] != 0x04) ||
        (response[2] != count*2) ||
        (Crc16(reinterpret_cast<uint8_t const*>(response.data()), response.size() - 2) != ((((uint8_t)*std::prev(response.cend(), 2)) << 8) | ((uint8_t)response.back()))))
    {
      return;
    }
    requestedRegisters.reserve(count);
    auto currentRegIt = std::next(response.cbegin(), 3);
    auto endRegIt = std::prev(response.cend(), 2);
    while(currentRegIt != endRegIt)
    {
      requestedRegisters.emplace_back((*currentRegIt << 8) | *(currentRegIt + 1));
      currentRegIt += 2;
    }
  }, timeoutMs);
  return requestedRegisters;
}


