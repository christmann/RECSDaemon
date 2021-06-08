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
// Author: Micha vor dem Berge <micha.vordemberge@christmann.info>
////////////////////////////////////////////////////////////////////////////////

#ifndef SENSORPROVIDERJETSON_H
#define SENSORPROVIDERJETSON_H

#include <object_model.h>
#include <string>
#include <map>
#include <logger.h>
#include <c_object_model.h>
#include <SensorBean.h>

struct PF_ObjectParams;

class SensorProviderJetson: public ISensorProvider {
public:
	// static plugin interface
	static void * create(PF_ObjectParams *);
	static int32_t destroy(void *);
	~SensorProviderJetson();

	// ISensorProvider methods
	virtual std::map<std::string, ISensor*> getSensors(void);

	static LoggerPtr logger;
private:
	SensorProviderJetson();
	static void updatePowerCpu(SensorBean* sensor);
	static void updatePowerGpu(SensorBean* sensor);
	static void updatePowerSoC(SensorBean* sensor);
	static void updatePowerCv(SensorBean* sensor);
	static void updatePowerDDR(SensorBean* sensor);
	static void updatePowerSys5V(SensorBean* sensor);
	static void updateTempSoC(SensorBean* sensor);

	std::map<std::string, ISensor*> mSensors;
};

#endif
