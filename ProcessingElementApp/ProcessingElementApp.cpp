#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <set>
#include <assert.h>
#include <array>
#include <numeric>
#include <cstring>
#include <chrono>
#include <thread>
#include <fstream>
#include <iostream>


// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 3
#define DATA_LEN_FOR_PROCESSING 2097152/4
#define DEFAULT_PORT "27016"

#include <string.h>

#include <iostream>
#include <array>
#include <string>
#include <string_view>

#include "ProcessingElementNetworkConfiguration.h"
#include "InputArgumentChecker.h"



class AccumulateTask
{
public:
    // int accumulate();
    // int B;
    AccumulateTask(std::array<uint8_t, 4096> a) : arr(a) {}
    ~AccumulateTask() {}
    std::array<uint8_t, 4096> getArray()
    {
        return this->arr;
    }
    void setArray(std::array<uint8_t, 4096> a)
    {
        this->arr = a;
    }
    int getResult()
    {
        return this->result;
    }
    void setResult(int r)
    {
        this->result = r;
    }
private:
    std::array<uint8_t, 4096> arr;
    int result;
};

bool IsMainApplicationRequestReceived(const SOCKET& ClientSocket, const unsigned int TimeoutUsec = 1000 /*1ms*/)
{
  bool isResponseAvailable{ false };
  int iResult;

  fd_set fd;
  FD_ZERO(&fd);
  FD_SET(ClientSocket, &fd);

  timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = TimeoutUsec; // 1ms

  iResult = select(ClientSocket + 1, &fd, NULL, NULL, &tv);
  if (0 != iResult and SOCKET_ERROR != iResult)
  {
    std::cout << "Data on process element Timeout waiting for data" << std::endl;
    isResponseAvailable = true;
  }
  else if (iResult == 0)
  {
    std::cout << "Timeout waiting for data" << std::endl;
  }
  else if (iResult == SOCKET_ERROR)
  {
    iResult = WSAGetLastError();
    std::cout << "Winsock error code: " << iResult << std::endl;
    exit(1);
  }

  return isResponseAvailable;
}


class ProcessingElementServer
{
public:
  ProcessingElementServer(const NetworkAddress server_network_info, const bool is_debug_config)
  {
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
      std::cout << "WSAStartup failed with error: " << iResult << std::endl;
      exit(1);
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(server_network_info.m_ipAddress.c_str(), server_network_info.m_port.c_str(), &hints, &result);
    if (iResult != 0)
    {
      std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
      WSACleanup();
      exit(1);
    }

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
      std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
      freeaddrinfo(result);
      WSACleanup();
      exit(1);
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
      std::cout << "bind failed with error: " << WSAGetLastError() << std::endl;
      freeaddrinfo(result);
      closesocket(ListenSocket);
      WSACleanup();
      exit(1);
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
      std::cout << "listen failed with error: " << WSAGetLastError() << std::endl;
      closesocket(ListenSocket);
      WSACleanup();
      exit(1);
    }

    cout << "Processing Element Server Successfully started..." << std::endl;
    cout << "Processing Element Server IP: " << server_network_info.m_ipAddress << std::endl;
    cout << "Processing Element Server Port: " << server_network_info.m_port << std::endl;
  }

  ~ProcessingElementServer()
  {
    // No longer need server socket
    closesocket(ListenSocket);

    // Cleanup WSA env
    WSACleanup();
  }

  SOCKET&& AcceptMainApplicationConnection()
  {
    SOCKET ClientSocket = INVALID_SOCKET;

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET)
    {
      std::cout << "accept failed with error: " << WSAGetLastError() << std::endl;
      closesocket(ListenSocket);
      WSACleanup();
      exit(1);
    }

    dummy_cnt++;
    std::cout << "New main application conection received #" << dummy_cnt << std::endl;

    return std::move(ClientSocket);
  }

  void SendProcessingResponse()
  {
  }



private:

  WSADATA wsaData;
  int iResult;

  SOCKET ListenSocket = INVALID_SOCKET;

  struct addrinfo* result = NULL;
  struct addrinfo hints { 0, 0, 0, 0, 0, 0, 0 };

  int iSendResult;
  char recvbuf[DEFAULT_BUFLEN];
  int recvbuflen = DEFAULT_BUFLEN;

  int dummy_cnt{ 0 };
};


AccumulateTask* Accumulate_DeserializeInputBuffer(char* buffer)
{
    // return reinterpret_cast<AccumulateTask*>(buffer);
     // Pretpostavljamo da je buffer dovoljno velik da sadrži 4096 int-a
   // std::array<uint8_t, 4096> arr;
    // Kopiramo sadržaj buffer-a u arr
  //  std::memcpy(arr.data(), buffer, sizeof());
    // Kreiramo novi AccumulateTask objekat sa deserializovanim nizom
    return reinterpret_cast<AccumulateTask*>(buffer);
}

