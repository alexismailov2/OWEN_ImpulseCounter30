#include <OWEN/ImpulseCounter30.hpp>

#include "SerialPort.hpp"
#include "ModBus.hpp"

#include <iostream>

namespace OWEN {

namespace {

auto ToSerialPortType(ImpulseCounter30::CommunicationOptions::eParity parity) -> SerialPort::eParity
{
   using eParity = ImpulseCounter30::CommunicationOptions::eParity;
   switch(parity)
   {
      case eParity::EVEN:
         return SerialPort::eParity::even;
      case eParity::ODD:
         return SerialPort::eParity::odd;
      case eParity::NO:
      default:
         return SerialPort::eParity::none;
   }
}

static auto ToSerialPortType(ImpulseCounter30::CommunicationOptions::eBaudrate baudrate) -> uint32_t
{
   using eBaudrate = ImpulseCounter30::CommunicationOptions::eBaudrate;
   switch(baudrate)
   {
      case eBaudrate::_2400bps:
         return 2400;
      case eBaudrate::_4800bps:
         return 4800;
      case eBaudrate::_9600bps:
         return 9600;
      case eBaudrate::_14400bps:
         return 14400;
      case eBaudrate::_19200bps:
         return 19200;
      case eBaudrate::_28800bps:
         return 28800;
      case eBaudrate::_38400bps:
         return 38400;
      case eBaudrate::_57600bps:
         return 57600;
      case eBaudrate::_115200bps:
         return 115200;
      default: return 0;
   }
}

auto AutoFind(ImpulseCounter30::CommunicationOptions& communicationOptions, ImpulseCounter30::tFindProgress progress) -> SerialPort
{
   auto serialPortCreator = [](ImpulseCounter30::CommunicationOptions const& communicationOptions) -> SerialPort {
     return SerialPort(communicationOptions._portPath,
                       ToSerialPortType(communicationOptions._baudrate.value()),
                       ToSerialPortType(communicationOptions._parity.value()),
                       communicationOptions._stopBitsExtended.value() ? SerialPort::eStopBits::two : SerialPort::eStopBits::one,
                       communicationOptions._dataBitsExtended.value() ? static_cast<uint8_t>(8) : static_cast<uint8_t>(7));
   };
   using eBaudrate = ImpulseCounter30::CommunicationOptions::eBaudrate;
   using eParity = ImpulseCounter30::CommunicationOptions::eParity;


   auto const totalIterations = (static_cast<uint32_t>(eBaudrate::_115200bps) + 1) *
                                2 *
                                (static_cast<uint32_t>(eParity::EVEN) + 1) *
                                2 *
                                255;
   auto currentIt = 0;
   auto& co = communicationOptions;
   for (co._baseAddr = 1; co._baseAddr.value() < 255; ++co._baseAddr.value())
   {
      for (co._baudrate = eBaudrate::_2400bps; co._baudrate.value() <= eBaudrate::_115200bps;)
      {
         auto dataBitsExtended = 0;
         for (co._dataBitsExtended = false; dataBitsExtended < 2; ++dataBitsExtended)
         {
            for (co._parity = eParity::NO; co._parity.value() <= eParity::ODD;)
            {
               auto stopBitsExtended = 0;
               for (co._stopBitsExtended = false; stopBitsExtended < 2; ++stopBitsExtended)
               {
                  //for (co._lengthAddrExtended = false; co._lengthAddrExtended.value() < true; ++co._lengthAddrExtended.value())
                  {
                     ++currentIt;
                     std::cout << "Trying to get response from device with next serial configuration: \n" << co << std::endl;
                     try
                     {
                        auto serialPort = serialPortCreator(co);
                        std::cout << "Trying to send request" << std::endl;
                        auto modbus = ModBus(serialPort, co._baseAddr.value());
                        if (!modbus.ReadHoldingRegisters(0x0000, 1, 100).empty())
                        {
                           progress(totalIterations, totalIterations, co);
                           return serialPortCreator(co);
                        }
                        if (!progress(currentIt, totalIterations, co))
                        {
                           return serialPortCreator(co);
                        }
                     }
                     catch(boost::system::system_error ex)
                     {
                        std::cout << ex.what() << std::endl;
                     }
                  }
                  co._stopBitsExtended = stopBitsExtended == 0;
               }
               auto parity = static_cast<uint32_t>(co._parity.value());
               ++parity;
               co._parity = static_cast<eParity>(parity);
            }
            co._dataBitsExtended = dataBitsExtended == 0;
         }
         auto baudrate = static_cast<uint32_t>(co._baudrate.value());
         ++baudrate;
         co._baudrate = static_cast<eBaudrate>(baudrate);
      }
   }
}
} /// end namespace anonymous

class ImpulseCounter30::Impl
{
public:
  Impl(CommunicationOptions communicationOptions,
       bool neededToBeFound,
       tFindProgress progress)
    : _serialPort{!neededToBeFound
        ? SerialPort(communicationOptions._portPath,
                     ToSerialPortType(communicationOptions._baudrate.value()),
                     ToSerialPortType(communicationOptions._parity.value()),
                     communicationOptions._stopBitsExtended.value() ? SerialPort::eStopBits::two : SerialPort::eStopBits::one,
                     communicationOptions._dataBitsExtended.value() ? static_cast<uint8_t>(8) : static_cast<uint8_t>(7))
        : AutoFind(communicationOptions, progress)}
    , _modBus{_serialPort, static_cast<uint8_t>(communicationOptions._baseAddr.value())}
  {
     if (_modBus.ReadHoldingRegisters(0x0000, 1, 100).empty())
     {
        throw std::runtime_error("Could not connected");
     }
  }

