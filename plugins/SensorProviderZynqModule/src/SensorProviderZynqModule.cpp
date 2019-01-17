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

#include "SensorProviderZynqModule.h"

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "daemon_msgs.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#ifndef WIN32
#include <sys/signal.h>
#endif
#include <sys/types.h>

#define TIMEOUT_US	100000

using namespace std;

LoggerPtr SensorProviderZynqModule::logger;
IConfig* SensorProviderZynqModule::config;

void * SensorProviderZynqModule::create(PF_ObjectParams *) {
	return new SensorProviderZynqModule();
}

int32_t SensorProviderZynqModule::destroy(void * p) {
	if (!p)
		return -1;
	delete static_cast<SensorProviderZynqModule*>(p);
	return 0;
}

SensorProviderZynqModule::SensorProviderZynqModule() {
#ifndef WIN32
	mComFileDescriptor = -1;
	mOldComSettings.c_cflag = 0;
	mOldComSettings.c_iflag = 0;
	mOldComSettings.c_ispeed = 0;
	mOldComSettings.c_ospeed = 0;
#else
	mComFileDescriptor = INVALID_HANDLE_VALUE;
#endif
	string comPort = config->GetString("Plugins", "zynqSerialPort", "");
	if (comPort == "") {
		LOG_ERROR(logger, "No serial port configured (Plugins->zynqSerialPort)");
		return;
	}
	InitComPort(comPort);
}

SensorProviderZynqModule::~SensorProviderZynqModule() {
#ifndef WIN32
	if (mComFileDescriptor >= 0) {
		tcsetattr(mComFileDescriptor, TCSANOW, &mOldComSettings); // Restore old com port settings
		close(mComFileDescriptor);
	}
#else
    if (INVALID_HANDLE_VALUE != mComFileDescriptor) {
    	CloseHandle(mComFileDescriptor);
    }
#endif
}

