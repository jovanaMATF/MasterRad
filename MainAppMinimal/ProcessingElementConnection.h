#pragma once
#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <array>
#include <vector>
#include <algorithm> 
#include <iostream>
#include <numeric>
#include <assert.h>

using namespace std;

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include "InputArgumentChecker.h"
#include "ProcessingElementNetworkConfiguration.h"

namespace
{
    constexpr char kInvalidProcessingElementIdentification{ -1 };
}; // end of anonymous namespace

template<int RCV_BUFLEN>
class ProcessingElementConnection
{

public:

    ProcessingElementConnection() = default;

    void ConnectToProcessingElement(const NetworkAddress server_network_info, const ProcessingElementIdentification pe_id)
    {
        //Init pe id     
        m_pe_id = pe_id;
        std::cout << "7  " << (int)m_pe_id << "\n";
        // Initialize Winsock
        iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            std::cout << "WSAStartup failed with error: " << iResult << std::endl;
            exit(1);
        }

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        std::cout << "8\n ";
        // Resolve the server address and port
        iResult = getaddrinfo(server_network_info.m_ipAddress.c_str(), server_network_info.m_port.c_str(), &hints, &result);
        if (iResult != 0)
        {
            std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
            WSACleanup();
            exit(1);
        }
        std::cout << "9\n ";
        // Attempt to connect to an address until one succeeds
        for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

            // Create a SOCKET for connecting to server
            std::cout << "10 " << ptr->ai_socktype << " " << ptr->ai_protocol << "\n";
            ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                ptr->ai_protocol);
            if (ConnectSocket == INVALID_SOCKET) {
                std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
                WSACleanup();
                exit(1);
            }

            // Connect to server.
            std::cout << "11\n ";
            std::cout << "ADDRESS: " << server_network_info.m_ipAddress << " " << (int)ptr->ai_addrlen << "\n";
            iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
            if (iResult == SOCKET_ERROR) {
                closesocket(ConnectSocket);
                ConnectSocket = INVALID_SOCKET;
                continue;
            }
            break;
        }
     
        freeaddrinfo(result);

        if (ConnectSocket == INVALID_SOCKET) {
            std::cout << "Unable to connect to server! " << std::endl;
            WSACleanup();
            exit(1);
        }
        else
        {
            cout << "Successfully connected to ProcessingElement_" << static_cast<int>(m_pe_id) + 1 << std::endl;
            cout << "ProcessingElement_" << static_cast<int>(m_pe_id) + 1 << " Server IP: " << server_network_info.m_ipAddress << std::endl;
            cout << "ProcessingElement_" << static_cast<int>(m_pe_id) + 1 << " Port: " << server_network_info.m_port << std::endl;
        }
    }

    void SendData(const char* buffer, int buff_len) const
    {
        int iResult = send(ConnectSocket, buffer, buff_len, 0);

        std::cout << ">";

        if (iResult == SOCKET_ERROR)
        {
            std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
            closesocket(ConnectSocket);
            WSACleanup();
            exit(1);
        }

        //std::cout << "Bytes Sent to ProcessingElement_" << static_cast<int>(m_pe_id) + 1 << ": " << iResult << std::endl;
    }

    int ReceiveData(char* buff, int len)
    {
        assert(len == RCV_BUFLEN);

        unsigned int received_data = 0;

        do {
            iResult = recv(ConnectSocket, buff + received_data, RCV_BUFLEN - received_data, 0);

            if (iResult > 0)
            {
              //  std::cout << "Bytes received:" << iResult << std::endl;
                std::cout << "<";
                received_data += iResult;

            }
            else if (iResult == 0)
            {
                std::cout << "Connection closing..." << std::endl;
            }
            else
            {
                std::cout << "recv failed with error: %d" << WSAGetLastError() << std::endl;
                closesocket(ConnectSocket);
                WSACleanup();
                exit(1);
            }
        } while (received_data != RCV_BUFLEN);

        return received_data;
    }

    SOCKET GetConnectSocket() const
    {
        return ConnectSocket;
    }

    ~ProcessingElementConnection()
    {
        // cleanup
        closesocket(ConnectSocket);
        WSACleanup();
    }

private:

    ProcessingElementIdentification m_pe_id = static_cast<ProcessingElementIdentification>(-1);

    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL, * ptr = NULL, hints;
    char sendbuf[RCV_BUFLEN];
    char recvbuf[RCV_BUFLEN];
    int iResult;
    int recvbuflen = RCV_BUFLEN;
};