void Accumulate_PrintTaskInfo(std::array<uint8_t, 4096> data)
{
    // auto arr = task->getArray(); // Preuzmi niz iz zadatka
    for (const auto& element : data)
    {
        std::cout << element << " "; // Ispiši svaki element niza
    }
    std::cout << std::endl;
}


unsigned int Accumulate_ProcessTask(std::array<uint8_t, DEFAULT_BUFLEN> data)
{
    //std::this_thread::sleep_for(std::chrono::seconds(1u));
    //return std::accumulate(data.begin(), data.end(), 0);
    unsigned int r = 0;

    for (int i = 0; i < DEFAULT_BUFLEN; i++)
    {
        r += data[i];
    }
    return r;
}


std::chrono::duration<long double, std::milli> elapsed_seconds{ 0 };

std::map<unsigned int, unsigned int> ProcessFnHis(std::array<uint8_t, DATA_LEN_FOR_PROCESSING> buff, char* recvbuf, int buffer_len)
{
    std::map<unsigned int, unsigned int> histogram;
    const auto start_m{ std::chrono::steady_clock::now() };
    int r = 0;
    int u = 0;
    unsigned int taskType = (unsigned int)recvbuf[0];
   
    for (int j = 0; j < buffer_len; j++)
    {
        // r += test[i];
        histogram[j] = 0;
    }

    uint8_t tmp;

    for (int j = 0; j < buffer_len; j++)
    {
        tmp = buff[j];
        histogram[tmp] = histogram[tmp] + 1;
    }
   

    const auto end_main{ std::chrono::steady_clock::now() };
    elapsed_seconds = end_main - start_m;
    //std::cout << "PE execution time in `milliseconds: " << elapsed_seconds.count() << '\n';
    std::ofstream outfile;

    outfile.open("meassurements.txt", std::ios_base::app); // append instead of overwrite
    outfile << "[buffer len= " << buffer_len;
    outfile << " ]" << std::endl;
    outfile << "PE execution time in milliseconds: " << elapsed_seconds.count() << '\n';
    outfile.flush();
    return histogram;
}

unsigned int ProcessFnAndAcc(std::array<uint8_t, DATA_LEN_FOR_PROCESSING> buff, int buffer_len)
{
    const auto start_main{ std::chrono::steady_clock::now() };
   
    unsigned int r = 0;

    for (int i = 0; i < 16; i++)
    {
        for (int i = 0; i < DATA_LEN_FOR_PROCESSING; i++)
        {
            r += buff[i];
        }
    }
   

    // int * test = new int[len*BUFFER_SIZE_FOR_SEND];

    const auto end_main{ std::chrono::steady_clock::now() };
    elapsed_seconds = end_main - start_main;

    std::ofstream outfile;

    outfile.open("meassurements.txt", std::ios_base::app); // append instead of overwrite
    outfile << "[buffer len= " << buffer_len;
    outfile << " ]" << std::endl;
    outfile << "PE execution time in milliseconds: " << elapsed_seconds.count() << '\n';
    outfile.flush();
    return r;
}

// For minimal
unsigned int ProcessFnMin(std::array<uint8_t, DATA_LEN_FOR_PROCESSING> buff, char* recvbuf, int buffer_len)
{
   
    //unsigned int* p = (unsigned int*)recvbuf;

    unsigned int r = 108;
    unsigned int taskType = (unsigned int)recvbuf[0];
    unsigned int min_1 = (unsigned int)recvbuf[1];
    unsigned int min_2 = (unsigned int)recvbuf[2];
    std::cout << "type: " << (unsigned int)recvbuf[0] << " min1: " << (unsigned int)recvbuf[1] << " min2: " << (unsigned int)recvbuf[2] << "\n";
    std::cout << "type task: " << taskType << "\n";
    std::ofstream outfile;
    outfile.open("meassurements.txt", std::ios_base::app); // append instead of overwrite
    outfile << "type: " << (unsigned int)recvbuf[0] << " min1: " << (unsigned int)recvbuf[1] << " min2: " << (unsigned int)recvbuf[2] << "\n";
    // if task is leaf
    const auto start_main{ std::chrono::steady_clock::now() };

    if (0 == taskType)
    {
        for (int i = 0; i < 16; i++)
        {
            for (int i = 0; i < DATA_LEN_FOR_PROCESSING; i++)
            {
                if (buff[i] < r)
                {
                    r = buff[i];

                }
            }
        }
    }
    else
    {

        r = min_1 < min_2 ? min_1 : min_2;
    }
    // int * test = new int[len*BUFFER_SIZE_FOR_SEND];

    const auto end_main{ std::chrono::steady_clock::now() };
    elapsed_seconds = end_main - start_main;;

    std::cout << "PE execution time in milliseconds: " << elapsed_seconds.count() << '\n';
    outfile << "[buffer len= " << buffer_len;
    outfile << " ]" << std::endl;
  //  outfile << "type: " << (int)taskType << std::endl;
    outfile << "PE execution time in milliseconds: " << elapsed_seconds.count() << '\n';
    outfile.flush();
    return r;
}

