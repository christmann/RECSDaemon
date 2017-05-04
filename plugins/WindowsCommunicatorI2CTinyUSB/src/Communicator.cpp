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
#include "daemon_msgs.h"

#define I2C_ADDRESS			0x54
#define I2C_ADDRESS_OLD1	0x44
#define I2C_ADDRESS_OLD2	0x50

#define USB_CTRL_IN    (USB_TYPE_CLASS | USB_ENDPOINT_IN)
#define USB_CTRL_OUT   (USB_TYPE_CLASS)

#define I2C_TINY_USB_VID  0x0403
#define I2C_TINY_USB_PID  0xc631

#define I2C_M_RD		0x01

#define CMD_ECHO       0
#define CMD_GET_FUNC   1
#define CMD_SET_DELAY  2
#define CMD_GET_STATUS 3
#define CMD_I2C_IO     4
#define CMD_I2C_BEGIN  1  // flag to I2C_IO
#define CMD_I2C_END    2  // flag to I2C_IO
#define STATUS_IDLE          0
#define STATUS_ADDRESS_ACK   1
#define STATUS_ADDRESS_NAK   2

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

Communicator::Communicator() : mHandle(NULL), mI2CAddress(I2C_ADDRESS) {
}

bool Communicator::initInterface() {
	usb_init();

	usb_find_busses();
	usb_find_devices();

	for (struct usb_bus* bus = usb_get_busses(); bus; bus = bus->next) {
		for (struct usb_device* dev = bus->devices; dev; dev = dev->next) {
			if ((dev->descriptor.idVendor == I2C_TINY_USB_VID) && (dev->descriptor.idProduct == I2C_TINY_USB_PID)) {
				LOG4CXX_INFO(logger, "Found i2c_tiny_usb device on bus " << bus->dirname << " device " << dev->filename);

				// open device
				if (!(mHandle = usb_open(dev))) {
					LOG4CXX_ERROR(logger, "Cannot open the device: " << usb_strerror());
				}

				break;
			}
		}
	}

	if (!mHandle) {
		LOG4CXX_ERROR(logger, "Could not find i2c_tiny_usb device");
		return false;
	}
	return true;
}

Communicator::~Communicator() {
	if (mHandle) {
		usb_close(mHandle);
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

	// Write offset
	char temp[2];
	temp[0] = (offset >> 8) & 0xff;
	temp[1] = offset & 0xff;
	if (usb_control_msg(mHandle, USB_CTRL_OUT, CMD_I2C_IO + CMD_I2C_BEGIN + CMD_I2C_END, 0, mI2CAddress, &temp[0], 2, 1000) < 1) {
		LOG4CXX_ERROR(logger, "USB error: " << usb_strerror());
		return -2;
	}

	// Read data
	if (usb_control_msg(mHandle, USB_CTRL_IN, CMD_I2C_IO + CMD_I2C_BEGIN + CMD_I2C_END, I2C_M_RD, mI2CAddress, (char*) buf, count, 1000) < 1) {
		LOG4CXX_ERROR(logger, "USB error: " << usb_strerror())
		return -2;
	}

	int status = i2c_tiny_usb_get_status();
	if (status != STATUS_ADDRESS_ACK) {
//		fprintf(stderr, "read data status failed (status = %d)\n", status);
		return -1;
	}

	return count;
}

ssize_t Communicator::writeData(size_t offset, const void* buf, size_t count) {
	if (!mHandle)
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
	if (usb_control_msg(mHandle, USB_CTRL_OUT, CMD_I2C_IO + CMD_I2C_BEGIN + CMD_I2C_END, 0, mI2CAddress, &temp[0], count + 2, 1000) < 1) {
		LOG4CXX_ERROR(logger, "USB error: " << usb_strerror());
		free(temp);
		return -2;
	}
	free(temp);

	int status = i2c_tiny_usb_get_status();
	if (status != STATUS_ADDRESS_ACK) {
//		fprintf(stderr, "write command status failed (status = %d)\n", status);
		return 0;
	}

	return count;
}

// read a set of bytes from the i2c_tiny_usb device
int Communicator::i2c_tiny_usb_read(unsigned char cmd, void *data, int len) {
	int nBytes;

	if (!mHandle)
		return -1;

	// send control request and accept return value
	nBytes = usb_control_msg(mHandle, USB_CTRL_IN, cmd, 0, 0, (char*)data, len, 1000);

	if (nBytes < 0) {
		LOG4CXX_ERROR(logger, "USB error: " << usb_strerror());
		return nBytes;
	}

	return 0;
}

// get the current transaction status from the i2c_tiny_usb interface
int Communicator::i2c_tiny_usb_get_status(void) {
	int i;
	unsigned char status;

	if (!mHandle)
		return -1;

	if ((i = i2c_tiny_usb_read(CMD_GET_STATUS, &status, sizeof(status))) < 0) {
		LOG4CXX_ERROR(logger, "Error reading status");
		return i;
	}

	return status;
}