bool SensorProviderZynqModule::InitComPort(string device) {
#ifndef WIN32
	// open the device(com port)
    mComFileDescriptor = open(device.c_str(), O_RDWR | O_NOCTTY);
    if (mComFileDescriptor < 0) {
        perror(device.c_str());
        return false;
    }

    struct termios newComSettings;
    if (tcgetattr(mComFileDescriptor, &mOldComSettings) != 0) { // save current port settings
    	LOG_ERROR(logger, "Error reading current port settings!");
    	close(mComFileDescriptor);
    	mComFileDescriptor = -1;
    	return false;
    }
    memcpy(&newComSettings, &mOldComSettings, sizeof(struct termios));
    // set new port settings for canonical input processing
    newComSettings.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    newComSettings.c_iflag = IGNPAR;
    newComSettings.c_oflag = 0;

    newComSettings.c_lflag = 0;       //ICANON;

    newComSettings.c_cc[VMIN] = 1;
    newComSettings.c_cc[VTIME] = 0;

    tcflush(mComFileDescriptor, TCIFLUSH);
    tcsetattr(mComFileDescriptor, TCSANOW, &newComSettings);
    return true;
#else
	// open the comm port.
	mComFileDescriptor = CreateFile(device.c_str(),
	GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if ( INVALID_HANDLE_VALUE == mComFileDescriptor) {
		LOG_ERROR(logger, "Could not open COM port " << device << ", error " << GetLastError());
		return false;
	}

	// get the current DCB, and adjust a few bits to our liking.
	DCB port;
	memset(&port, 0, sizeof(port));
	port.DCBlength = sizeof(port);
	if (!GetCommState(mComFileDescriptor, &port)) {
		LOG_ERROR(logger, "Could not get current port settings, error " << GetLastError());
		return false;
	}
	if (!BuildCommDCB("baud=115200 parity=n data=8 stop=1", &port)) {
		LOG_ERROR(logger, "Could not build new port settings");
		return false;
	}
	if (!SetCommState(mComFileDescriptor, &port)) {
		LOG_ERROR(logger, "Could not adjust port settings");
		return false;
	}

	COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = TIMEOUT_US / 1000;
    timeouts.ReadTotalTimeoutMultiplier = 1;
    timeouts.ReadTotalTimeoutConstant = TIMEOUT_US / 1000;
    timeouts.WriteTotalTimeoutMultiplier = 1;
    timeouts.WriteTotalTimeoutConstant = TIMEOUT_US / 1000;
    if (!SetCommTimeouts(mComFileDescriptor, &timeouts)) {
		LOG_ERROR(logger, "Could not set port timeouts, error " << GetLastError());
		return false;
    }

	return true;
#endif
}

void SensorProviderZynqModule::SendData(uint8_t* data, size_t size) {
#ifndef WIN32
	if (mComFileDescriptor < 0) {
		return;
	}

	ssize_t written = write(mComFileDescriptor, data, size);
	if (written <= 0) {
		LOG_ERROR(logger, "Error writing to serial port");
	}
#else
	if (INVALID_HANDLE_VALUE == mComFileDescriptor) {
		return;
	}

	DWORD written = 0;
	if (!WriteFile(mComFileDescriptor, data, size, &written, NULL)) {
		LOG_ERROR(logger, "Error writing to serial port, errorcode " << GetLastError());
		return;
	}

	if (written != size) {
		LOG_ERROR(logger, "Could not write all data to port");
	}
#endif
}

ssize_t SensorProviderZynqModule::ReadData(uint8_t* data, size_t maxSize) {
#ifndef WIN32
	fd_set setRead;
	int notused = 63;
	int cnt;

	FD_ZERO(&setRead);
	FD_SET(mComFileDescriptor, &setRead);

    struct timeval timeout;
    timeout.tv_usec = TIMEOUT_US;
    timeout.tv_sec  = 0;
    cnt = select(notused, &setRead, NULL, NULL, &timeout);
    if (cnt > 0) {
        if (FD_ISSET(mComFileDescriptor, &setRead)) {
        	int res = read(mComFileDescriptor, (void*)data, maxSize);
        	return res;
        } else {
        	return -1;
        }
    } else {
    	// Timeout
    	return -1;
    }
#else
    DWORD read = 0;
    ReadFile(mComFileDescriptor, data, maxSize, &read, NULL);
    return read;
#endif
}

size_t SensorProviderZynqModule::ReadUntilTerminator(uint8_t* data, size_t maxSize, uint8_t terminator, uint8_t ignoreFirstBytes) {
	size_t readTotal = 0;
	size_t remainingSize = maxSize;

	do {
		ssize_t read = ReadData(&data[readTotal], remainingSize);
		if (read > 0) {
			void* o = NULL;
			if (readTotal <= ignoreFirstBytes) {
				if (read > ignoreFirstBytes) {
					o = memchr(&data[readTotal + ignoreFirstBytes], terminator, read - ignoreFirstBytes);
				}
			} else {
				o = memchr(&data[readTotal], terminator, read);
			}
			if (o != NULL) {
				return readTotal + read;
			}
			readTotal += read;
			remainingSize -= read;
		} else {
			return read;
		}
	} while (1);

	return readTotal;
}

const char* SensorProviderZynqModule::getSensorsDescription(void) {
	uint8_t data[] = { 'd', '\r' };
	SendData(data, sizeof(data));

	ssize_t read = ReadUntilTerminator(mReceiveBuffer, MAX_RECEIVE_SIZE, '\n', 20);
	if (read > 0) {
		mReceiveBuffer[read] = '\0'; // Zero-terminate string
		char* start = strstr((char*)mReceiveBuffer, "d\r\n");
		if (start == NULL) {
			start = (char*)mReceiveBuffer;
			LOG_WARN(logger, "Could not find expected echo");
		} else {
			start += 3; // Skip echo
		}
		// Chop off after newline
		char* ptr;
		if( (ptr = strchr(start, '\r')) != NULL) {
		    *ptr = '\0';
		} else {
			LOG_WARN(logger, "Could not find terminator");
		}
		return start;
	}
	return "";
/*
	return "["
			"	{"
			"		\"name\": \"Voltage 1.0 V rail\","
			"		\"dataType\": \"double\","
			"		\"unit\": \"V\","
			"		\"lowerThresholds\": [0.5, 0.8],"
			"		\"upperThresholds\": [1.2, 1.5]"
			"	},"
			"	{"
			"		\"name\": \"Voltage 1.1 V rail\","
			"		\"dataType\": \"double\","
			"		\"unit\": \"V\","
			"		\"lowerThresholds\": [0.6, 0.9],"
			"		\"upperThresholds\": [1.3, 1.6]"
			"	},"
			"	{"
			"		\"name\": \"Voltage 1.5 V (DDR) rail\","
			"		\"dataType\": \"double\","
			"		\"unit\": \"V\","
			"		\"lowerThresholds\": [1.0, 1.3],"
			"		\"upperThresholds\": [1.7, 2.0]"
			"	}"
			"]";
*/
}

const char* SensorProviderZynqModule::getSensorsData(void) {
	uint8_t data[] = { 'm', '\r' };
	SendData(data, sizeof(data));

	ssize_t read = ReadUntilTerminator(mReceiveBuffer, MAX_RECEIVE_SIZE, '\n', 20);
	if (read > 0) {
		mReceiveBuffer[read] = '\0'; // Zero-terminate string
		char* start = strstr((char*)mReceiveBuffer, "m\r\n");
		if (start == NULL) {
			start = (char*)mReceiveBuffer;
			LOG_WARN(logger, "Could not find expected echo");
		} else {
			start += 3; // Skip echo
		}
		// Chop off after newline
		char* ptr;
		if( (ptr = strchr(start, '\r')) != NULL) {
		    *ptr = '\0';
		} else {
			LOG_WARN(logger, "Could not find terminator");
		}
		return start;
	}
	return "";

//	return "[ 1.0, 1.1, 1.5 ]";
}
