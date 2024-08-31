#include <iostream>
#include <array>
#include <string>
#include <string_view>
#include <map>

#include "ProcessingElementNetworkConfiguration.h"

using namespace std;

namespace {

  const std::array <NetworkAddress, kMaxNumberOfProcessingElements> NetworkAddressTable =
  {
    NetworkAddress(std::string("173.38.11.202") /* testmachine002 */ , "28016"),
    NetworkAddress(std::string("173.38.11.203") /* testmachine003 */ , "28017"),
    NetworkAddress(std::string("173.38.11.204") /* testmachine004 */ , "28018"),
    NetworkAddress(std::string("173.38.11.205") /* testmachine005 */ , "28019"),
    NetworkAddress(std::string("173.38.11.206") /* testmachine006 */ , "28020"),
    NetworkAddress(std::string("173.38.11.207") /* testmachine007 */ , "28021"),
    NetworkAddress(std::string("173.38.11.208") /* testmachine008 */ , "28022"),
    NetworkAddress(std::string("173.38.11.209") /* testmachine009 */ , "28023"),
    NetworkAddress(std::string("173.38.11.210") /* testmachine010 */ , "28024"),
    NetworkAddress(std::string("173.38.11.211") /* testmachine011 */ , "28025"),
    NetworkAddress(std::string("173.38.11.212") /* testmachine012 */ , "28026"),
    NetworkAddress(std::string("173.38.11.213") /* testmachine013 */ , "28027"),
    NetworkAddress(std::string("173.38.11.214") /* testmachine014 */ , "28028"),
    NetworkAddress(std::string("173.38.11.215") /* testmachine015 */ , "28029"),
    NetworkAddress(std::string("173.38.11.216") /* testmachine016 */ , "28030"),
    NetworkAddress(std::string("173.38.11.217") /* testmachine017 */ , "28031"),
    NetworkAddress(std::string("173.38.11.218") /* testmachine018 */ , "28032"),
    NetworkAddress(std::string("173.38.11.219") /* testmachine019 */ , "28033"),
    NetworkAddress(std::string("173.38.11.220") /* testmachine020 */ , "28034"),
    // MAIN application (scheduler)                                    
    NetworkAddress(std::string("173.38.11.201") /* testmachine001 */ , "28015"),
    // MAIN application (scheduler) -> DEBUG
    NetworkAddress(std::string("127.0.0.1")     /* localhost */, "28015"),
  };


  std::map<ProcessingElementIdentification, NetworkAddress> ProcessingElementToNetworkMapping =
  {
    {ProcessingElementIdentification::ProcessingElement_1, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_1)]},
    {ProcessingElementIdentification::ProcessingElement_2, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_2)]},
    {ProcessingElementIdentification::ProcessingElement_3, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_3)]},
    {ProcessingElementIdentification::ProcessingElement_4, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_4)]},
    {ProcessingElementIdentification::ProcessingElement_5, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_5)]},
    {ProcessingElementIdentification::ProcessingElement_6, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_6)]},
    {ProcessingElementIdentification::ProcessingElement_7, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_7)]},
    {ProcessingElementIdentification::ProcessingElement_8, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_8)]},
    {ProcessingElementIdentification::ProcessingElement_9, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_9)]},
    {ProcessingElementIdentification::ProcessingElement_10, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_10)]},
    {ProcessingElementIdentification::ProcessingElement_11, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_11)]},
    {ProcessingElementIdentification::ProcessingElement_12, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_12)]},
    {ProcessingElementIdentification::ProcessingElement_13, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_13)]},
    {ProcessingElementIdentification::ProcessingElement_14, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_14)]},
    {ProcessingElementIdentification::ProcessingElement_15, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_15)]},
    {ProcessingElementIdentification::ProcessingElement_16, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_16)]},
    {ProcessingElementIdentification::ProcessingElement_17, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_17)]},
    {ProcessingElementIdentification::ProcessingElement_18, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_18)]},
    {ProcessingElementIdentification::ProcessingElement_19, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_19)]},
    {ProcessingElementIdentification::ProcessingElement_20, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_20)]},
    {ProcessingElementIdentification::ProcessingElement_LOCALHOST, NetworkAddressTable[static_cast<unsigned char>(ProcessingElementIdentification::ProcessingElement_LOCALHOST)]},
  };

}; // end of anonymous namespace 


const std::map<ProcessingElementIdentification, NetworkAddress>& GetProcessingElementToNetworkMapping()
{
    std::cout << "2\n ";
  return ProcessingElementToNetworkMapping;
}
