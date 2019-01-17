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

#include "CommunicatorTCP.h"

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "daemon_msgs.h"

#define MEMORY_SIZE		65535
#define SLOTS			1

using namespace std;

LoggerPtr CommunicatorTCP::logger;
IConfig* CommunicatorTCP::config;
PF_InvokeServiceFunc CommunicatorTCP::invokeService;

void * CommunicatorTCP::create(PF_ObjectParams *) {
	return new CommunicatorTCP();
}

int32_t CommunicatorTCP::destroy(void * p) {
	if (!p)
		return -1;
	delete static_cast<CommunicatorTCP*>(p);
	return 0;
}

CommunicatorTCP::CommunicatorTCP() : mBaseboardID(0), mClient(NULL), mData(NULL), mReceiveBuffer(NULL), mReceiveBufferPos(0) {
}

bool CommunicatorTCP::initInterface() {
	mBaseboardID = config->GetInt("Comm", "baseboard", -1);
	if (mBaseboardID == -1) {
		LOG_ERROR(logger, "No baseboard ID configured (Comm->baseboard)");
		return false;
	}

	if (!tryConnect()) {
		return false;
	}

	mData = (uint8_t*)malloc(MEMORY_SIZE);
	if (mData == NULL) {
		LOG_ERROR(logger, "Failed to allocate " << MEMORY_SIZE << " bytes shared memory buffer");
		return false;
	}
	memset(mData, 0, MEMORY_SIZE);
	mReceiveBuffer = (uint8_t*)malloc(MEMORY_SIZE);
	if (mReceiveBuffer == NULL) {
		LOG_ERROR(logger, "Failed to allocate " << MEMORY_SIZE << " bytes receive buffer");
		return false;
	}

	Daemon_Header hdr;
	hdr.magic[0] = 'R'; hdr.magic[1] = 'E'; hdr.magic[2] = 'C'; hdr.magic[3] = 'S';
	hdr.size = MEMORY_SIZE;
	hdr.baseboardID = mBaseboardID;
	hdr.maxSlots = SLOTS;
	hdr.version = 3;
	hdr.chunksOffset = sizeof(Daemon_Header);
	hdr.dynamicAreaOffset = hdr.chunksOffset;
	memcpy((void*)mData, &hdr, sizeof(Daemon_Header));

	return true;
}

CommunicatorTCP::~CommunicatorTCP() {
	if (mData != NULL) {
		free(mData);
	}
	if (mReceiveBuffer != NULL) {
		free(mReceiveBuffer);
	}
	if (mClient != NULL) {
		delete mClient;
		mClient = NULL;
	}
}

size_t CommunicatorTCP::getMaxDataSize(void) {
	return MEMORY_SIZE;
}

ssize_t CommunicatorTCP::readData(size_t offset, void* buf, size_t count) {
	if (mData == NULL)
		return -3;

	if (offset + count > MEMORY_SIZE) {
		count = MEMORY_SIZE - offset;
	}

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 500 * 1000;
	if (mClient != NULL) {
		size_t read = mClient->receiveData((char*)&mReceiveBuffer[mReceiveBufferPos], MEMORY_SIZE - mReceiveBufferPos, &timeout, NULL, NULL);
		if (read > 0) {
			mReceiveBufferPos += read;

			if (mReceiveBufferPos > sizeof(TCP_Message_Header)) {
				if (mReceiveBuffer[0] == 'R' && mReceiveBuffer[1] == 'E' && mReceiveBuffer[2] == 'C' && mReceiveBuffer[3] == 'S') {
					size_t length = (mReceiveBuffer[6] << 8) | mReceiveBuffer[7];
					if (mReceiveBufferPos >= sizeof(TCP_Message_Header) + length) {
						// Packet received
						memcpy(&mData[sizeof(Daemon_Header)], &mReceiveBuffer[sizeof(TCP_Message_Header)], length);

						// Move remaining data to beginning of buffer
						memcpy(mReceiveBuffer, &mReceiveBuffer[sizeof(TCP_Message_Header) + length], mReceiveBufferPos - (sizeof(TCP_Message_Header) + length));
						mReceiveBufferPos -= (sizeof(TCP_Message_Header) + length);
					}
				} else {
					// Invalid packet, reset buffer
					mReceiveBufferPos = 0;
				}
			}
		}
	}

	memcpy(buf, &mData[offset], count);
	return count;
}

ssize_t CommunicatorTCP::writeData(size_t offset, const void* buf, size_t count) {
	if (mData == NULL)
		return -3;

	if (offset + count > MEMORY_SIZE) {
		count = MEMORY_SIZE - offset;
	}

	memcpy(&mData[offset], buf, count);

	int messageOffset = sizeof(Daemon_Header);
	if (mData[messageOffset] != 0) { // MessageType != 0 -> Message available
		if (mClient != NULL) {
			TCP_Message_Header hdr;
			hdr.magic[0] = 'R'; hdr.magic[1] = 'E'; hdr.magic[2] = 'C'; hdr.magic[3] = 'S';
			hdr.baseboard = mBaseboardID;
			hdr.node = 0;
			if (invokeService != NULL) {
				int8_t slot;
				invokeService((const uint8_t *)"getSlot", &slot);
				//LOG_INFO(logger, "Got slot " << (uint32_t)slot << " from daemon");
				hdr.node = slot;
			}
			hdr.size = htons(count);
			if (mClient->sendData((char*)&hdr, sizeof(TCP_Message_Header))) {
				if (!mClient->sendData((char*)&mData[offset], count)) {
					delete mClient;
					mClient = NULL;
				}
			} else {
				delete mClient;
				mClient = NULL;
			}
		} else {
			LOG_INFO(logger, "Trying to reconnect to controller...");
			if (tryConnect()) {
				if (invokeService != NULL) {
					invokeService((const uint8_t *)"resetStatemachine", NULL);
				}
			}
		}
		mData[messageOffset] = 0; // Mark message as processed
	}

	return count;
}

bool CommunicatorTCP::tryConnect() {
	string controller = config->GetString("Comm", "controller", "");
	if (controller.empty()) {
		LOG_ERROR(logger, "Controller IP address not configured (Comm->controller)");
		return false;
	}
	int port = config->GetInt("Comm", "port", 2022);
	if (mClient != NULL) {
		delete mClient;
		mClient = NULL;
	}
	mClient = new NetworkClient(NetworkClient::resolveHost(controller), port, logger);
	if (mClient == NULL || !mClient->isConnected()) {
		LOG_ERROR(logger, "Could not connect to controller");
		delete mClient;
		mClient = NULL;
		return false;
	}
	LOG_INFO(logger, "Connected to controller");
	return true;
}
