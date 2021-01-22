#include <OWEN/ImpulseCounter30.hpp>
#include "TimeMeasuring.hpp"

#include <iostream>
#include <thread>
#include <fstream>

auto main(int argc, char** argv) -> int32_t
{
   auto communicationOptions = OWEN::ImpulseCounter30::CommunicationOptions{};
   communicationOptions.PortPath(argv[1])
           .BaudeRate(OWEN::ImpulseCounter30::CommunicationOptions::eBaudrate::_115200bps)
           .Parity(OWEN::ImpulseCounter30::CommunicationOptions::eParity::NO)
           .StopBits(false)
           .DataBits(true)
           .BaseAddr(16);
   auto impulseCounter = OWEN::ImpulseCounter30{communicationOptions};
   auto const communicationOptionsGotten = impulseCounter.GetCommunicationOptions();
   if (communicationOptionsGotten.has_value())
   {
     std::cout << communicationOptionsGotten.value() << std::endl;
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

   auto file = std::ofstream("testOVENSI30_.txt");
   while(1)
   {
     TAKEN_TIME();
     auto counterValue = impulseCounter.GetCounterValue().value();
     auto counterEU = impulseCounter.GetCounterEU().value();
     std::cout << "Counter EU:" << std::dec << counterEU << std::endl;
     std::cout << "Counter value: " << std::dec << (counterValue - 1100) << std::endl;
     //std::this_thread::sleep_for(std::chrono::milliseconds(500));
     file << counterEU << '\t' << counterValue << std::endl;
   }
   return 0;
}