  auto GetCommunicationOptions() -> std::optional<CommunicationOptions>
  {
    auto const registers = _modBus.ReadHoldingRegisters(0x0000, 7);
    if (registers.size() < 7)
    {
      return {};
    }
    return CommunicationOptions{}.BaudeRate(static_cast<CommunicationOptions::eBaudrate>(registers[0]))
                                 .DataBits(registers[1])
                                 .Parity(static_cast<CommunicationOptions::eParity>(registers[2]))
                                 .StopBits(registers[3])
                                 .LengthAddr(registers[4])
                                 .BaseAddr(registers[5])
                                 .DelayAnswer(static_cast<uint8_t>(registers[6]));
  }

  bool SetCommunicationOptions(ImpulseCounter30::CommunicationOptions const& communicationOptions)
  {
    bool result{true};
    if (communicationOptions._baudrate.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x0000, static_cast<uint16_t>(communicationOptions._baudrate.value()));
    }
    if (communicationOptions._dataBitsExtended.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x0001, static_cast<uint16_t>(communicationOptions._dataBitsExtended.value()));
    }
    if (communicationOptions._parity.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x0002, static_cast<uint16_t>(communicationOptions._parity.value()));
    }
    if (communicationOptions._stopBitsExtended.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x0003, static_cast<uint16_t>(communicationOptions._stopBitsExtended.value()));
    }
    if (communicationOptions._lengthAddrExtended.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x0004, static_cast<uint16_t>(communicationOptions._lengthAddrExtended.value()));
    }
    if (communicationOptions._baseAddr.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x0005, static_cast<uint16_t>(communicationOptions._baseAddr.value()));
    }
    if (communicationOptions._delayAnswerMs.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x0006, static_cast<uint16_t>(communicationOptions._delayAnswerMs.value()));
    }
    return result;
  }

  auto GetCounterOptions() -> std::optional<ImpulseCounter30::CounterOptions>
  {
    auto registers = _modBus.ReadHoldingRegisters(0x0007, 13, 10000);
    auto registersContinue = _modBus.ReadHoldingRegisters(0x0007 + 13, 11, 10000);
    registers.insert(registers.end(), registersContinue.cbegin(), registersContinue.cend());
    if (registers.size() < 24)
    {
      return {};
    }
    return CounterOptions{}.DecPoint(static_cast<CounterOptions::eDecPoint>(registers[0]))
                           .InputMode(static_cast<CounterOptions::eInputMode>(registers[1]))
                           .OutputMode(static_cast<CounterOptions::eOutputMode>(registers[2]))
                           .SetPointMode(static_cast<CounterOptions::ePointMode>(registers[3]))
                           .ResetType(static_cast<CounterOptions::eResetType>(registers[4]))
                           .SetPoint1((((uint32_t)registers[5]) << 16) | (((uint32_t)registers[6]) & 0xFFFF))
                           .SetPoint2((((uint32_t)registers[7]) << 16) | (((uint32_t)registers[8]) & 0xFFFF))
                           .TimeOUT1((((uint32_t)registers[9]) << 16) | (((uint32_t)registers[10]) & 0xFFFF))
                           .TimeOUT2((((uint32_t)registers[11]) << 16) | (((uint32_t)registers[12]) & 0xFFFF))
                           .DecPointMult(static_cast<uint8_t>(registers[13]))
                           .Multiplexer((((uint32_t)registers[14]) << 16) | (((uint32_t)registers[15]) & 0xFFFF))
                           .MaxFreq(registers[16])
                           .MinControl((((uint32_t)registers[17]) << 16) | (((uint32_t)registers[18]) & 0xFFFF))
                           .LockKBD(static_cast<CounterOptions::eLockKBD>(registers[19]))
                           .ShowSetPoint(static_cast<CounterOptions::eShowSetPoint>(registers[20]))
                           .Brightness(registers[21])
                           .InputType(static_cast<CounterOptions::eInputType>(registers[22]))
                           .Password(registers[23]);
  }

  bool SetCounterOptions(ImpulseCounter30::CounterOptions const& counterOptions)
  {
    bool result{true};
    if (counterOptions._decPoint.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x0007, static_cast<uint16_t>(counterOptions._decPoint.value()));
    }
    if (counterOptions._inputMode.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x0008, static_cast<uint16_t>(counterOptions._inputMode.value()));
    }
    if (counterOptions._outputMode.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x0009, static_cast<uint16_t>(counterOptions._outputMode.value()));
    }
    if (counterOptions._pointMode.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x000A, static_cast<uint16_t>(counterOptions._pointMode.value()));
    }
    if (counterOptions._resetType.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x000B, static_cast<uint16_t>(counterOptions._resetType.value()));
    }
    if (counterOptions._point1Threshold.has_value())
    {
      result &= _modBus.WriteMultipleHoldingRegister(0x000C,
                                                   {static_cast<uint16_t>(counterOptions._point1Threshold.value() >> 16),
                                                    static_cast<uint16_t>(counterOptions._point1Threshold.value() & 0xFFFF)});
    }
    if (counterOptions._point2Threshold.has_value())
    {
      result &= _modBus.WriteMultipleHoldingRegister(0x000E,
                                                     {static_cast<uint16_t>(counterOptions._point2Threshold.value() >> 16),
                                                      static_cast<uint16_t>(counterOptions._point2Threshold.value() & 0xFFFF)});
    }
    if (counterOptions._timeout1.has_value())
    {
      result &= _modBus.WriteMultipleHoldingRegister(0x0010,
                                                     {static_cast<uint16_t>(counterOptions._timeout1.value() >> 16),
                                                      static_cast<uint16_t>(counterOptions._timeout1.value() & 0xFFFF)});
    }
    if (counterOptions._timeout2.has_value())
    {
      result &= _modBus.WriteMultipleHoldingRegister(0x0012,
                                                     {static_cast<uint16_t>(counterOptions._timeout2.value() >> 16),
                                                      static_cast<uint16_t>(counterOptions._timeout2.value() & 0xFFFF)});
    }
    if (counterOptions._decPointMult.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x0014, static_cast<uint16_t>(counterOptions._decPointMult.value()));
    }
    if (counterOptions._multiplexer.has_value())
    {
      result &= _modBus.WriteMultipleHoldingRegister(0x0015,
                                                     {static_cast<uint16_t>(counterOptions._multiplexer.value() >> 16),
                                                      static_cast<uint16_t>(counterOptions._multiplexer.value() & 0xFFFF)});
    }
    if (counterOptions._maxFreq.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x0017, static_cast<uint16_t>(counterOptions._maxFreq.value()));
    }
    if (counterOptions._minControl.has_value())
    {
      result &= _modBus.WriteMultipleHoldingRegister(0x0018,
                                                     {static_cast<uint16_t>(counterOptions._minControl.value() >> 16),
                                                      static_cast<uint16_t>(counterOptions._minControl.value() & 0xFFFF)});
    }
    if (counterOptions._lockKbd.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x001A, static_cast<uint16_t>(counterOptions._lockKbd.value()));
    }
    if (counterOptions._showSetPoint.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x001B, static_cast<uint16_t>(counterOptions._showSetPoint.value()));
    }
    if (counterOptions._brightness.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x001C, static_cast<uint16_t>(counterOptions._brightness.value()));
    }
    if (counterOptions._inputType.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x001D, static_cast<uint16_t>(counterOptions._inputType.value()));
    }
    if (counterOptions._password.has_value())
    {
      result &= _modBus.WriteSingleHoldingRegister(0x001E, static_cast<uint16_t>(counterOptions._password.value()));
    }
    return result;
  }

  auto GetCounterValue() -> std::optional<int32_t>
  {
    auto const registers = _modBus.ReadInputRegisters(0x0000, 2);
    return (registers.size() != 2)
              ? std::optional<int32_t>{}
              : std::optional<int32_t>{(((uint32_t)registers[0]) << 16) | (((uint32_t)registers[1]) & 0xFFFF)};
  }

  auto GetCounterEU() -> std::optional<int32_t>
  {
    auto const registers = _modBus.ReadInputRegisters(0x0002, 2);
    return (registers.size() != 2)
           ? std::optional<int32_t>{}
           : std::optional<int32_t>{(((uint32_t)registers[0]) << 16) | (((uint32_t)registers[1]) & 0xFFFF)};
  }

  auto GetStartStopMode() -> std::optional<bool>
  {
    auto const registers = _modBus.ReadInputRegisters(0x0004, 1);
    return (registers.size() != 1) ? std::optional<bool>{} : std::optional<bool>{static_cast<bool>(registers[0])};
  }

  auto GetCurrentMode() -> std::optional<ImpulseCounter30::eCurrentMode>
  {
    auto const registers = _modBus.ReadInputRegisters(0x0005, 1);
    return (registers.size() != 1)
              ? std::optional<ImpulseCounter30::eCurrentMode>{}
              : std::optional<ImpulseCounter30::eCurrentMode>{static_cast<ImpulseCounter30::eCurrentMode>(registers[0])};
  }

  auto GetCodeErrNet() -> std::optional<uint8_t>
  {
    auto const registers = _modBus.ReadInputRegisters(0x0006, 1);
    return (registers.size() != 1) ? std::optional<uint8_t >{} : std::optional<uint8_t >{static_cast<uint8_t >(registers[0])};
  }

  auto GetNameDevice() -> std::optional<std::string>
  {
    auto const registers = _modBus.ReadInputRegisters(0x0007, 2);
    return (registers.size() != 2)
           ? std::optional<std::string>{}
           : std::optional<std::string>{std::string{reinterpret_cast<char const*>(registers.data()), registers.size() * 2}};
  }

  auto GetVersion() -> std::optional<std::string>
  {
    auto const registers = _modBus.ReadInputRegisters(0x0009, 2);
    return (registers.size() != 2)
           ? std::optional<std::string>{}
           : std::optional<std::string>{std::string{reinterpret_cast<char const*>(registers.data()), registers.size() * 2}};
  }

  auto IsResetInput() -> std::optional<bool>
  {
     auto const bits = _modBus.ReadInputStatus(0x0000, 1);
     return (bits.empty()) ? std::optional<bool>{} : std::optional<bool>{bits[0]};
  }

  auto IsLockInput() -> std::optional<bool>
  {
    auto const bits = _modBus.ReadInputStatus(0x0001, 1);
    return (bits.empty()) ? std::optional<bool>{} : std::optional<bool>{bits[0]};
  }

  auto GetOutState1() -> std::optional<bool>
  {
    auto const bits = _modBus.ReadCoilStatus(0x0000, 1);
    return (bits.empty()) ? std::optional<bool>{} : std::optional<bool>{bits[0]};
  }

  auto GetOutState2() -> std::optional<bool>
  {
    auto const bits = _modBus.ReadCoilStatus(0x0001, 1);
    return (bits.empty()) ? std::optional<bool>{} : std::optional<bool>{bits[0]};
  }

  auto IsResetCount() -> std::optional<bool>
  {
    auto const bits = _modBus.ReadCoilStatus(0x0002, 1);
    return (bits.empty()) ? std::optional<bool>{} : std::optional<bool>{bits[0]};
  }

  bool ResetCount()
  {
    return _modBus.ForceSingleCoil(0x0002, true);
  }

  bool ControlCounterFromProgram(bool isEnabled)
  {
    return _modBus.ForceSingleCoil(0x0003, isEnabled);
  }

  bool StartCounter(bool isStart)
  {
    return _modBus.ForceSingleCoil(0x0004, isStart);
  }

  bool StopCounter(bool isStop)
  {
    return StartCounter(!isStop);
  }

