////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2021 christmann informationstechnik + medien GmbH & Co. KG
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

#include <semaeapi.h>

#define I2C_ADDRESS			(0x54 << 1)
#define I2C_ADDRESS_OLD1	(0x44 << 1)
#define I2C_ADDRESS_OLD2	(0x50 << 1)
#define I2C_SPEED		400000 // Hz

using namespace std;

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

Communicator::Communicator() : mHandle(0), mI2CAddress(I2C_ADDRESS), mMaxBlockLen(29) {
}

bool Communicator::initInterface() {

	uint32_t ret = SemaEApiLibInitialize(false, IP_V4, (char*)"127.0.0.1", 0, (char*)"123", &mHandle);
	if (ret != EAPI_STATUS_SUCCESS) {
		LOG_ERROR(logger, "Could not initialize SEMA library. Error " << ret);
		return false;
	}
	// Returned block length seems to be too large. Returned 64, but only values up to 29 worked.
	// Higher values seemed to be executed (no error returned), but did not show up on the bus
	/*ret = SemaEApiI2CGetBusCap(mHandle, EAPI_ID_I2C_EXTERNAL, &mMaxBlockLen);
	if (ret != EAPI_STATUS_SUCCESS) {
		LOG_ERROR(logger, "Could not get I2C capabilities. Error " << ret);
		return false;
	}*/
	LOG_DEBUG(logger, "Maximum block length: " << mMaxBlockLen);

	return true;
}

Communicator::~Communicator() {
	if (mHandle) {
		SemaEApiLibUnInitialize(mHandle);
	}
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
	if (!mHandle)
		return -3;

	uint32_t remaining = count;
	uint32_t pos = 0;
	while (remaining > 0) {
		uint32_t blockLen = min(remaining, mMaxBlockLen);

		// Write offset
		char temp[2];
		temp[0] = ((offset + pos) >> 8) & 0xff;
		temp[1] = (offset + pos) & 0xff;

		uint32_t ret = SemaEApiI2CWriteReadRaw(mHandle, EAPI_ID_I2C_EXTERNAL, mI2CAddress, temp, 2 + 1, (char*)buf + pos, blockLen, blockLen + 1);
		if (ret != EAPI_STATUS_SUCCESS) {
			LOG_WARN(logger, "I2C read failed, retrying (error 0x" << hex << ret << ")");
			ret = SemaEApiI2CWriteReadRaw(mHandle, EAPI_ID_I2C_EXTERNAL, mI2CAddress, temp, 2 + 1, (char*)buf + pos, blockLen, blockLen + 1);
			if (ret != EAPI_STATUS_SUCCESS) {
				LOG_ERROR(logger, "I2C read failed (error 0x" << hex << ret << ")");
				return -1;
			}
		}
		remaining -= blockLen;
		pos += blockLen;
	}
	return count;
}

ssize_t Communicator::writeData(size_t offset, const void* buf, size_t count) {
	if (!mHandle)
		return -3;

	char* temp = (char*)malloc(mMaxBlockLen);
	if (temp == NULL) {
		LOG_ERROR(logger, "Could not allocate " << (count + 2) << " bytes of memory");
		return -4;
	}

	uint32_t remaining = count;
	uint32_t pos = 0;
	while (remaining > 0) {
		uint32_t blockLen = min(remaining, mMaxBlockLen - 2);

		// Prepend offset to data, then write
		temp[0] = ((offset + pos) >> 8) & 0xff;
		temp[1] = (offset + pos) & 0xff;
		memcpy(&temp[2], (char*)buf + pos, blockLen);

		uint32_t ret = SemaEApiI2CWriteRaw(mHandle, EAPI_ID_I2C_EXTERNAL, mI2CAddress, temp, blockLen + 2 + 1);
		if (ret != EAPI_STATUS_SUCCESS) {
			LOG_WARN(logger, "I2C write failed, retrying (error 0x" << hex << ret << ")");
			ret = SemaEApiI2CWriteRaw(mHandle, EAPI_ID_I2C_EXTERNAL, mI2CAddress, temp, blockLen + 2 + 1);
			if (ret != EAPI_STATUS_SUCCESS) {
				LOG_ERROR(logger, "I2C write failed (error 0x" << hex << ret << ")");
				free(temp);
				return -1;
			}
		}
		remaining -= blockLen;
		pos += blockLen;
	}
	free(temp);
	return count;
}
