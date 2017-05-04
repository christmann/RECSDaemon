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

#include "SlotDetector.h"

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;
using namespace log4cxx;

LoggerPtr SlotDetector::logger;
IConfig* SlotDetector::config;

void * SlotDetector::create(PF_ObjectParams *) {
	return new SlotDetector();
}

int32_t SlotDetector::destroy(void * p) {
	if (!p)
		return -1;
	delete static_cast<SlotDetector*>(p);
	return 0;
}

SlotDetector::SlotDetector() {
}

SlotDetector::~SlotDetector() {
}

int SlotDetector::exportGPIO(int pinNr) {
	FILE* fp;
	char set_value[5];

	if ((fp = fopen("/sys/class/gpio/export", "ab")) == NULL) {
		LOG4CXX_ERROR(logger, "Cannot open GPIO export file: " << string(strerror(errno)));
		return -1;
	}
	//Set pointer to beginning of the file
	rewind(fp);
	//Write our value to the file
	sprintf(&set_value[0], "%d", pinNr);
	fwrite(&set_value[0], sizeof(char), strlen(set_value), fp);
	fclose(fp);

	return 0;
}

int SlotDetector::setDirection(int pinNr, int direction) {
	FILE* fp;
	char filename[35];
	char set_value[5];

	sprintf(&filename[0], "/sys/class/gpio/gpio%d/direction", pinNr);
	if ((fp = fopen(filename, "rb+")) == NULL) {
		LOG4CXX_ERROR(logger, "cannot open GPIO " << pinNr << " direction file: " << string(strerror(errno)));
		return -1;
	}
	//Set pointer to begining of the file
	rewind(fp);
	//Write our value to the file
	if (direction == 1) {
		strcpy(&set_value[0], "out");
	} else {
		strcpy(&set_value[0], "in");
	}
	fwrite(&set_value[0], sizeof(char), strlen(set_value), fp);
	fclose(fp);

	return 0;
}

FILE* SlotDetector::openGPIO(int pinNr) {
	FILE* fp;
	char filename[35];

	sprintf(&filename[0], "/sys/class/gpio/gpio%d/value", pinNr);
	if ((fp = fopen(filename, "rb+")) == NULL) {
		LOG4CXX_ERROR(logger, "cannot open GPIO " << pinNr << " value file: " << string(strerror(errno)));
		return NULL;
	}
	return fp;
}

int8_t SlotDetector::getGPIO(FILE* fp) {
	if (fp != NULL) {
		char buffer[1];
		rewind(fp);
		fread(&buffer[0], sizeof(char), sizeof(buffer), fp);
		if (buffer[0] == '1') {
			return 1;
		} else if (buffer[0] == '0') {
			return 0;
		}
	}
	return -1;
}

int8_t SlotDetector::getSlot(void) {
	int bit0GPIO = config->GetInt("Slot", "Bit0GPIO", -1);
	int bit1GPIO = config->GetInt("Slot", "Bit1GPIO", -1);
	if (bit0GPIO == -1 || bit1GPIO == -1) {
		LOG4CXX_ERROR(logger, "GPIO config not set. Please set entries Bit0GPIO and Bit1GPIO in section Slot in config file");
		return -1;
	}

	// Make sure they are exported
	int ret;
	ret = exportGPIO(bit0GPIO);
	if (ret < 0) return ret;
	ret = exportGPIO(bit1GPIO);
	if (ret < 0) return ret;

	// Set as inputs
	ret = setDirection(bit0GPIO, 0);
	if (ret < 0) return ret;
	ret = setDirection(bit1GPIO, 0);
	if (ret < 0) return ret;

	FILE* bit0 = openGPIO(bit0GPIO);
	FILE* bit1 = openGPIO(bit1GPIO);
	if (bit0 == NULL || bit1 == NULL) {
		fclose(bit0);
		fclose(bit1);
		return -1;
	}

	int8_t bit0Value = getGPIO(bit0);
	int8_t bit1Value = getGPIO(bit1);
	if (bit0Value < 0 || bit1Value < 0) return -1;

	fclose(bit0);
	fclose(bit1);

	return (bit1Value << 1) | bit0Value;
}
