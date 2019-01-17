////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2017 christmann informationstechnik + medien GmbH & Co. KG
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
////////////////////////////////////////////////////////////////////////////////
// Author: Stefan Krupop <stefan.krupop@christmann.info>
////////////////////////////////////////////////////////////////////////////////

#ifndef NETWORK_H_
#define NETWORK_H_

#include <stdint.h>
#include <string>
#include <logger.h>

#ifdef WIN32
  #include <winsock2.h>
  typedef int socklen_t;
#else
  #include <arpa/inet.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/ioctl.h>
  #include <net/if.h>
  #include <net/if_arp.h>
  #include <netdb.h>
  typedef int SOCKET;
  #define SOCKET_ERROR -1
  #define INVALID_SOCKET -1
#endif

class Network {
public:
  Network(uint32_t address, uint16_t port);
  Network(uint16_t port);
  Network(SOCKET socket, struct sockaddr_in clientAddr);
  ~Network();

  Network* acceptConnection();

  bool sendData(std::string data);
  bool sendData(const char* data, size_t length);
  size_t receiveData(char* data, size_t maxLength);
  size_t receiveData(char* data, size_t maxLength, struct timeval* timeout, uint32_t *addr, uint16_t *port);
  uint32_t getLocalIP();
  uint32_t getRemoteIP();
  uint16_t getRemotePort();
  uint32_t getInterfaceIP() const;
  void setRemoteIP(uint32_t address);
  static std::string printIP(uint32_t ip);
  static uint32_t resolveHost(std::string host);
  bool isConnected() const;
  void closeSocket();

private:
  void construct(uint32_t address, uint16_t port);
  static void initWinsocks();

  SOCKET mSocket;
  struct sockaddr_in mSource;
  struct sockaddr_in mDestination;

  static uint32_t mInstanceCounter;
  static LoggerPtr logger;
};

#endif /*NETWORK_H_*/