private:
  SerialPort _serialPort;
  ModBus _modBus;
};

ImpulseCounter30::ImpulseCounter30(CommunicationOptions const& communicationOptions, bool neededToBeFound, tFindProgress progress)
  : pImpl{std::make_unique<Impl>(communicationOptions, neededToBeFound, progress)}
{
}

ImpulseCounter30::~ImpulseCounter30() = default;

ImpulseCounter30::ImpulseCounter30(ImpulseCounter30&&) noexcept = default;

ImpulseCounter30& ImpulseCounter30::operator=(ImpulseCounter30&&) noexcept = default;

bool ImpulseCounter30::SetCommunicationOptions(CommunicationOptions const& communicationOptions)
{
   return pImpl->SetCommunicationOptions(communicationOptions);
}

auto ImpulseCounter30::GetCommunicationOptions() -> std::optional<CommunicationOptions>
{
   return pImpl->GetCommunicationOptions();
}

bool ImpulseCounter30::SetCounterOptions(CounterOptions const& counterOptions)
{
   return pImpl->SetCounterOptions(counterOptions);
}

auto ImpulseCounter30::GetCounterOptions() -> std::optional<CounterOptions>
{
   return pImpl->GetCounterOptions();
}

auto ImpulseCounter30::GetCounterValue() -> std::optional<int32_t>
{
  return pImpl->GetCounterValue();
}

