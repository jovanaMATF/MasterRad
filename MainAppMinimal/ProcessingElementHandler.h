#pragma once
#pragma once

#define WIN32_LEAN_AND_MEAN

#include "ProcessingElementConnection.h"

namespace
{
    constexpr char kInvalidMaxSocket{ -1 };
}; // end of anonymous namespace

template<int NUMBER_OF_PROCESSING_ELEMENTS, int DEFAULT_BUFLEN = 262144>
class ProcessingElementHandler
{
    using ProcessingElementNetworkAddressMap = std::map<ProcessingElementIdentification, NetworkAddress>;
    using ProcessingElementConnectionMap = std::map<ProcessingElementIdentification, ProcessingElementConnection<DEFAULT_BUFLEN>>;

public:
    ProcessingElementHandler(const ProcessingElementNetworkAddressMap& processingElementNetworkAddressMap)
        :m_processingElementNetworkAddressMap(processingElementNetworkAddressMap)
    {
        // used to find max socket (needed for select function later in the system) 
        std::vector<SOCKET> tmp_sockets;

        // Init connectiosn with not connected objects 
        for (int i = 0; i < NUMBER_OF_PROCESSING_ELEMENTS; ++i)
        {
            // Construct pe_id
            const ProcessingElementIdentification pe_id = static_cast<ProcessingElementIdentification>(i);

            // Init map with not connection conection

            const auto& pe_connection{ ProcessingElementConnection<DEFAULT_BUFLEN>() };
            m_processingElementConnectionMap.emplace(std::make_pair(pe_id, pe_connection));

            // Tmp vector used for max_element search
            tmp_sockets.push_back(pe_connection.GetConnectSocket());

            // Initialize pending requests (needed for checking on pending responses from pe)
            m_pendingResponsesFromPE.emplace(std::make_pair(pe_id, false));
        }
        // Get max socket 
        const auto& max_elem_iter = std::max_element(tmp_sockets.begin(), tmp_sockets.end());
        m_maxSocket = *max_elem_iter;
    }

    void ConnectToProcessingElements()
    {
        // Connect to all PEs 
        std::cout << "5\n ";
        for (int i = 0; i < NUMBER_OF_PROCESSING_ELEMENTS; ++i)
        {
            // Construct pe_id
            const ProcessingElementIdentification pe_id = static_cast<ProcessingElementIdentification>(i);

            // Get PE Network Address
            const auto& processingElementNetworkAddress = m_processingElementNetworkAddressMap.at(pe_id);
            std::cout << "6 " << (int)pe_id;
            // Connect to server
            m_processingElementConnectionMap.at(pe_id).ConnectToProcessingElement(processingElementNetworkAddress, pe_id);
        }
    }

    bool IsResponseFromAnyProcessingElementsAvailable()
    {
        bool isResponseAvailable{ false };
        int iResult;
        const unsigned int TimeoutUsec = 100000; /*100ms*/

        fd_set fd;
        FD_ZERO(&fd);

        for (const auto& pe_connection : m_processingElementConnectionMap)
        {
            SOCKET s = pe_connection.second.GetConnectSocket();
            FD_SET(s, &fd);
        }

        timeval tv{ 0,0 };
        tv.tv_sec = 0;
        tv.tv_usec = TimeoutUsec;

        iResult = select(m_maxSocket + 1, &fd, NULL, NULL, &tv);
        if (0 != iResult and SOCKET_ERROR != iResult)
        {
            // std::cout << "Data avaialble" << std::endl;
            isResponseAvailable = true;

            //Update pending requests
            for (const auto& pe_connection : m_processingElementConnectionMap)
            {
                if (FD_ISSET(pe_connection.second.GetConnectSocket(), &fd))
                {
                    m_pendingResponsesFromPE[pe_connection.first] = true;
                }
            }
        }
        else if (iResult == 0)
        {
            //std::cout << "Timeout waiting for data" << std::endl;
            std::cout << "T";
        }
        else if (iResult == SOCKET_ERROR)
        {
            iResult = WSAGetLastError();
            std::cout << "Winsock error code: " << iResult << std::endl;
            exit(1);
        }

        return isResponseAvailable;
    }

    bool IsResponseFromProcessingElementAvailable(const ProcessingElementIdentification& pe_id) const
    {
        return m_pendingResponsesFromPE.at(pe_id);
    }

    int ReadDataFromProcessingElement(const ProcessingElementIdentification& pe_id, char* buff, int len)
    {
        // Reset pending data
        m_pendingResponsesFromPE[pe_id] = false;

        // Receive data and store it in buff[len] 
        // Returns the number of received bytes
        return m_processingElementConnectionMap.at(pe_id).ReceiveData(buff, len);
    }

    void SendDataToProcessingElement(const ProcessingElementIdentification& pe_id, char* buff, int len)
    {
        m_processingElementConnectionMap.at(pe_id).SendData(buff, len);
    }

    const ProcessingElementConnection<DEFAULT_BUFLEN>& GetProcessingElementConnection(const ProcessingElementIdentification pe_id)
    {
        return m_processingElementConnectionMap.at(pe_id);
    }

private:

    int m_maxSocket{ kInvalidMaxSocket };
    std::map<ProcessingElementIdentification, bool> m_pendingResponsesFromPE;
    const ProcessingElementNetworkAddressMap& m_processingElementNetworkAddressMap;
    ProcessingElementConnectionMap m_processingElementConnectionMap;
};