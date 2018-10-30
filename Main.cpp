#pragma comment(lib, "Ws2_32.lib")

#include <WS2tcpip.h>
#include <iostream>

// Initialize Winsock.
int init_winsock()
{
	WSADATA wsa;
  return WSAStartup(MAKEWORD(2, 2), &wsa);
}

int main()
{
  if (init_winsock() != 0)
  {
    std::cout << "WSAStartup failed. Error code: " << WSAGetLastError();
    exit(EXIT_FAILURE);
  }

  return 0;
}