auto ImpulseCounter30::GetCounterEU() -> std::optional<int32_t>
{
  return pImpl->GetCounterEU();
}

auto ImpulseCounter30::GetStartStopMode() -> std::optional<bool>
{
  return pImpl->GetStartStopMode();
}

auto ImpulseCounter30::GetCurrentMode() -> std::optional<ImpulseCounter30::eCurrentMode>
{
  return pImpl->GetCurrentMode();
}

auto ImpulseCounter30::GetCodeErrNet() -> std::optional<uint8_t>
{
  return pImpl->GetCodeErrNet();
}

auto ImpulseCounter30::GetNameDevice() -> std::optional<std::string>
{
  return pImpl->GetNameDevice();
}

auto ImpulseCounter30::GetVersion() -> std::optional<std::string>
{
  return pImpl->GetVersion();
}

auto ImpulseCounter30::IsResetInput() -> std::optional<bool>
{
  return pImpl->IsResetInput();
}

auto ImpulseCounter30::IsLockInput() -> std::optional<bool>
{
  return pImpl->IsLockInput();
}

auto ImpulseCounter30::GetOutState1() -> std::optional<bool>
{
  return pImpl->GetOutState1();
}

auto ImpulseCounter30::GetOutState2() -> std::optional<bool>
{
  return pImpl->GetOutState2();
}

