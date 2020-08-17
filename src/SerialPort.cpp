#include "SerialPort.hpp"

#include <iostream>
#include <string>

SerialPort::SerialPort(std::string const& portPath, uint32_t baudrate)
  : _io{}
  , _port{_io}
  , _timer{_io}
{
  _port.open(portPath);
  _port.set_option(boost::asio::serial_port_base::baud_rate(baudrate));
}

void SerialPort::SendCommand(std::string const& data, SerialPort::tResponseCallback&& response, size_t timeoutResponseMs)
{
  boost::system::error_code errorCode;
  bool readError{true};
  std::string responseData(256, '\0');
  _port.async_read_some(boost::asio::buffer(responseData.data(), responseData.size()), [&](boost::system::error_code const& error, size_t bytes_transferred) {
    readError = (error || (bytes_transferred == 0));
    _timer.cancel();
    responseData.resize(bytes_transferred);
  });
  _timer.expires_from_now(boost::posix_time::milliseconds(timeoutResponseMs));
  _timer.async_wait([&](const boost::system::error_code& error) {
    if (error)
    {
      std::cout << error.message() << std::endl;
      return;
    }
    _port.cancel();
  });
  auto bytesTranfered = _port.write_some(boost::asio::buffer(data), errorCode);
  if (errorCode || (bytesTranfered != data.size()))
  {
    _io.reset();
    response({}, true);
    return;
  }
  _io.run();
  response(responseData, readError);
}
