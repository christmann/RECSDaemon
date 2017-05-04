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

#ifndef DUMMYSENSOR_H
#define DUMMYSENSOR_H

#include <object_model.h>
#include <BaseSensor.h>
#include <string>
#include <log4cxx/logger.h>

using namespace std;

struct PF_ObjectParams;

class LinuxSensorIP: public BaseSensor {
public:

	// static plugin interface
	static void * create(PF_ObjectParams *);
	static int32_t destroy(void *);
	~LinuxSensorIP();

	// ISensor methods
	virtual bool configure(const char* data);
	virtual ISensorDataType getDataType(void);
	virtual size_t getMaxDataSize(void);
	virtual bool getData(uint8_t* data);
	virtual const char* getDescription(void);
	virtual ISensorUnit getUnit(void);

	virtual log4cxx::LoggerPtr getLogger(void);

	static log4cxx::LoggerPtr logger;

private:
	LinuxSensorIP();

	string mIPs;
};

#endif
