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

#include "Communicator.h"

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include "daemon_msgs.h"

#define I2C_ADDRESS			0x54
#define I2C_ADDRESS_OLD1	0x44
#define I2C_ADDRESS_OLD2	0x50
#define I2C_PATH		"/dev/i2c-%d"

using namespace std;
using namespace log4cxx;

LoggerPtr Communicator::logger;
IConfig* Communicator::config;

void * Communicator::create(PF_ObjectParams *) {
	return new Communicator();
}

int32_t Communicator::destroy(void * p) {
	if (!p)
		return -1;
	delete static_cast<Communicator*>(p);
	return 0;
}

Communicator::Communicator() : mHandle(0), mI2CAddress(I2C_ADDRESS) {
}

bool Communicator::initInterface() {
	char filename[16];

	int i2cNr = config->GetInt("Comm", "i2cBus", 0);

	sprintf(&filename[0], I2C_PATH, i2cNr);

	mHandle = open(filename, O_RDWR);
	if (mHandle < 0) {
		LOG4CXX_ERROR(logger, "Failed to open I2C device " << filename << ". Code " << errno);
		return false;
	}

	int addressed = ioctl(mHandle, I2C_SLAVE, mI2CAddress);
	if (addressed < 0) {
		LOG4CXX_ERROR(logger, "Failed to set slave address 0x" << hex << mI2CAddress << ", error " << errno << " (" << strerror(errno) << ")");
		return false;
	}
	return true;
}

Communicator::~Communicator() {
	if (mHandle > 0) {
		close(mHandle);
	}
}

size_t Communicator::getMaxDataSize(void) {
	Daemon_Header hdr;

	hdr.size = 0;
	readData(0, &hdr, sizeof(Daemon_Header));

	if (hdr.size == 0) { // No valid header found, maybe on old address?
		mI2CAddress = I2C_ADDRESS_OLD1;
		int addressed = ioctl(mHandle, I2C_SLAVE, mI2CAddress);
		if (addressed < 0) {
			LOG4CXX_ERROR(logger, "Failed to set slave address 0x" << hex << mI2CAddress << ", error " << errno << " (" << strerror(errno) << ")");
		}

		readData(0, &hdr, sizeof(Daemon_Header));
		if (hdr.size == 0) { // Still not found, try even older one
			mI2CAddress = I2C_ADDRESS_OLD2;
			int addressed = ioctl(mHandle, I2C_SLAVE, mI2CAddress);
			if (addressed < 0) {
				LOG4CXX_ERROR(logger, "Failed to set slave address 0x" << hex << mI2CAddress << ", error " << errno << " (" << strerror(errno) << ")");
			}
			readData(0, &hdr, sizeof(Daemon_Header));
			if (hdr.size == 0) { // Still not found, change back address
				mI2CAddress = I2C_ADDRESS;
				int addressed = ioctl(mHandle, I2C_SLAVE, mI2CAddress);
				if (addressed < 0) {
					LOG4CXX_ERROR(logger, "Failed to set slave address 0x" << hex << mI2CAddress << ", error " << errno << " (" << strerror(errno) << ")");
				}
			}
		}
	}

	return hdr.size;
}

ssize_t Communicator::readData(size_t offset, void* buf, size_t count) {
	if (mHandle <= 0)
		return -3;

	// Write offset
	char temp[2];
	temp[0] = (offset >> 8) & 0xff;
	temp[1] = offset & 0xff;
	int bytesWritten = write(mHandle, &temp[0], 2);
	if (bytesWritten < 0) {
		LOG4CXX_ERROR(logger, "Failed to write offset: Error " << errno << " (" << strerror(errno) << ")");
	}

	if (bytesWritten < 2) {
		return bytesWritten;
	}

	// Read data
	int bytesRead = read(mHandle, buf, count);
	if (bytesRead < 0) {
		LOG4CXX_ERROR(logger, "Failed to read " << count << " bytes: error " << errno << " (" << strerror(errno) << ")");
	//} else {
	//	LOG4CXX_DEBUG(logger, "Read " << bytesRead << " of " << count << " bytes at 0x" << hex << offset);
	}

	return bytesRead;
}

ssize_t Communicator::writeData(size_t offset, const void* buf, size_t count) {
	if (mHandle <= 0)
		return -3;

	// Prepend offset to data, then write
	char* temp = (char*)malloc(count + 2);
	if (temp == NULL) {
		LOG4CXX_ERROR(logger, "Could not allocate " << (count + 2) << " bytes of memory");
		return -4;
	}
	temp[0] = (offset >> 8) & 0xff;
	temp[1] = offset & 0xff;
	memcpy(&temp[2], buf, count);

	int bytesWritten = write(mHandle, &temp[0], count + 2);
	if (bytesWritten < 0) {
		LOG4CXX_ERROR(logger, "Failed to write " << count + 2 << " bytes: error " << errno << " (" << strerror(errno) << ")");
	//} else {
	//	LOG4CXX_DEBUG(logger, "Wrote " << bytesWritten << " of " << count + 2 << " bytes at 0x" << hex << offset);
	}
	free(temp);
	return bytesWritten;
}

