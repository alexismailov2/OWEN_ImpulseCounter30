#pragma once

#include <memory>
#include <string>
#include <optional>
#include <functional>

namespace OWEN {

class ImpulseCounter30
{
public:
  class CommunicationOptions
  {
  public:
    enum class eBaudrate {
      _2400bps,
      _4800bps,
      _9600bps,
      _14400bps,
      _19200bps,
      _28800bps,
      _38400bps,
      _57600bps,
      _115200bps
    };

    enum class eParity {
      NO,
      EVEN,
      ODD
    };

  public:
     CommunicationOptions() = default;

     /**
      * Set port path.
      * @param portPath path to the port or port name.
      * @return reference to CommunicationOptions.
      */
     CommunicationOptions& PortPath(std::string portPath) {
        _portPath = std::move(portPath);
        return *this;
     }

    /**
     * Set baudrate.
     * @param baudrate all values from eBaudrate enumeration.
     * @return reference to CommunicationOptions.
     */
    CommunicationOptions& BaudeRate(eBaudrate baudrate) {
      _baudrate = baudrate;
      return *this;
    }

    /**
     * Set usual or extended data bits.
     * @param dataBitsExtended true - 8bit, false - 7bit.
     * @return reference to CommunicationOptions.
     */
    CommunicationOptions& DataBits(bool dataBitsExtended) {
      _dataBitsExtended = dataBitsExtended;
      return *this;
    }

    /**
     * Set parity checking.
     * @param parity NO - no parity checking, EVEN - even parity checking, ODD - odd parity checking.
     * @return reference to CommunicationOptions.
     */
    CommunicationOptions& Parity(eParity parity) {
       _parity = parity;
      return *this;
    }

    /**
     * Set usual or extended stop bits count.
     * @param stopBitsExtended true - 2 stop bit, false 1 stop bit.
     * @return reference to CommunicationOptions.
     */
    CommunicationOptions& StopBits(bool stopBitsExtended) {
      _stopBitsExtended = stopBitsExtended;
      return *this;
    }

    /**
     * Set usial or extended length of addr.
     * @param extended if true - 11 bit, false - 8 bit.
     * @return reference to CommunicationOptions.
     */
    CommunicationOptions& LengthAddr(bool lengthAddrExtended) {
      _lengthAddrExtended = lengthAddrExtended;
      return *this;
    }

    /**
     * Set base addr.
     * @param baseAddr base addr.
     * @return reference to CommunicationOptions.
     */
    CommunicationOptions& BaseAddr(uint16_t baseAddr) {
      if (baseAddr < 1 || baseAddr > 2047)
      {
        throw std::runtime_error("BaseAddr should be in range 1 - 2047.");
      }
      _baseAddr = baseAddr;
      return *this;
    }

    /**
     * Set delay answer for RS485 interface.
     * @param delayMs delay for answer in milliseconds for RS485 only.
     * @return reference to CommunicationOptions.
     */
    CommunicationOptions& DelayAnswer(uint8_t delayAnswerMs) {
      if (delayAnswerMs > 45)
      {
        throw std::runtime_error("DelayAnswer should be in range 0 - 45.");
      }
      _delayAnswerMs = delayAnswerMs;
      return *this;
    }

    std::string              _portPath;
    std::optional<eBaudrate> _baudrate;
    std::optional<bool> _dataBitsExtended;
    std::optional<eParity> _parity;
    std::optional<bool> _stopBitsExtended;
    std::optional<bool> _lengthAddrExtended;
    std::optional<uint16_t> _baseAddr;
    std::optional<uint8_t> _delayAnswerMs;
  };

  using tFindProgress = std::function<bool(uint32_t currentProgress, uint32_t finishValue, CommunicationOptions const& communicationOptions)>;

  class CounterOptions {
  public:
    enum class eDecPoint {
      _0,
      _1,
      _2,
      _3,
      _4
    };