auto ImpulseCounter30::IsResetCount() -> std::optional<bool>
{
  return pImpl->IsResetCount();
}

bool ImpulseCounter30::ResetCount()
{
  return pImpl->ResetCount();
}

bool ImpulseCounter30::ControlCounterFromProgram(bool isEnabled)
{
  return pImpl->ControlCounterFromProgram(isEnabled);
}

bool ImpulseCounter30::StartCounter(bool isStart)
{
  return pImpl->StartCounter(isStart);
}

bool ImpulseCounter30::StopCounter(bool isStop)
{
  return pImpl->StopCounter(isStop);
}

auto operator<<(std::ostream& out, OWEN::ImpulseCounter30::CommunicationOptions::eBaudrate const& baudrate) -> std::ostream&
{
  using eBaudrate = OWEN::ImpulseCounter30::CommunicationOptions::eBaudrate;
  switch(baudrate)
  {
    case eBaudrate::_2400bps:
      out << "2400bps";
      break;
    case eBaudrate::_4800bps:
      out << "4800bps";
      break;
    case eBaudrate::_9600bps:
      out << "9600bps";
      break;
    case eBaudrate::_14400bps:
      out << "14400bps";
      break;
    case eBaudrate::_19200bps:
      out << "19200bps";
      break;
    case eBaudrate::_28800bps:
      out << "28800bps";
      break;
    case eBaudrate::_38400bps:
      out << "38400bps";
      break;
    case eBaudrate::_57600bps:
      out << "57600bps";
      break;
    case eBaudrate::_115200bps:
      out << "115200bps";
      break;
    default:
      out << "-";
  }
  return out;
}

