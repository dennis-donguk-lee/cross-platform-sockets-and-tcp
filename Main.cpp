#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#include <WS2tcpip.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define SOCKET int
#define INVALID_SOCKET -1
#define NO_ERROR 0L
#define SOCKET_ERROR -1
#endif

#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>

#ifdef _WIN32
// Initialize Winsock.
int init_winsock()
{
  WSADATA wsa;
  return WSAStartup(MAKEWORD(2, 2), &wsa);
}
#endif

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
#ifdef _WIN32
    const auto err = ioctlsocket(res, FIONBIO, &mode);
#else
    const auto err = ioctl(res, FIONBIO, &mode);
#endif
    if (err != NO_ERROR)
    {
#ifdef _WIN32
      closesocket(res);
#else
      close(res);
#endif
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
#ifdef _WIN32
    addr->sin_addr.S_un.S_addr = INADDR_ANY;
#endif
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

int connect_socket_and_address(const SOCKET sock, sockaddr_in* addr)
{
  if (
    connect(sock, reinterpret_cast<sockaddr*>(addr), sizeof(sockaddr_in))
    == SOCKET_ERROR
    )
  {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return -1;
#endif
  }

  return 0;
}

int send_tcp(const SOCKET sock, char const* buf, const int len)
{
  const auto res = send(sock, buf, len, 0);

  if (res == SOCKET_ERROR)
  {
    return -1;
  }

  return res;
}

int receive_tcp(const SOCKET sock, char* buf, const int len)
{
  const auto size = recv(sock, buf, len, 0);
  if (size == SOCKET_ERROR)
  {
    return -1;
  }

  return size;
}


#define EMTU 1500 // Ethernet MTU size
int main(int argc, char** argv)
{
#ifdef _WIN32
  if (init_winsock() != 0)
  {
    std::cout << "WSAStartup failed. Error code: " << WSAGetLastError();
    exit(EXIT_FAILURE);
  }
#endif

  // Create a socket and an address.
  const auto sock = create_socket(IPPROTO_TCP);
  const auto addr = create_address("104.131.138.5", 8888);

  // Connect the socket and the address.
  auto err = connect_socket_and_address(sock, addr);
  while (err == SOCKET_ERROR)
  {
    err = connect_socket_and_address(sock, addr);
  }
  delete addr;

  // Get the sending buffer.
  std::string sendbuf = argv[0];
  const auto pos = sendbuf.find_last_of('\\');
  sendbuf = sendbuf.substr(pos + 1);
  const auto csendbuf = sendbuf.c_str();

  // Send data over TCP.
  auto sendlen = send_tcp(sock, csendbuf, sendbuf.length());
  while (sendlen == SOCKET_ERROR)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << '.';
    sendlen = send_tcp(sock, csendbuf, sendbuf.length());
  }

  // Listen for a response.
  std::string res;
  char recvbuf[EMTU]{};
  auto recvlen = receive_tcp(sock, recvbuf, sizeof(recvbuf));
  res += recvbuf;
  while (recvlen != 0)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << '.';
    for (auto& ch : recvbuf)
    {
      ch = 0;
    }
    recvlen = receive_tcp(sock, recvbuf, sizeof(recvbuf));
    res += recvbuf;
  }

  // clean-ups
#ifdef _WIN32
      closesocket(sock);
#else
      close(sock);
#endif
#ifdef _WIN32
  WSACleanup();
#endif

  std::cout << res.c_str() << std::endl;
#ifdef _WIN32
  std::getchar();
#endif

  return 0;
}