    enum class eInputMode {
      FORWARD,
      BACKWARD,
      BY_COMMAND,
      INDIVIDUAL,
      REVERSE,
      QUAD
    };

    enum class eOutputMode {
      TURNED_ON_ABOVE_THRESHOLD,
      TURNED_ON_BELOW_THRESHOLD,
      TURNED_ON_ABOVE_THRESHOLD_WITH_TIMEOUT,
      TURNED_ON_MULTIPLICITY_THRESHOLD
    };

    enum class ePointMode {
      CONTINUE_WITHOUT_RESET,
      STOP_UNTILL_RESET_WILL_APPEAR,
      RESET_AND_CONTINUE
    };

    enum class eResetType {
      RESET_ONLY,
      RESET_AND_CLEAR_OUTPUTS,
      RESET_AND_WAIT_FOR_START,
      RESET_AND_WAIT_FOR_STOP
    };

    enum class eLockKBD {
      UNLOCKED,
      LOCKED_RESET,
      LOCKED_OPTIONS,
      LOCKED_RESET_AND_OPTIONS
    };

    enum class eShowSetPoint {
      _1,
      _2
    };

    enum class eInputType {
      NPN,
      PNP
    };
  public:
    CounterOptions& DecPoint(eDecPoint decPoint)
    {
      _decPoint = decPoint;
      return *this;
    }

    CounterOptions& InputMode(eInputMode inputMode)
    {
      _inputMode = inputMode;
      return *this;
    }

    CounterOptions& OutputMode(eOutputMode outputMode)
    {
      _outputMode = outputMode;
      return *this;
    }

    CounterOptions& SetPointMode(ePointMode pointMode)
    {
      _pointMode = pointMode;
      return *this;
    }

    CounterOptions& ResetType(eResetType resetType)
    {
      _resetType = resetType;
      return *this;
    }

    CounterOptions& SetPoint1(int32_t point1Threshold)
    {
      if ((point1Threshold > 999999) || (point1Threshold < -99999)) {
        throw std::runtime_error("Should be in range -99999..999999.");
      }
      _point1Threshold = point1Threshold;
      return *this;
    }

    CounterOptions& SetPoint2(int32_t point2Threshold)
    {
      if ((point2Threshold > 999999) || (point2Threshold < -99999)) {
        throw std::runtime_error("Should be in range -99999..999999.");
      }
      _point2Threshold = point2Threshold;
      return *this;
    }

    CounterOptions& TimeOUT1(uint32_t timeout1)
    {
      if (timeout1 > 999990) {
        throw std::runtime_error("Should be in range 0..999990.");
      }
      _timeout1 = timeout1;
      return *this;
    }

    CounterOptions& TimeOUT2(uint32_t timeout2)
    {
      if (timeout2 > 999990) {
        throw std::runtime_error("Should be in range 0..999990.");
      }
      _timeout2 = timeout2;
      return *this;
    }

    CounterOptions& DecPointMult(uint8_t decPointMult)
    {
      if (decPointMult > 5) {
        throw std::runtime_error("Should be in range 0..5.");
      }
      _decPointMult = decPointMult;
      return *this;
    }

    CounterOptions& Multiplexer(uint32_t multiplexer)
    {
      if ((multiplexer < 1) || (multiplexer > 999999)) {
        throw std::runtime_error("Should be in range 1..999999.");
      }
      _multiplexer = multiplexer;
      return *this;
    }

    CounterOptions& MaxFreq(uint16_t maxFreq)
    {
      if ((maxFreq < 1) || (maxFreq > 50000)) {
        throw std::runtime_error("Should be in range 1..50000.");
      }
      _maxFreq = maxFreq;
      return *this;
    }

    CounterOptions& MinControl(uint32_t minControl)
    {
      if ((minControl < 1) || (minControl > 999999)) {
        throw std::runtime_error("Should be in range 1..999999.");
      }
      _minControl = minControl;
      return *this;
    }

    CounterOptions& LockKBD(eLockKBD lockKbd)
    {
      _lockKbd = lockKbd;
      return *this;
    }