using namespace std::chrono;
int __cdecl main(int argc, char** argv)
{
  int iResult = FALSE;

  SOCKET ListenSocket = INVALID_SOCKET;
  SOCKET ClientSocket = INVALID_SOCKET;

  struct addrinfo* result = NULL;

  int iSendResult;
  char recvbuf[DEFAULT_BUFLEN];
  memset(recvbuf, 0, DEFAULT_BUFLEN);
  uint64_t recvbuflen = DEFAULT_BUFLEN;

  // For minimal
  char recvbuf_min[DEFAULT_BUFLEN];
  memset(recvbuf_min, 0, DEFAULT_BUFLEN);

  char recvbuf_his[DATA_LEN_FOR_PROCESSING];
  memset(recvbuf_his, 0, DEFAULT_BUFLEN);

  std::array<uint8_t, DATA_LEN_FOR_PROCESSING> array;
  array.fill(0);
  array[0] = 1;
  array[1] = 1;
  for (int i = 0; i < array.size(); i++)
    {
        array[i] = i % 254;
    }

  // For minimal
  std::array<uint8_t, DATA_LEN_FOR_PROCESSING> array_min;
  for (int i = 0; i < array_min.size(); i++)
  {
      array_min[i] = 255;
  }
  array_min[5] = 77;
  InputArgumentsChecker iac(argc, argv);

  const auto& areAreInputArgumentsPlausable = iac.AreAreInputArgumentsPlausable();
  const auto& isDebugConfig = iac.IsDebugConfig();

  if (true != areAreInputArgumentsPlausable)
  {
    std::cout << "->INPUT ARGUMENTS NOT PLAUSABLE" << std::endl;
    return 1;
  }
  if (true == isDebugConfig)
  {
    std::cout << "->DEBUG CONFIG SELECTED" << std::endl;
  }

  const auto& processingElementId = GetProcessingElementIdFromInputArgument(argv[1]);
 
  const std::map<ProcessingElementIdentification, NetworkAddress>& processingElementToNetworkMapping = GetProcessingElementToNetworkMapping();
  const auto& processingElementNetworkAddress = processingElementToNetworkMapping.at(processingElementId);

  ProcessingElementServer processingElementServer(processingElementNetworkAddress, isDebugConfig);

  int dummy_switch_cnt{ 0 };
 
  // Run server loop
  for (;;)
  {
    SOCKET ClientSocket = processingElementServer.AcceptMainApplicationConnection();
   
    // Receive until the peer shuts down the connection
    do {

      iResult = recv(ClientSocket, (char *)recvbuf_his, recvbuflen, 0);
      
      // New task received
      if (iResult > 0)
      {
        //std::cout << "Bytes received:" << iResult << std::endl;
          std::cout << ">";
#if 0
        ProcessFnAndLog(recvbuf, recvbuflen);
        unsigned int sum = 1;
        std::cout << "sum = " << sum << std::endl;
        memcpy(recvbuf, &sum, sizeof(sum));
#endif


       /* std::array<uint8_t, DEFAULT_BUFLEN> rcv_data;
       rcv_data.fill(0);

       for (int i = 0; i < DEFAULT_BUFLEN; i++)
       {
            rcv_data[i] = data[i];
       }
       */
       
        std::map<unsigned int, unsigned int> sum = ProcessFnHis(array_min, recvbuf_min, DATA_LEN_FOR_PROCESSING);
       
      //  std::this_thread::sleep_for(std::chrono::milliseconds(500u));// std::cout << "sum = " << sum << std::endl;
        memcpy(recvbuf_min, &sum, sizeof(sum));
       // std::cout << "sum = " << (unsigned int)recvbuf[0] << std::endl;
        //int dummy_smth = 0;
        //std::cout << "ENTER DUMMY char for EXIT" << std::endl;
        //std::cin >> dummy_smth;

        std::cout << "<";
        // Send processed taks back to the sender
        iSendResult = send(ClientSocket, recvbuf_his, sizeof(char), 0);
        
        if (iSendResult == SOCKET_ERROR) {
          std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
          closesocket(ClientSocket);
          WSACleanup();
          return 1;
        }
        //std::cout << "Bytes sent: = " << iSendResult << std::endl;

       // MyClass_PrintTaskInfo(task);
      }
      else if (iResult == 0)
      {
        std::cout << "Connection closing..." << std::endl;
      }
      else
      {
        std::cout << "recv failed with error: %d" << WSAGetLastError() << std::endl;
        closesocket(ClientSocket);
        WSACleanup();
        //return 1;
        break;
      }

    } while (iResult > 0);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
      std::cout << "shutdown failed with error: " << WSAGetLastError() << std::endl;
      closesocket(ClientSocket);
      WSACleanup();
      //return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();
  }

  return 0;
}



