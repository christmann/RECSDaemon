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

#ifndef SENSORADAPTER_H
#define SENSORADAPTER_H

//----------------------------------------------------------------------

#include "include/object_model.h"

//----------------------------------------------------------------------

class SensorAdapter: public ISensor {
public:
	SensorAdapter(C_Sensor * Sensor, PF_DestroyFunc destroyFunc) :
			Sensor_(Sensor), destroyFunc_(destroyFunc) {
	}

	~SensorAdapter() {
		if (destroyFunc_)
			destroyFunc_(Sensor_);
	}

	// ISensor implementation
	bool configure(const char* options) {
		return Sensor_->configure(Sensor_->handle, options);
	}

	ISensorDataType getDataType(void) {
		return Sensor_->getDataType(Sensor_->handle);
	}

	size_t getMaxDataSize(void) {
		return Sensor_->getMaxDataSize(Sensor_->handle);
	}

	bool getData(uint8_t* data) {
		return Sensor_->getData(Sensor_->handle, data);
	}

	const char* getDescription(void) {
		return Sensor_->getDescription(Sensor_->handle);
	}

	ISensorUnit getUnit(void) {
		return Sensor_->getUnit(Sensor_->handle);
	}

	bool getUseLowerThresholds(void) {
		return Sensor_->getUseLowerThresholds(Sensor_->handle);
	}

	bool getUseUpperThresholds(void) {
		return Sensor_->getUseUpperThresholds(Sensor_->handle);
	}

	double getLowerCriticalThreshold(void) {
		return Sensor_->getLowerCriticalThreshold(Sensor_->handle);
	}

	double getLowerWarningThreshold(void) {
		return Sensor_->getLowerWarningThreshold(Sensor_->handle);
	}

	double getUpperWarningThreshold(void) {
		return Sensor_->getUpperWarningThreshold(Sensor_->handle);
	}

	double getUpperCriticalThreshold(void) {
		return Sensor_->getUpperCriticalThreshold(Sensor_->handle);
	}

	uint16_t getNumberOfValues(void) {
		return Sensor_->getNumberOfValues(Sensor_->handle);
	}

	IRenderingType getRenderingType(void) {
		return Sensor_->getRenderingType(Sensor_->handle);
	}

	const char* getGroup(void) {
		return Sensor_->getGroup(Sensor_->handle);
	}

private:
	C_Sensor * Sensor_;
	PF_DestroyFunc destroyFunc_;
};

#endif // SENSORADAPTER_H