auto operator<<(std::ostream& out, OWEN::ImpulseCounter30::CommunicationOptions::eParity const& parity) -> std::ostream&
{
  using eParity = OWEN::ImpulseCounter30::CommunicationOptions::eParity;
  switch(parity)
  {
    case eParity::NO:
      out << "NO";
      break;
    case eParity::ODD:
      out << "ODD";
      break;
    case eParity::EVEN:
      out << "EVEN";
      break;
    default:
      out << "-";
  }
  return out;
}

auto operator<<(std::ostream& out, OWEN::ImpulseCounter30::CommunicationOptions const& communicationOptions) -> std::ostream&
{
   using eBaudrate = OWEN::ImpulseCounter30::CommunicationOptions::eBaudrate;
   using eParity = OWEN::ImpulseCounter30::CommunicationOptions::eParity;
   auto const baudrate = communicationOptions._baudrate.has_value()
                            ? communicationOptions._baudrate.value() : static_cast<eBaudrate>(-1);
   auto const databits = communicationOptions._dataBitsExtended.has_value()
                            ? (communicationOptions._dataBitsExtended.value() ? '8' : '7') : '-';
   auto const parity = communicationOptions._parity.has_value()
                          ? communicationOptions._parity.value() : static_cast<eParity>(-1);
   auto const stopBits = communicationOptions._stopBitsExtended.has_value()
                            ? (communicationOptions._stopBitsExtended.value() ? '2' : '1') : '-';
   auto const lengthAddr = communicationOptions._lengthAddrExtended.has_value()
                              ? (communicationOptions._lengthAddrExtended.value() ? "11bit - [0..2047]" : "8bit - [1..255]") : "-";
   auto const baseAddr = communicationOptions._baseAddr.has_value()
                            ? std::to_string(communicationOptions._baseAddr.value()) : "-";
   auto const delayAnswerMs = communicationOptions._delayAnswerMs.has_value()
                            ? std::to_string(communicationOptions._delayAnswerMs.value()) : "-";
   out << "baudrate: " << baudrate << '\n'
       << "dataBits: " << databits << '\n'
       << "parity: " << parity << '\n'
       << "stopBits: " << stopBits << '\n'
       << "lengthAddr: " << lengthAddr << '\n'
       << "baseAddr: " << baseAddr << '\n'
       << "delayAnswerMs: " << delayAnswerMs;
   return out;
}

auto operator<<(std::ostream& out, OWEN::ImpulseCounter30::CounterOptions::eDecPoint const& decPoint) -> std::ostream&
{
  using eDecPoint = OWEN::ImpulseCounter30::CounterOptions::eDecPoint;
  switch(decPoint)
  {
    case eDecPoint::_0:
      out << '0';
      break;
    case eDecPoint::_1:
      out << '1';
      break;
    case eDecPoint::_2:
      out << '2';
      break;
    case eDecPoint::_3:
      out << '3';
      break;
    case eDecPoint::_4:
      out << '4';
      break;
    default:
      out << '-';
  }
  return out;
}

auto operator<<(std::ostream& out, OWEN::ImpulseCounter30::CounterOptions::eInputMode const& inputMode) -> std::ostream&
{
  using eInputMode = OWEN::ImpulseCounter30::CounterOptions::eInputMode;
  switch(inputMode)
  {
    case eInputMode::FORWARD :
      out << "FORWARD";
      break;
    case eInputMode::BACKWARD :
      out << "BACKWARD";
      break;
    case eInputMode::BY_COMMAND :
      out << "BY_COMMAND";
      break;
    case eInputMode::INDIVIDUAL :
      out << "INDIVIDUAL";
      break;
    case eInputMode::REVERSE :
      out << "REVERSE";
      break;
    case eInputMode::QUAD :
      out << "QUAD";
      break;
    default:
      out << '-';
  }
  return out;
}

