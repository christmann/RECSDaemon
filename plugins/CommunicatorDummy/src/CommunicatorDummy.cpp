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

#include "CommunicatorDummy.h"

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "daemon_msgs.h"

#define MEMORY_SIZE		4096
#define SLOTS			4

using namespace std;
using namespace log4cxx;

LoggerPtr CommunicatorDummy::logger;

void * CommunicatorDummy::create(PF_ObjectParams *) {
	return new CommunicatorDummy();
}

int32_t CommunicatorDummy::destroy(void * p) {
	if (!p)
		return -1;
	delete static_cast<CommunicatorDummy*>(p);
	return 0;
}

CommunicatorDummy::CommunicatorDummy() : mData(NULL) {
}

bool CommunicatorDummy::initInterface() {
	mData = (uint8_t*)malloc(MEMORY_SIZE);
	if (mData == NULL) {
		LOG4CXX_ERROR(logger, "Failed to allocate " << MEMORY_SIZE << " bytes");
		return false;
	}
	memset(mData, 0, MEMORY_SIZE);

	Daemon_Header hdr;
	hdr.magic[0] = 'R'; hdr.magic[1] = 'E'; hdr.magic[2] = 'C'; hdr.magic[3] = 'S';
	hdr.size = MEMORY_SIZE;
	hdr.baseboardID = 1;
	hdr.maxSlots = SLOTS;
	hdr.version = 3;
	hdr.chunksOffset = sizeof(Daemon_Header);
	hdr.dynamicAreaOffset = hdr.chunksOffset;
	memcpy((void*)mData, &hdr, sizeof(Daemon_Header));

	return true;
}

CommunicatorDummy::~CommunicatorDummy() {
	if (mData != NULL) {
		free(mData);
	}
}

size_t CommunicatorDummy::getMaxDataSize(void) {
	return MEMORY_SIZE;
}

ssize_t CommunicatorDummy::readData(size_t offset, void* buf, size_t count) {
	if (mData == NULL)
		return -3;

	if (offset + count > MEMORY_SIZE) {
		count = MEMORY_SIZE - offset;
	}

	memcpy(buf, &mData[offset], count);
	return count;
}

ssize_t CommunicatorDummy::writeData(size_t offset, const void* buf, size_t count) {
	if (mData == NULL)
		return -3;

	if (offset + count > MEMORY_SIZE) {
		count = MEMORY_SIZE - offset;
	}

	memcpy(&mData[offset], buf, count);

	parseData();

	return count;
}

void CommunicatorDummy::parseData() {
	for (int i = 0; i < SLOTS; ++i) {
		size_t messageOffset = sizeof(Daemon_Header);
		size_t messageMaxSize = MEMORY_SIZE - (messageOffset);
		if (SLOTS > 1) {
			// Determine my memory segment
			messageMaxSize /= SLOTS;
			messageOffset += (i * messageMaxSize);
		}

		mData[messageOffset] = 0; // Mark message as processed
	}
}
