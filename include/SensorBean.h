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
// Created on: 26.10.2015
////////////////////////////////////////////////////////////////////////////////

#ifndef SENSORBEAN_H_
#define SENSORBEAN_H_

#include <object_model.h>
#include <BaseSensor.h>
#include <string>
#include <cstdlib>
#include <cstring>
#include <log4cxx/logger.h>
#include <c_object_model.h>
// For htonl
#ifdef WIN32
  #include <winsock2.h>
#else
  #include <arpa/inet.h>
#endif

class SensorBean: public BaseSensor {
public:
	typedef void (*updateSensorCallback_t)(SensorBean* sensor);
	typedef void (*destroySensorCallback_t)(SensorBean* sensor);

	SensorBean(std::string name, ISensorDataType dataType,
			size_t maxDataSize, uint16_t numberOfValues, ISensorUnit unit,
			bool useLowerThresholds, bool useUpperThresholds,
			double lowerCriticalThreshold, double lowerWarningThreshold,
			double upperWarningThreshold, double upperCriticalThreshold,
			std::string group, IRenderingType renderingType) :
				mTag(NULL), mName(name), mDataType(dataType), mUnit(unit), mMaxDataSize(maxDataSize),
				mUpdateCallback(NULL), mDestroyCallback(NULL)
	{
		mUseLowerThresholds = useLowerThresholds;
		mUseUpperThresholds = useUpperThresholds;
		mLowerCriticalThreshold = lowerCriticalThreshold;
		mLowerWarningThreshold = lowerWarningThreshold;
		mUpperWarningThreshold = upperWarningThreshold;
		mUpperCriticalThreshold = upperCriticalThreshold;
		mNumberOfValues = numberOfValues;
		mGroup = group;
		mRendering = renderingType;
		mData = (uint8_t*)malloc(mMaxDataSize);
	}

	virtual ~SensorBean() {
		if (mDestroyCallback != NULL) {
			mDestroyCallback(this);
		}
		free(mData);
	}

	virtual ISensorDataType getDataType(void) {
		return mDataType;
	}

	virtual size_t getMaxDataSize(void) {
		return mMaxDataSize;
	}

	virtual bool getData(uint8_t* data) {
		if (mUpdateCallback != NULL) {
			mUpdateCallback(this);
		}
		memcpy(data, mData, mMaxDataSize);
		return true;
	}

	virtual const char* getDescription(void) {
		return mName.c_str();
	}

	virtual ISensorUnit getUnit(void) {
		return mUnit;
	}

	std::string getName(void) {
		return mName;
	}

	virtual log4cxx::LoggerPtr getLogger(void) {
		static log4cxx::LoggerPtr logger;
		return logger;
	}

	void setData(string value) {
		memcpy(mData, value.c_str(), min(mMaxDataSize, value.length() + 1)); // +1 for \0
	}

	void setData(uint32_t value) {
		uint32_t val = htonl(value); // val is now Big Endian
		// Copy only the requested number of bytes, skipping the unused first ones
		memcpy(mData, ((uint8_t*)(&val)) + (4 - mMaxDataSize), mMaxDataSize);
	}

	void setData(uint64_t value) {
		uint64_t val = htonll(value); // val is now Big Endian
		// Copy only the requested number of bytes, skipping the unused first ones
		memcpy(mData, ((uint8_t*)(&val)) + (8 - mMaxDataSize), mMaxDataSize);
	}

	void setData(double value) {
		double* pToDouble = &value;
		char* bytes = reinterpret_cast<char*>(pToDouble);
		memcpy(mData, bytes, mMaxDataSize);
	}

	void setUpdateCallback(updateSensorCallback_t callback) {
		mUpdateCallback = callback;
	}

	void setDestroyCallback(destroySensorCallback_t callback) {
		mDestroyCallback = callback;
	}

	void* mTag;

private:
	std::string mName;
	ISensorDataType mDataType;
	ISensorUnit mUnit;
	size_t mMaxDataSize;
	uint8_t* mData;
	updateSensorCallback_t mUpdateCallback;
	destroySensorCallback_t mDestroyCallback;

	uint64_t htonll(uint64_t value) {
		// The answer is 42
		static const int num = 42;

		// Check the endianness
		if (*reinterpret_cast<const char*>(&num) == num) {
			const uint32_t high_part = htonl(static_cast<uint32_t>(value >> 32));
			const uint32_t low_part = htonl(static_cast<uint32_t>(value & 0xFFFFFFFFLL));

			return (static_cast<uint64_t>(low_part) << 32) | high_part;
		} else {
			return value;
		}
	}
};

#endif /* SENSORBEAN_H_ */
