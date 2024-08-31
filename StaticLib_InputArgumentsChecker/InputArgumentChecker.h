#pragma once

#include <iostream>
#include <array>
#include <string>
#include <string_view>
#include <map>
#include <stdlib.h>
#include <stdio.h>

#include "ProcessingElementNetworkConfiguration.h"

using namespace std;

ProcessingElementIdentification GetProcessingElementIdFromInputArgument(const char* input_arg);

class InputArgumentsChecker
{
public:

  InputArgumentsChecker(int argc, char** argv);

  bool AreAreInputArgumentsPlausable() const
  {
    return m_AreInputArgumentsPlausable;
  }

  bool IsDebugConfig() const
  {
    return m_IsDebugConfig;
  }

private:
  bool m_AreInputArgumentsPlausable{ false };
  bool m_IsDebugConfig{ false };
};