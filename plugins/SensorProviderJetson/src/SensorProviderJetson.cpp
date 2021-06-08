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

#include "SensorProviderJetson.h"

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <object_model.h>
#include <SensorBean.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <sys/statvfs.h>

using namespace std;

LoggerPtr SensorProviderJetson::logger;

void * SensorProviderJetson::create(PF_ObjectParams *) {
	return new SensorProviderJetson();
}

int32_t SensorProviderJetson::destroy(void * p) {
	if (!p)
		return -1;
	delete static_cast<SensorProviderJetson*>(p);
	return 0;
}

SensorProviderJetson::SensorProviderJetson() :
	mSensors() {

	ifstream myfile("/sys/bus/i2c/drivers/ina3221x/1-0040/iio_device/in_power0_input");
	if (myfile.is_open()) {
		myfile.close();

		// Power Measurements
		SensorBean* sensor = new SensorBean("Power CPU", TYPE_FLOAT, 8, 1, UNIT_POWER, false, false, 0, 0, 0, 0, "", RENDERING_TEXTUAL);
		sensor->setData((uint32_t)0);
		sensor->setUpdateCallback(&SensorProviderJetson::updatePowerCpu);
		mSensors["Power CPU"] = sensor;

		sensor = new SensorBean("Power GPU", TYPE_FLOAT, 8, 1, UNIT_POWER, false, false, 0, 0, 0, 0, "", RENDERING_TEXTUAL);
		sensor->setData((uint32_t)0);
		sensor->setUpdateCallback(&SensorProviderJetson::updatePowerGpu);
		mSensors["Power GPU"] = sensor;

		sensor = new SensorBean("Power SoC", TYPE_FLOAT, 8, 1, UNIT_POWER, false, false, 0, 0, 0, 0, "", RENDERING_TEXTUAL);
		sensor->setData((uint32_t)0);
		sensor->setUpdateCallback(&SensorProviderJetson::updatePowerSoC);
		mSensors["Power SoC"] = sensor;

		sensor = new SensorBean("Power CV", TYPE_FLOAT, 8, 1, UNIT_POWER, false, false, 0, 0, 0, 0, "", RENDERING_TEXTUAL);
		sensor->setData((uint32_t)0);
		sensor->setUpdateCallback(&SensorProviderJetson::updatePowerCv);
		mSensors["Power CV"] = sensor;

		sensor = new SensorBean("Power DDR", TYPE_FLOAT, 8, 1, UNIT_POWER, false, false, 0, 0, 0, 0, "", RENDERING_TEXTUAL);
		sensor->setData((uint32_t)0);
		sensor->setUpdateCallback(&SensorProviderJetson::updatePowerSoC);
		mSensors["Power DDR"] = sensor;

		sensor = new SensorBean("Power SYS5V", TYPE_FLOAT, 8, 1, UNIT_POWER, false, false, 0, 0, 0, 0, "", RENDERING_TEXTUAL);
		sensor->setData((uint32_t)0);
		sensor->setUpdateCallback(&SensorProviderJetson::updatePowerSys5V);
		mSensors["Power SYS5V"] = sensor;

		// Temperature Measurements
		sensor = new SensorBean("Temp SoC", TYPE_FLOAT, 8, 1, UNIT_TEMPERATURE, false, false, 0, 0, 0, 0, "", RENDERING_TEXTUAL);
		sensor->setData((uint32_t)0);
		sensor->setUpdateCallback(&SensorProviderJetson::updateTempSoC);
		mSensors["Temp SoC"] = sensor;
	} else {
		LOG_ERROR(logger, "This plugin only works on Nvidia Jetson Xavier AGX, not initializing.");
	}
}

SensorProviderJetson::~SensorProviderJetson() {
}

map<string, ISensor*> SensorProviderJetson::getSensors(void) {
	return mSensors;
}

void SensorProviderJetson::updatePowerCpu(SensorBean* sensor) {
	string line;
	uint16_t milli_power;

	ifstream myfile("/sys/bus/i2c/drivers/ina3221x/1-0040/iio_device/in_power1_input");
	if (myfile.is_open()) {
		getline(myfile, line);
		std::istringstream stream(line);
		stream >> milli_power;
		myfile.close();
	} else {
		LOG_ERROR(logger, "Could not read file '/sys/bus/i2c/drivers/ina3221x/1-0040/iio_device/in_power1_input'");
		return;
	}

	//LOG_DEBUG(logger, "Power CPU: " << milli_power / 1000.0 << " W");
	sensor->setData(milli_power / 1000.0);
}

