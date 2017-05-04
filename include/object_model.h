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

#ifndef OBJECT_MODEL
#define OBJECT_MODEL

#include "c_object_model.h"
#include <map>
#include <string>

struct ICommunicator {
	virtual ~ICommunicator() {}

	virtual bool initInterface(void) = 0;

	virtual size_t getMaxDataSize(void) = 0;

	virtual ssize_t readData(size_t offset, void* buf, size_t count) = 0;
	virtual ssize_t writeData(size_t offset, const void* buf, size_t count) = 0;
};

struct ISlotDetector {
	virtual ~ISlotDetector() {}

	virtual int8_t getSlot(void) = 0;
};

struct ISensor {
	virtual ~ISensor() {}

	virtual bool configure(const char* options) = 0;
	virtual ISensorDataType getDataType(void) = 0;
	virtual size_t getMaxDataSize(void) = 0;
	virtual bool getData(uint8_t* data) = 0;
	virtual const char* getDescription(void) = 0;

	virtual ISensorUnit getUnit(void) = 0;
	virtual bool getUseLowerThresholds(void) = 0;
	virtual bool getUseUpperThresholds(void) = 0;
	virtual double getLowerCriticalThreshold(void) = 0;
	virtual double getLowerWarningThreshold(void) = 0;
	virtual double getUpperWarningThreshold(void) = 0;
	virtual double getUpperCriticalThreshold(void) = 0;

	virtual uint16_t getNumberOfValues(void) = 0;
	virtual IRenderingType getRenderingType(void) = 0;
	virtual const char* getGroup(void) = 0;
};

struct IJSONSensorProvider {
	virtual ~IJSONSensorProvider() {}

	virtual const char* getSensorsDescription(void) = 0;
	virtual const char* getSensorsData(void) = 0;
};

struct ISensorProvider {
	virtual ~ISensorProvider() {}

	virtual std::map<std::string, ISensor*> getSensors(void) = 0;
};

#endif
