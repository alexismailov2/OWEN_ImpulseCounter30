#include <OWEN/ImpulseCounter30.hpp>

#include <iostream>
#include <thread>

auto main(int argc, char** argv) -> int32_t
{
   auto impulseCounter = OWEN::ImpulseCounter30{argv[1], 115200, 16};
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

   while(1)
   {
     auto counterValue = impulseCounter.GetCounterValue().value();
     std::cout << "Counter value: " << std::dec << (counterValue - 1100) << std::endl;
     std::this_thread::sleep_for(std::chrono::milliseconds(500));
   }
   return 0;
}
