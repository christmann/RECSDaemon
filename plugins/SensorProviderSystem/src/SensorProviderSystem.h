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

#ifndef SENSORPROVIDERZYNQ_H
#define SENSORPROVIDERZYNQ_H

#ifdef WIN32
#define WINVER 0x0501 // At least Win XP
#include <Windows.h>
#endif
#include <object_model.h>
#include <string>
#include <map>
#include <log4cxx/logger.h>
#include <c_object_model.h>
#include <SensorBean.h>

struct PF_ObjectParams;

class SensorProviderSystem: public ISensorProvider {
public:
	// static plugin interface
	static void * create(PF_ObjectParams *);
	static int32_t destroy(void *);
	~SensorProviderSystem();

	// ISensorProvider methods
	virtual std::map<std::string, ISensor*> getSensors(void);

	static log4cxx::LoggerPtr logger;
private:
	SensorProviderSystem();
	static void updateCpuUtilization(SensorBean* sensor);
	static void updateMemoryFree(SensorBean* sensor);
	static void updateDiskFree(SensorBean* sensor);
#ifdef WIN32
	static uint64_t filetimeToUint64(const FILETIME &v);
#endif


	std::map<std::string, ISensor*> mSensors;
	static uint64_t mLastTotalTime;
	static uint64_t mLastWorkTime;
};

#endif
