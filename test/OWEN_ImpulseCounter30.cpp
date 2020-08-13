#include <OWEN/ImpulseCounter30.hpp>

#include <iostream>

auto main(int argc, char** argv) -> int32_t
{
   auto impulseCounter = OWEN::ImpulseCounter30{"/dev/cu.usbserial-14410", 115200, };
   auto const communicationOptions = impulseCounter.GetCommunicationOptions();
   if (communicationOptions.has_value())
   {
     std::cout << communicationOptions.value() << std::endl;
   }
   else
   {
     std::cout << "error" << std::endl;
   }

   auto const counterOptions = impulseCounter.GetCounterOptions();
   if (counterOptions.has_value())
   {
     std::cout << counterOptions.value() << std::endl;
   }
   else
   {
     std::cout << "error" << std::endl;
   }
   return 0;
}
