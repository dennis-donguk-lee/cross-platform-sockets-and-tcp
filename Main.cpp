#pragma comment(lib, "Ws2_32.lib")
#include <WS2tcpip.h>
#include <iostream>

// Initialize Winsock.
int init_winsock()
{
	WSADATA wsa;
  return WSAStartup(MAKEWORD(2, 2), &wsa);
}

// Create a socket.
SOCKET create_socket(const int protocol)
{
    // UDP datagram socket type
    auto type = SOCK_DGRAM;
    // Handle TCP protocol.
    if (protocol == IPPROTO_TCP)
    {
        type = SOCK_STREAM;
    }

    auto res = socket(AF_INET, type, protocol);
    // Make it a non-blocking socket.
    if (res != INVALID_SOCKET && protocol == IPPROTO_TCP)
    {
        auto mode = 1ul;
        if (ioctlsocket(res, FIONBIO, &mode) != NO_ERROR)
        {
            closesocket(res);
            res = INVALID_SOCKET;
        }
    }

    // INVALID_SOCKET when failed in which case call WSAGetLastError()
    return res;
}

// Create an address.
sockaddr_in* create_address(const char* ip, const int port)
{
  // Allocate memory for the sockaddr in structure.
  const auto addr = new sockaddr_in;
  addr->sin_family = AF_INET;
  // Remember to go from host byte order to network byte order.
  addr->sin_port = htons(port);
 
  if (ip == nullptr)
  {
    // Bind to all IP addresses local machine may have.
    addr->sin_addr.S_un.S_addr = INADDR_ANY;
  }
  else
  {
    // Take dotted-decimal string and turn it into a 32-bit IP address,
    // in network byte order. Return INADDR_NONE on fail.
    //addr->sin_addr.S_un.S_addr = inet_addr(ip);
    inet_pton(addr->sin_family, ip, &(addr->sin_addr));
  }

  return addr;
  // Caller will be responsible for free().
}

int main()
{
  if (init_winsock() != 0)
  {
    std::cout << "WSAStartup failed. Error code: " << WSAGetLastError();
    exit(EXIT_FAILURE);
  }

  // Create a socket and an address.
  const auto sock = create_socket(IPPROTO_TCP);
  const auto addr = create_address("104.131.138.5", 8888);

  return 0;
}
