#pragma comment(lib, "Ws2_32.lib")

#include <WS2tcpip.h>
#include <iostream>

// Initialize Winsock.
int init_winsock()
{
	WSADATA wsa;
  return WSAStartup(MAKEWORD(2, 2), &wsa);
}

SOCKET create_socket(const int protocol)
{
    // UDP datagram socket type
    auto type = SOCK_DGRAM;
    // Handle TCP protocol.
    if (protocol == IPPROTO_TCP)
    {
        type = SOCK_STREAM;
    }

    // Return INVALID_SOCKET when failed in which case call WSAGetLastError().
    auto res = INVALID_SOCKET;
    res = socket(AF_INET, type, protocol);

    return res;
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
  //const auto addr = CreateAddress("104.131.138.5", PORT);

  return 0;
}
