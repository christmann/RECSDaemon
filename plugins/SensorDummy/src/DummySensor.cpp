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

#include "DummySensor.h"

#include <cstring>
#include <cstdlib>
#include "daemon_msgs.h"

using namespace std;

LoggerPtr DummySensor::logger;

void * DummySensor::create(PF_ObjectParams *) {
	return new DummySensor();
}

int32_t DummySensor::destroy(void * p) {
	if (!p)
		return -1;
	delete static_cast<DummySensor*>(p);
	return 0;
}

DummySensor::DummySensor() : mLength(0), mData(NULL) {
}

DummySensor::~DummySensor() {
	free(mData);
}

bool DummySensor::configure(const char* data) {
	BaseSensor::configure(data);

	mLength = strlen(data) + 1; // +1 for 0 end marker
	mData = (char*)malloc(mLength);
	strcpy(mData, data);
	return true;
}

ISensorDataType DummySensor::getDataType(void) {
	return TYPE_STR;
}

size_t DummySensor::getMaxDataSize(void) {
	return mLength;
}

bool DummySensor::getData(uint8_t* data) {
	memcpy(data, mData, mLength);
	return true;
}

const char* DummySensor::getDescription(void) {
	return "Dummy sensor that returns configured string";
}

ISensorUnit DummySensor::getUnit(void) {
	return UNIT_DIMENSIONLESS;
}

LoggerPtr DummySensor::getLogger(void) {
	return logger;
}