auto operator<<(std::ostream& out, OWEN::ImpulseCounter30::CounterOptions::eOutputMode const& outputMode) -> std::ostream&
{
  using eOutputMode = OWEN::ImpulseCounter30::CounterOptions::eOutputMode;
  switch(outputMode)
  {
    case eOutputMode::TURNED_ON_ABOVE_THRESHOLD :
      out << "TURNED_ON_ABOVE_THRESHOLD";
      break;
    case eOutputMode::TURNED_ON_BELOW_THRESHOLD :
      out << "TURNED_ON_BELOW_THRESHOLD";
      break;
    case eOutputMode::TURNED_ON_ABOVE_THRESHOLD_WITH_TIMEOUT :
      out << "TURNED_ON_ABOVE_THRESHOLD_WITH_TIMEOUT";
      break;
    case eOutputMode::TURNED_ON_MULTIPLICITY_THRESHOLD :
      out << "TURNED_ON_MULTIPLICITY_THRESHOLD";
      break;
    default:
      out << '-';
  }
  return out;
}

auto operator<<(std::ostream& out, OWEN::ImpulseCounter30::CounterOptions::ePointMode const& pointMode) -> std::ostream&
{
  using ePointMode = OWEN::ImpulseCounter30::CounterOptions::ePointMode;
  switch(pointMode)
  {
    case ePointMode::CONTINUE_WITHOUT_RESET :
      out << "CONTINUE_WITHOUT_RESET";
      break;
    case ePointMode::STOP_UNTILL_RESET_WILL_APPEAR :
      out << "STOP_UNTILL_RESET_WILL_APPEAR";
      break;
    case ePointMode::RESET_AND_CONTINUE :
      out << "RESET_AND_CONTINUE";
      break;
    default:
      out << '-';
  }
  return out;
}

auto operator<<(std::ostream& out, OWEN::ImpulseCounter30::CounterOptions::eResetType const& resetType) -> std::ostream&
{
  using eResetType = OWEN::ImpulseCounter30::CounterOptions::eResetType;
  switch(resetType)
  {
    case eResetType::RESET_ONLY :
      out << "RESET_ONLY";
      break;
    case eResetType::RESET_AND_CLEAR_OUTPUTS :
      out << "RESET_AND_CLEAR_OUTPUTS";
      break;
    case eResetType::RESET_AND_WAIT_FOR_START :
      out << "RESET_AND_WAIT_FOR_START";
      break;
    case eResetType::RESET_AND_WAIT_FOR_STOP :
      out << "RESET_AND_WAIT_FOR_STOP";
      break;
    default:
      out << '-';
  }
  return out;
}

auto operator<<(std::ostream& out, OWEN::ImpulseCounter30::CounterOptions::eLockKBD const& lockKbd) -> std::ostream&
{
  using eLockKBD = OWEN::ImpulseCounter30::CounterOptions::eLockKBD;
  switch(lockKbd)
  {
    case eLockKBD::UNLOCKED :
      out << "UNLOCKED";
      break;
    case eLockKBD::LOCKED_RESET :
      out << "LOCKED_RESET";
      break;
    case eLockKBD::LOCKED_OPTIONS :
      out << "LOCKED_OPTIONS";
      break;
    case eLockKBD::LOCKED_RESET_AND_OPTIONS :
      out << "LOCKED_RESET_AND_OPTIONS";
      break;
    default:
      out << '-';
  }
  return out;
}

auto operator<<(std::ostream& out, OWEN::ImpulseCounter30::CounterOptions::eShowSetPoint const& showSetPoint) -> std::ostream&
{
  using eShowSetPoint = OWEN::ImpulseCounter30::CounterOptions::eShowSetPoint;
  switch(showSetPoint)
  {
    case eShowSetPoint::_1 :
      out << '1';
      break;
    case eShowSetPoint::_2 :
      out << '2';
      break;
    default:
      out << '-';
  }
  return out;
}

auto operator<<(std::ostream& out, OWEN::ImpulseCounter30::CounterOptions::eInputType const& inputType) -> std::ostream&
{
  using eInputType = OWEN::ImpulseCounter30::CounterOptions::eInputType;
  switch(inputType)
  {
    case eInputType::NPN :
      out << "NPN";
      break;
    case eInputType::PNP :
      out << "PNP";
      break;
    default:
      out << '-';
  }
  return out;
}