void SensorProviderJetson::updatePowerGpu(SensorBean* sensor) {
	string line;
	uint16_t milli_power;

	ifstream myfile("/sys/bus/i2c/drivers/ina3221x/1-0040/iio_device/in_power0_input");
	if (myfile.is_open()) {
		getline(myfile, line);
		std::istringstream stream(line);
		stream >> milli_power;
		myfile.close();
	} else {
		LOG_ERROR(logger, "Could not read file '/sys/bus/i2c/drivers/ina3221x/1-0040/iio_device/in_power0_input'");
		return;
	}

	//LOG_DEBUG(logger, "Power GPU: " << milli_power / 1000.0 << " W");
	sensor->setData(milli_power / 1000.0);
}

void SensorProviderJetson::updatePowerSoC(SensorBean* sensor) {
	string line;
	uint16_t milli_power;

	ifstream myfile("/sys/bus/i2c/drivers/ina3221x/1-0040/iio_device/in_power2_input");
	if (myfile.is_open()) {
		getline(myfile, line);
		std::istringstream stream(line);
		stream >> milli_power;
		myfile.close();
	} else {
		LOG_ERROR(logger, "Could not read file '/sys/bus/i2c/drivers/ina3221x/1-0040/iio_device/in_power2_input'");
		return;
	}

	//LOG_DEBUG(logger, "Power SoC: " << milli_power / 1000.0 << " W");
	sensor->setData(milli_power / 1000.0);
}

void SensorProviderJetson::updatePowerCv(SensorBean* sensor) {
	string line;
	uint16_t milli_power;

	ifstream myfile("/sys/bus/i2c/drivers/ina3221x/1-0041/iio_device/in_power0_input");
	if (myfile.is_open()) {
		getline(myfile, line);
		std::istringstream stream(line);
		stream >> milli_power;
		myfile.close();
	} else {
		LOG_ERROR(logger, "Could not read file '/sys/bus/i2c/drivers/ina3221x/1-0041/iio_device/in_power0_input'");
		return;
	}

	//LOG_DEBUG(logger, "Power CV: " << milli_power / 1000.0 << " W");
	sensor->setData(milli_power / 1000.0);
}
void SensorProviderJetson::updatePowerDDR(SensorBean* sensor) {
	string line;
	uint16_t milli_power;

	ifstream myfile("/sys/bus/i2c/drivers/ina3221x/1-0041/iio_device/in_power1_input");
	if (myfile.is_open()) {
		getline(myfile, line);
		std::istringstream stream(line);
		stream >> milli_power;
		myfile.close();
	} else {
		LOG_ERROR(logger, "Could not read file '/sys/bus/i2c/drivers/ina3221x/1-0041/iio_device/in_power1_input'");
		return;
	}

	//LOG_DEBUG(logger, "Power DDR: " << milli_power / 1000.0 << " W");
	sensor->setData(milli_power / 1000.0);
}

void SensorProviderJetson::updatePowerSys5V(SensorBean* sensor) {
	string line;
	uint16_t milli_power;

	ifstream myfile("/sys/bus/i2c/drivers/ina3221x/1-0041/iio_device/in_power2_input");
	if (myfile.is_open()) {
		getline(myfile, line);
		std::istringstream stream(line);
		stream >> milli_power;
		myfile.close();
	} else {
		LOG_ERROR(logger, "Could not read file '/sys/bus/i2c/drivers/ina3221x/1-0041/iio_device/in_power2_input'");
		return;
	}

	//LOG_DEBUG(logger, "Power SYS5V: " << milli_power / 1000.0 << " W");
	sensor->setData(milli_power / 1000.0);
}

void SensorProviderJetson::updateTempSoC(SensorBean* sensor) {
	string line;
	uint16_t milli_temp;

	ifstream myfile("/sys/devices/virtual/thermal/thermal_zone4/temp");
	if (myfile.is_open()) {
		getline(myfile, line);
		std::istringstream stream(line);
		stream >> milli_temp;
		myfile.close();
	} else {
		LOG_ERROR(logger, "Could not read file '/sys/devices/virtual/thermal/thermal_zone4/temp'");
		return;
	}

	//LOG_DEBUG(logger, "Temp SoC: " << milli_power / 1000.0 << " C");
	sensor->setData(milli_temp / 1000.0);
}
