#include "InputArgumentChecker.h"

#include <algorithm>
#include <assert.h>

namespace {

  bool IsInputArgumentPlausable(const std::string& pe_id)
  {
    bool isPlausable = false;
    const std::array<std::string, kMaxNumberOfProcessingElements> processElementIds
      = { "pe_1", "pe_2" , "pe_3" , "pe_4" , "pe_5" , "pe_6" , "pe_7" , "pe_8" , "pe_9" ,"pe_10" ,
         "pe_11", "pe_12" , "pe_13" , "pe_14" , "pe_15" , "pe_16" , "pe_17" , "pe_18" , "pe_19" ,"pe_20",
        "debug" };

    const auto& iterator{ std::find(processElementIds.begin(), processElementIds.end(), pe_id) };

    if (iterator != processElementIds.end())
    {
      isPlausable = true;
    }

    return isPlausable;
  }

  void PrintUsageInfo()
  {
    std::cout << "usage: ProcessingElementApp [pe_[1,2,3,4...20] | [debug]] " << std::endl;
    std::cout << "example1: ProcessingElementApp pe_1 " << std::endl;
    std::cout << "example2: ProcessingElementApp debug " << std::endl;
    std::cout << "debug info: ip=127.0.0.1 port=28015" << std::endl;
  }

}; // end of anonymous namespace


InputArgumentsChecker::InputArgumentsChecker(int argc, char** argv)
{
  if ((argc != 2) || (false == IsInputArgumentPlausable(std::string(argv[1]))))
  {
    m_AreInputArgumentsPlausable = false;
    PrintUsageInfo();
  }
  else
  {
    m_AreInputArgumentsPlausable = true;

    if (std::string("debug") == std::string(argv[1]))
    {
      m_IsDebugConfig = true;
    }
  }
}

namespace
{
  constexpr unsigned char kInputArgumentNameOffset{ 3u }; //pe_1 -> "pe_" = 3
};//end of anonymous namespace

ProcessingElementIdentification GetProcessingElementIdFromInputArgument(const char* input_arg)
{
  ProcessingElementIdentification processing_element_id{ ProcessingElementIdentification::ProcessingElement_LOCALHOST };

  if (std::string("debug") != std::string(input_arg))
  {
    unsigned int pe_id = atoi((input_arg + kInputArgumentNameOffset));
    assert(pe_id > 0 && pe_id < kMaxNumberOfProcessingElements); // "PROCESS ELEMENT ID NOT PLAUSABLE
    processing_element_id = static_cast<ProcessingElementIdentification>(pe_id - 1);
  }
  return processing_element_id;
}


