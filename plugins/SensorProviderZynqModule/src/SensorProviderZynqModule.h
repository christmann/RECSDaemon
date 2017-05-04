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

#ifndef SENSORPROVIDERZYNQMODULE_H
#define SENSORPROVIDERZYNQMODULE_H

#include <object_model.h>
#include <string>
#include <log4cxx/logger.h>
#include <c_object_model.h>
#include <IConfig.h>

#ifdef WIN32
#include <windows.h>
#else
#include <termios.h>
#endif

#define MAX_RECEIVE_SIZE	2048

struct PF_ObjectParams;

class SensorProviderZynqModule: public IJSONSensorProvider {
public:

	// static plugin interface
	static void * create(PF_ObjectParams *);
	static int32_t destroy(void *);
	~SensorProviderZynqModule();

	// ISensorProvider methods
	virtual const char* getSensorsDescription(void);
	virtual const char* getSensorsData(void);

	static log4cxx::LoggerPtr logger;
	static IConfig* config;

private:
	SensorProviderZynqModule();

	bool InitComPort(std::string device);
	void SendData(uint8_t* data, size_t size);
	ssize_t ReadData(uint8_t* data, size_t maxSize);
	size_t ReadUntilTerminator(uint8_t* data, size_t maxSize, uint8_t terminator, uint8_t ignoreFirstBytes);

#ifdef WIN32
	HANDLE mComFileDescriptor;
#else
	int mComFileDescriptor;
	struct termios mOldComSettings;
#endif
	uint8_t mReceiveBuffer[MAX_RECEIVE_SIZE];
};

#endif