    CounterOptions& ShowSetPoint(eShowSetPoint showSetPoint)
    {
      _showSetPoint = showSetPoint;
      return *this;
    }

    CounterOptions& Brightness(uint8_t brightness)
    {
      if ((brightness < 1) || (brightness > 3)) {
        throw std::runtime_error("Should be in range 1..3.");
      }
      _brightness = brightness;
      return *this;
    }

    CounterOptions& InputType(eInputType inputType)
    {
      _inputType = inputType;
      return *this;
    }

    CounterOptions& Password(uint16_t password = 0)
    {
      if ((password > 9999)) {
        throw std::runtime_error("Should be in range 0..9999.");
      }
      _password = password;
      return *this;
    }
    /// ...
    std::optional<eDecPoint> _decPoint{};
    std::optional<eInputMode> _inputMode{};
    std::optional<eOutputMode> _outputMode{};
    std::optional<ePointMode> _pointMode{};
    std::optional<eResetType> _resetType{};
    std::optional<int32_t> _point1Threshold{};
    std::optional<int32_t> _point2Threshold{};
    std::optional<uint32_t> _timeout1{};
    std::optional<uint32_t> _timeout2{};
    std::optional<uint8_t> _decPointMult{};
    std::optional<uint32_t> _multiplexer{};
    std::optional<uint16_t> _maxFreq{};
    std::optional<uint32_t> _minControl{};
    std::optional<eLockKBD> _lockKbd{};
    std::optional<eShowSetPoint> _showSetPoint{};
    std::optional<uint8_t> _brightness{};
    std::optional<eInputType> _inputType{};
    std::optional<uint16_t> _password{};
  };

  enum class eCurrentMode {
    COUNTING_PASSWORD_DOES_NOT_NEEDED,
    SET_FROM_PANEL,
    SET_FROM_RS485,
    COUNTING_PASWORD_NEEDED
  };

public:
  ImpulseCounter30(CommunicationOptions const& communicationOptions = {},
                   bool neededToBeFound = false,
                   tFindProgress progress = [](uint32_t currentProgress, uint32_t finishValue, CommunicationOptions const& communicationOptions) -> bool{ return true; });
  ~ImpulseCounter30();
  ImpulseCounter30(ImpulseCounter30&&) noexcept;
  ImpulseCounter30& operator=(ImpulseCounter30&&) noexcept;

  bool SetCommunicationOptions(CommunicationOptions const& communicationOptions);

  auto GetCommunicationOptions() -> std::optional<CommunicationOptions>;

  bool SetCounterOptions(CounterOptions const& counterOptions);

  auto GetCounterOptions() -> std::optional<CounterOptions>;

  auto GetCounterValue() -> std::optional<int32_t>;

  auto GetCounterEU() -> std::optional<int32_t>;

  auto GetStartStopMode() -> std::optional<bool>;

  auto GetCurrentMode() -> std::optional<ImpulseCounter30::eCurrentMode>;

  auto GetCodeErrNet() -> std::optional<uint8_t>;

  auto GetNameDevice() -> std::optional<std::string>;

  auto GetVersion() -> std::optional<std::string>;

  auto IsResetInput() -> std::optional<bool>;

  auto IsLockInput() -> std::optional<bool>;

  auto GetOutState1() -> std::optional<bool>;

  auto GetOutState2() -> std::optional<bool>;

  auto IsResetCount() -> std::optional<bool>;

  bool ResetCount();

  bool ControlCounterFromProgram(bool isEnabled);

  bool StartCounter(bool isStart);

  bool StopCounter(bool isStop);

private:
  class Impl;
  std::unique_ptr<Impl> pImpl;
};

std::ostream& operator<<(std::ostream& out, OWEN::ImpulseCounter30::CommunicationOptions const& communicationOptions);
std::ostream& operator<<(std::ostream& out, OWEN::ImpulseCounter30::CounterOptions const& counterOptions);

} /// end namespace OWEN