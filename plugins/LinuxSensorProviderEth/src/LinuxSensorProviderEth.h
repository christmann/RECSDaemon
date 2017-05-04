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

#include <object_model.h>
#include <string>
#include <map>
#include <log4cxx/logger.h>
#include <c_object_model.h>
#include <SensorBean.h>

struct PF_ObjectParams;

class LinuxSensorProviderEth: public ISensorProvider {
public:
	class AdapterInfo {
		public:
		string name;
		struct timespec lastUpdate;
		uint64_t lastCountRx;
		uint64_t lastCountTx;
		SensorBean* txSensor;
	};

	// static plugin interface
	static void * create(PF_ObjectParams *);
	static int32_t destroy(void *);
	~LinuxSensorProviderEth();

	// ISensorProvider methods
	virtual std::map<std::string, ISensor*> getSensors(void);

	static log4cxx::LoggerPtr logger;
private:
	LinuxSensorProviderEth();
	static void updateLinkStatus(SensorBean* sensor);
	static void updateUtilization(SensorBean* sensor);
	static void destroySensor(SensorBean* sensor);

	std::map<std::string, ISensor*> mSensors;
};

#endif