auto operator<<(std::ostream& out, OWEN::ImpulseCounter30::CounterOptions const& counterOptions) -> std::ostream&
{
  using eDecPoint = OWEN::ImpulseCounter30::CounterOptions::eDecPoint;
  using eInputMode = OWEN::ImpulseCounter30::CounterOptions::eInputMode;
  using eOutputMode = OWEN::ImpulseCounter30::CounterOptions::eOutputMode;
  using ePointMode = OWEN::ImpulseCounter30::CounterOptions::ePointMode;
  using eResetType = OWEN::ImpulseCounter30::CounterOptions::eResetType;
  using eLockKBD = OWEN::ImpulseCounter30::CounterOptions::eLockKBD;
  using eShowSetPoint = OWEN::ImpulseCounter30::CounterOptions::eShowSetPoint;
  using eInputType = OWEN::ImpulseCounter30::CounterOptions::eInputType;

  auto const decPoint = counterOptions._decPoint.has_value()
                           ? counterOptions._decPoint.value() : static_cast<eDecPoint>(-1);
  auto const inputMode = counterOptions._inputMode.has_value()
                        ? counterOptions._inputMode.value() : static_cast<eInputMode>(-1);
  auto const outputMode = counterOptions._outputMode.has_value()
                        ? counterOptions._outputMode.value() : static_cast<eOutputMode>(-1);
  auto const pointMode = counterOptions._pointMode.has_value()
                        ? counterOptions._pointMode.value() : static_cast<ePointMode>(-1);
  auto const resetType = counterOptions._resetType.has_value()
                         ? counterOptions._resetType.value() : static_cast<eResetType>(-1);
  auto const setPoint1 = counterOptions._point1Threshold.has_value()
                        ? std::to_string(counterOptions._point1Threshold.value()) : "-";
  auto const setPoint2 = counterOptions._point2Threshold.has_value()
                        ? std::to_string(counterOptions._point2Threshold.value()) : "-";
  auto const timeout1 = counterOptions._timeout1.has_value()
                        ? std::to_string(counterOptions._timeout1.value()) : "-";
  auto const timeout2 = counterOptions._timeout2.has_value()
                        ? std::to_string(counterOptions._timeout2.value()) : "-";
  auto const decPointMult = counterOptions._decPointMult.has_value()
                        ? std::to_string(counterOptions._decPointMult.value()) : "-";
  auto const multiplexer = counterOptions._multiplexer.has_value()
                        ? std::to_string(counterOptions._multiplexer.value()) : "-";
  auto const maxFreq = counterOptions._maxFreq.has_value()
                        ? std::to_string(counterOptions._maxFreq.value()) : "-";
  auto const minControl = counterOptions._minControl.has_value()
                        ? std::to_string(counterOptions._minControl.value()) : "-";
  auto const lockKbd = counterOptions._lockKbd.has_value()
                         ? counterOptions._lockKbd.value() : static_cast<eLockKBD>(-1);
  auto const showSetPoint = counterOptions._showSetPoint.has_value()
                       ? counterOptions._showSetPoint.value() : static_cast<eShowSetPoint>(-1);
  auto const brightness = counterOptions._brightness.has_value()
                        ? std::to_string(counterOptions._brightness.value()) : "-";
  auto const inputType = counterOptions._inputType.has_value()
                       ? counterOptions._inputType.value() : static_cast<eInputType>(-1);
  auto const password = counterOptions._password.has_value()
                          ? std::to_string(counterOptions._password.value()) : "-";
  out << "decPoint: " << decPoint << '\n'
      << "inputMode: " << inputMode << '\n'
      << "outputMode: " << outputMode << '\n'
      << "pointMode: " << pointMode << '\n'
      << "resetType: " << resetType << '\n'
      << "setPoint1: " << setPoint1 << '\n'
      << "setPoint2: " << setPoint2 << '\n'
      << "timeout1: " << timeout1 << '\n'
      << "timeout2: " << timeout2 << '\n'
      << "decPointMult: " << decPointMult << '\n'
      << "multiplexer: " << multiplexer << '\n'
      << "maxFreq: " << maxFreq << '\n'
      << "minControl: " << minControl << '\n'
      << "lockKbd: " << lockKbd << '\n'
      << "showSetPoint: " << showSetPoint << '\n'
      << "brightness: " << brightness << '\n'
      << "inputType: " << inputType << '\n'
      << "password: " << password << '\n';
  return out;
}

} /// end namespace OWEN
