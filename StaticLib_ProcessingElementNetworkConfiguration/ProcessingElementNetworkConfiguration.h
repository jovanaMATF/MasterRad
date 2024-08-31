#pragma once

#include <iostream>
#include <array>
#include <string>
#include <string_view>
#include <map>
#include <stdlib.h>
#include <stdio.h>

constexpr unsigned char kMaxNumberOfProcessingElements{ 21u };

enum class ProcessingElementIdentification : char
{
  ProcessingElement_1 = 0,
  ProcessingElement_2,
  ProcessingElement_3,
  ProcessingElement_4,
  ProcessingElement_5,
  ProcessingElement_6,
  ProcessingElement_7,
  ProcessingElement_8,
  ProcessingElement_9,
  ProcessingElement_10,
  ProcessingElement_11,
  ProcessingElement_12,
  ProcessingElement_13,
  ProcessingElement_14,
  ProcessingElement_15,
  ProcessingElement_16,
  ProcessingElement_17,
  ProcessingElement_18,
  ProcessingElement_19,
  ProcessingElement_20, // Used as main application for task scheduling
  ProcessingElement_LOCALHOST, // Debug
};

typedef struct NetworkAddress
{
  NetworkAddress() = default;
  NetworkAddress(
    const std::string ip_address,
    const std::string port)
    : m_ipAddress(ip_address),
    m_port(port)
  {};
  std::string m_ipAddress;
  std::string m_port;
} NetworkAddress;

const std::map<ProcessingElementIdentification, NetworkAddress>& GetProcessingElementToNetworkMapping();