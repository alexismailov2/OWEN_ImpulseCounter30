#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/deadline_timer.hpp>

class SerialPort
{
public:
  using tResponseCallback = std::function<void(std::string const&, bool error)>;
  using eParity = boost::asio::serial_port_base::parity::type;
  using eStopBits = boost::asio::serial_port_base::stop_bits::type;

public:
  SerialPort(std::string const& portPath,
             uint32_t baudrate,
             eParity parity = eParity::none,
             eStopBits = eStopBits::one,
             uint8_t characterSize = 8);

  SerialPort(SerialPort&&) = default;
  SerialPort& operator=(SerialPort&&) = default;

  void SendCommand(std::string const& data,
                   tResponseCallback && response = [](std::string const&, bool error){},
                   size_t timeoutResponseMs = 0);

private:
  boost::asio::io_service _io;
  boost::asio::serial_port _port;
  boost::asio::deadline_timer _timer;
};