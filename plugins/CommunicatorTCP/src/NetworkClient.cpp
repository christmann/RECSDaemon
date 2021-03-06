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

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#ifndef WIN32
#include <errno.h>
#include <signal.h>
#else
#define WINVER 0x0501
#include <winsock2.h>
#include <Ws2tcpip.h>
#endif

#include "NetworkClient.h"

using namespace std;

LoggerPtr NetworkClient::logger;
uint32_t NetworkClient::mInstanceCounter = 0;

NetworkClient::NetworkClient(uint32_t address, uint16_t port, LoggerPtr log) {
	logger = log;
#ifdef WIN32
	if (mInstanceCounter == 0) {
		initWinsocks();
	}
#endif

#ifndef WIN32
    // We expect write failures to occur but we want to handle them where
    // the error occurs rather than in a SIGPIPE handler.
    signal(SIGPIPE, SIG_IGN);
#endif

	// Create socket
	//lint -e{641} Enum to int
	mSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mSocket == INVALID_SOCKET) {
		LOG_ERROR(logger, "Unable to create socket!");
	}

	// Set address and bind to socket
	mDestination.sin_family = PF_INET;
	mDestination.sin_port = htons(port);
	mDestination.sin_addr.s_addr = htonl(address);
	if (connect(mSocket, (struct sockaddr *) &mDestination, (socklen_t) sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
		LOG_ERROR(logger, "Unable to connect!");
	#ifdef WIN32
		closesocket(mSocket);
	#else
		close(mSocket);
	#endif
		mSocket = INVALID_SOCKET;
	}

	mInstanceCounter++;
}

NetworkClient::~NetworkClient() {
	if (mSocket != INVALID_SOCKET) {
#ifdef WIN32
		closesocket(mSocket);
#else
		close(mSocket);
#endif
		mSocket = INVALID_SOCKET;
	}

	mInstanceCounter--;
#ifdef WIN32
	if (mInstanceCounter == 0) {
		// On windows platform: Terminate winsocks
		WSACleanup();
	}
#endif
}

bool NetworkClient::sendData(string data) {
	return sendData(data.c_str(), data.length());
}

bool NetworkClient::sendData(const char* data, size_t length) {
	ssize_t cnt; // number of bytes sent

	// Send datagram
	//lint --e(713)
	cnt = send(mSocket, data, length, 0);
	if (cnt < 0) {
		LOG_ERROR(logger, "Error in send: " << errno);
		return false;
	}
	return true;
}

size_t NetworkClient::receiveData(char* data, size_t maxLength) {
	return (size_t) recv(mSocket, data, maxLength, 0);
}

//lint -efunc(818,Network::receiveData)
size_t NetworkClient::receiveData(char* data, size_t maxLength,
		struct timeval* timeout, uint32_t *addr, uint16_t *port) {
	fd_set setRead;
	int notused = 63;
	ssize_t cnt; // number of instance
	struct sockaddr_in sa;
	socklen_t sa_len = sizeof(sa);
	// UINT16 size;

	// Add application socket to list
	FD_ZERO(&setRead);
	FD_SET(mSocket, &setRead);

	// Perform select operation to block until a port is ready
	cnt = select(notused, &setRead, NULL, NULL, timeout);
	if (cnt == SOCKET_ERROR) {
		LOG_ERROR(logger, "select returned an error!");
	}

	// Check if at least one port has valid data
	if (cnt > 0) {
		// Check if this socket is ready
		if (FD_ISSET(mSocket, &setRead)) {
			// Read datagram
			memset(&sa, 0, sizeof(struct sockaddr_in));
			//lint --e(713)
			cnt = recvfrom(mSocket, data, maxLength, 0, (struct sockaddr *) &sa,
					&sa_len);
			if (cnt > 0) {
				if (addr != NULL) { // only perform assignment if a valid buffer was provided
					*addr = ntohl(sa.sin_addr.s_addr);
				}
				if (port != NULL) { // only perform assignment if a valid buffer was provided
					*port = ntohs(sa.sin_port);
				}
				return (size_t) cnt;
			}
		}
	} else {
		// No message!
		return (size_t) 0;
	}
	return (size_t) 0;
}

void NetworkClient::initWinsocks() {
	// On windows platform: Init winsocks
#ifdef WIN32
	WSADATA wsaData;
	WORD wVersionRequested;
	int err;

	wVersionRequested = 2; // 2.0 and above version of WinSock
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		LOG_ERROR(logger, "Could not find a usable WinSock DLL!");
	}
#endif
}

uint32_t NetworkClient::getRemoteIP() {
	return ntohl(mDestination.sin_addr.s_addr);
}

uint16_t NetworkClient::getRemotePort() {
	return ntohs(mDestination.sin_port);
}

uint32_t NetworkClient::getInterfaceIP() const {
	char ac[80];
	if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
		LOG_ERROR(logger, "Error getting local host name: " << errno);
		return 0;
	}
	LOG_INFO(logger, "Host name is " << ac);

	struct addrinfo hints, *result, *res;
	memset(&hints, 0, sizeof(hints));
	//lint -e{641} Enum to int
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;

	if (getaddrinfo(ac, NULL, &hints, &result) != 0) {
		LOG_ERROR(logger, "Error getting addresses: " << errno);
		return 0;
	}

	uint32_t ret = 0;
	for (res = result; res != NULL; res = res->ai_next) {
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		memcpy(&addr, res->ai_addr,
				static_cast<size_t>((uint16_t) (res->ai_addrlen)));
		printf("<2> [Network] getInterfaceIP: Address: %s\n",
				inet_ntoa(addr.sin_addr));
		ret = ntohl(addr.sin_addr.s_addr);
	}

	freeaddrinfo(result);

	return ret;
}

string NetworkClient::printIP(uint32_t ip) {
	char buffer[16];
	sprintf(buffer, "%u.%u.%u.%u", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
			(ip >> 8) & 0xFF, (ip & 0xFF));
	return string(buffer);
}

uint32_t NetworkClient::resolveHost(string host) {
	struct hostent *hp;
	struct sockaddr_in ip;

#ifdef WIN32
	if (mInstanceCounter == 0) {
		initWinsocks();
	}
#endif

	hp = gethostbyname(host.c_str());
	if (hp == NULL) {
		printf("<1> [Network] resolveHost: Could not resolve host!\n");
		return 0;
	}
	memset(&ip, 0, sizeof(ip));
	memcpy(&ip.sin_addr, hp->h_addr,
			static_cast<size_t>((uint16_t) (hp->h_length)));

	if (mInstanceCounter == 0) {
		// On windows platform: Terminate winsocks
#ifdef WIN32
		WSACleanup();
#endif
	}

	return ntohl(ip.sin_addr.s_addr);
}

bool NetworkClient::isConnected() const {
	return mSocket != INVALID_SOCKET;
}

void NetworkClient::closeSocket() {
	if (mSocket != INVALID_SOCKET) {
		shutdown(mSocket, 2);
#ifdef WIN32
		closesocket(mSocket);
#else
		close(mSocket);
#endif
		mSocket = INVALID_SOCKET;
	}
}
