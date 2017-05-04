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
#include <unistd.h>
#include <stdint.h>
#include <climits>
#include "daemon_msgs.h"

#define I2C_ADDRESS			(0x54 << 1)
#define I2C_ADDRESS_OLD1	(0x44 << 1)
#define I2C_ADDRESS_OLD2	(0x50 << 1)
#define I2C_SPEED		400000 // Hz

using namespace std;
using namespace log4cxx;

LoggerPtr Communicator::logger;

void * Communicator::create(PF_ObjectParams *) {
	return new Communicator();
}

int32_t Communicator::destroy(void * p) {
	if (!p)
		return -1;
	delete static_cast<Communicator*>(p);
	return 0;
}

Communicator::Communicator() : mHandle(0), mBus(ULONG_MAX), mI2CAddress(I2C_ADDRESS) {
}

bool Communicator::initInterface() {
	// install the library
	if (!CgosLibInitialize()) {
		if (!CgosLibInstall(1)) {
			// error: can't install cgos library
			LOG4CXX_ERROR(logger, "Error: can't install CGOS library");
			return false;
		}
		LOG4CXX_INFO(logger, "The driver has been installed.");
		if (!CgosLibInitialize()) {
			LOG4CXX_ERROR(logger, "Still could not open driver, a reboot might be required!");
			return false;
		}
	}

	//check library version
	unsigned int dwLibVersion = CgosLibGetVersion();
	unsigned int dwDrvVersion = CgosLibGetDrvVersion();

	if (dwLibVersion < 0x0102000A || dwDrvVersion < 0x0102000A) {
		LOG4CXX_ERROR(logger, "Outdated CGOS Library/Driver version. Detected library version: 0x" << hex << dwLibVersion << ". Required library version: 0x0102000A. Detected driver version: 0x" << hex << dwDrvVersion << ". Required driver version: 0x0102000A");
		return false;
	}

	// open the cgos board
	if (CgosBoardOpen(0, 0, 0, &mHandle)) {
		unsigned long cnt = CgosI2CCount(mHandle); // determines the amount of available I2C interfaces
		LOG4CXX_DEBUG(logger, cnt << " I2C busses available");
		// navigate to the correct I2C bus
		for (unsigned long dwUnit = 0; dwUnit < cnt; ++dwUnit) {
			unsigned long dwType = CgosI2CType(mHandle, dwUnit);
			LOG4CXX_DEBUG(logger, "I2C bus " << dwUnit << " is of type 0x" << hex << dwType);
			if (dwType == CGOS_I2C_TYPE_PRIMARY) {
				mBus = dwUnit;
				LOG4CXX_DEBUG(logger, "Found primary I2C bus as bus " << mBus);
				break;
			}
		}
		if (mBus == ULONG_MAX) {
			LOG4CXX_ERROR(logger, "Could not find primary I2C bus");
			if (mHandle) {
				CgosBoardClose(mHandle);
				mHandle = 0;
			}
			return false;
		}

#ifdef WIN32
		unsigned long maxSpeed;
#else
		uint32_t maxSpeed;
#endif
		CgosI2CGetMaxFrequency(mHandle, mBus, &maxSpeed);
		LOG4CXX_DEBUG(logger, "Maximum bus speed is " << maxSpeed << " Hz");
		if (maxSpeed >= I2C_SPEED) {
			maxSpeed = I2C_SPEED;
			CgosI2CSetFrequency(mHandle, mBus, maxSpeed);
			LOG4CXX_INFO(logger, "Set bus speed to " << maxSpeed << " Hz");
		}
	} else {
		// error: can't open board
		LOG4CXX_ERROR(logger, "Can't open CGOS board");
		return false;
	}
	return true;
}

Communicator::~Communicator() {
	if (mHandle) {
		CgosBoardClose(mHandle);
	}
	CgosLibUninitialize();
}

size_t Communicator::getMaxDataSize(void) {
	Daemon_Header hdr;

	hdr.size = 0;
	readData(0, &hdr, sizeof(Daemon_Header));

	if (hdr.size == 0) { // No valid header found, maybe on old address?
		mI2CAddress = I2C_ADDRESS_OLD1;
		readData(0, &hdr, sizeof(Daemon_Header));
		if (hdr.size == 0) { // Still not found, try even older one
			mI2CAddress = I2C_ADDRESS_OLD2;
			readData(0, &hdr, sizeof(Daemon_Header));
			if (hdr.size == 0) { // Still not found, change back address
				mI2CAddress = I2C_ADDRESS;
			}
		}
	}

	return hdr.size;
}

ssize_t Communicator::readData(size_t offset, void* buf, size_t count) {
	if (!mHandle || mBus == ULONG_MAX)
		return -3;

	// Write offset
	char temp[2];
	temp[0] = (offset >> 8) & 0xff;
	temp[1] = offset & 0xff;
	if (!CgosI2CWrite(mHandle, mBus, mI2CAddress, (unsigned char*)&temp[0], 2)) {
		return -2;
	}

	// Read data
	if (CgosI2CRead(mHandle, mBus, mI2CAddress | 0x01, (unsigned char*)buf, count)) {
		return count;
	}
	return -1;
}

ssize_t Communicator::writeData(size_t offset, const void* buf, size_t count) {
	if (!mHandle || mBus == ULONG_MAX)
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

	if (!CgosI2CWrite(mHandle, mBus, mI2CAddress, (unsigned char*)&temp[0], count + 2)) {
		free(temp);
		return -1;
	}
	free(temp);
	return count;
}

