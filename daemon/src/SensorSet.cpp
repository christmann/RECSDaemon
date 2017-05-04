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
// Created on: 12.11.2014
////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cmath>
#include <cstring>
#include "network/Network.h" // For htonX
#include "SensorSet.h"
#include "Config.h"
#include "plugin_models/SensorFactory.h"
#include "plugin_models/SensorProviderFactory.h"
#include "plugin_models/JSONSensorProviderFactory.h"
#include "../include/daemon_msgs.h"
#include "JSONSensorsParser.h"

using namespace log4cxx;

LoggerPtr SensorSet::logger(Logger::getLogger("SensorSet"));

SensorSet::SensorSet() {
	int cnt = Config::GetInstance()->GetInt("Sensors", "count", 0);
	LOG4CXX_INFO(logger, cnt << " manual sensors configured");
	for (uint16_t i = 0; i < cnt; ++i) {
		stringstream sensorNrStream;
		sensorNrStream << "pluginName" << (i + 1);
		string pluginName = Config::GetInstance()->GetString("Sensors", sensorNrStream.str(), "");
		sensorNrStream.clear();
		sensorNrStream.str(std::string());
		sensorNrStream << "sensorName" << (i + 1);
		string sensorName = Config::GetInstance()->GetString("Sensors", sensorNrStream.str(), "");
		sensorNrStream.clear();
		sensorNrStream.str(std::string());
		sensorNrStream << "options" << (i + 1);
		string options = Config::GetInstance()->GetString("Sensors", sensorNrStream.str(), "");

		if (pluginName == "" || sensorName == "") {
			LOG4CXX_WARN(logger, "pluginName or sensorName can not be empty for sensor " << (i + 1) << ", skipping!");
			continue;
		}

		if (sensorName.length() > SENSOR_NAME_LENGTH) {
			LOG4CXX_ERROR(logger, "sensorName '" << sensorName << "' too long, max. " << SENSOR_NAME_LENGTH << " chars allowed!");
			continue;
		}

		LOG4CXX_INFO(logger, "Initializing sensor " << (i + 1) << ": " << pluginName << "(" << options << ") as " << sensorName);
		ISensor* sensor = SensorFactory::createSensor(pluginName);

		if (sensor != NULL) {
			if (sensor->configure(options.c_str())) {
				mSensorMap[sensorName] = sensor;
			} else {
				LOG4CXX_ERROR(logger, "Could not configure sensor " << pluginName << "!");
			}
		} else {
			LOG4CXX_ERROR(logger, "Could not initialize sensor " << pluginName << "!");
		}
	}

	{
		string SensorProviders = Config::GetInstance()->GetString("Plugins", "SensorProviders", "");
		std::stringstream ss(SensorProviders);
		std::string pluginName;
		while (std::getline(ss, pluginName, ',')) {
			LOG4CXX_INFO(logger, "Enabling sensor provider " << pluginName);
			ISensorProvider* SensorProvider = SensorProviderFactory::createSensorProvider(pluginName);

			if (SensorProvider != NULL) {
				SensorMap map =  SensorProvider->getSensors();
				mSensorMap.insert(map.begin(), map.end());
				LOG4CXX_INFO(logger, "Added " << map.size() << " sensors");

				delete SensorProvider;
			}
		}
	}

	{
		string JSONSensorProviders = Config::GetInstance()->GetString("Plugins", "JSONSensorProviders", "");
		std::stringstream ss(JSONSensorProviders);
		std::string pluginName;
		while (std::getline(ss, pluginName, ',')) {
			LOG4CXX_INFO(logger, "Enabling JSON sensor provider " << pluginName);
			IJSONSensorProvider* JSONSensorProvider = JSONSensorProviderFactory::createJSONSensorProvider(pluginName);

			if (JSONSensorProvider != NULL) {
				addJSONSensorProvider(JSONSensorProvider, pluginName);
			}
		}
	}

	mSize = sizeof(Monitoring_Data_Header);
	for (SensorMap::iterator iterator = mSensorMap.begin(); iterator != mSensorMap.end(); ++iterator) {
		mSize += iterator->second->getMaxDataSize();
	}
	mData = (uint8_t*)malloc(mSize);
}

bool SensorSet::addJSONSensorProvider(IJSONSensorProvider* provider, string name) {
	if (mJSONSensorsParsers.find(name) != mJSONSensorsParsers.end()) {
		return false;
	}
	JSONSensorsParser* jsonSensors = new JSONSensorsParser(provider, name);
	SensorMap map = jsonSensors->getSensors();
	mSensorMap.insert(map.begin(), map.end());
	mJSONSensorsParsers[name] = jsonSensors;
	LOG4CXX_INFO(logger, "Added " << map.size() << " sensors");
	return true;
}

IJSONSensorProvider* SensorSet::getJSONSensorProvider(std::string name) {
	JSONParsersMap::iterator iter = mJSONSensorsParsers.find(name);
	if (iter != mJSONSensorsParsers.end()) {
		return iter->second->getProvider();
	}
	return NULL;
}

size_t SensorSet::getSize() {
	return mSize;
}

uint8_t* SensorSet::getMessage() {
	Monitoring_Data_Header* header = (Monitoring_Data_Header*)mData;
	header->header.type = Monitoring_Data;
	header->header.size = htons(mSize);
	header->flags = 0;
	header->sensorCnt = htons(mSensorMap.size());

	// Update JSON sensors
	for (JSONParsersMap::iterator iterator = mJSONSensorsParsers.begin(); iterator != mJSONSensorsParsers.end(); ++iterator) {
		iterator->second->updateSensors();
	}

	size_t offset = sizeof(Monitoring_Data_Header);
	for (SensorMap::iterator iterator = mSensorMap.begin(); iterator != mSensorMap.end(); ++iterator) {
		iterator->second->getData(&mData[offset]);
		offset += iterator->second->getMaxDataSize();
	}
	return mData;
}

size_t SensorSet::getDescriptionPage(uint8_t* buffer, size_t bufferSize, uint8_t page, uint8_t* maxPages) {
	uint16_t sensorCnt = mSensorMap.size();
	bufferSize -= sizeof(Monitoring_Description_Header);
	size_t offset = sizeof(Monitoring_Description_Header);

	uint8_t sensorsPerPage = floor((double)bufferSize / (double)sizeof(Sensor_Description));
	*maxPages = ceil((double)sensorCnt / (double)sensorsPerPage);
	if (*maxPages < 1) {
		*maxPages = 1;
	}

	uint8_t sensorsOnPage = 0;
	if (page < *maxPages) {
		sensorsOnPage = sensorsPerPage;
	} else {
		sensorsOnPage = sensorCnt % sensorsPerPage;
		if (sensorsOnPage == 0 && sensorCnt > 0) {
			sensorsOnPage = sensorsPerPage;
		}
	}
	uint16_t startingSensor = (page - 1) * sensorsPerPage;

	SensorMap::iterator iterator = mSensorMap.begin();
	for (uint16_t i = 0; i < startingSensor; ++i) {
		++iterator;
	}
	for (uint16_t i = startingSensor; i < startingSensor + sensorsOnPage; ++i) {
		Sensor_Description* sensorDesc = (Sensor_Description*)&buffer[offset];

		strncpy((char*)&sensorDesc->name[0], iterator->first.c_str(), SENSOR_NAME_LENGTH);
		sensorDesc->dataType = iterator->second->getDataType();
		sensorDesc->unit = iterator->second->getUnit();
		sensorDesc->maxDataSize = htons(iterator->second->getMaxDataSize());
		sensorDesc->useLowerThresholds = iterator->second->getUseLowerThresholds();
		sensorDesc->useUpperThresholds = iterator->second->getUseUpperThresholds();
		sensorDesc->lowerCriticalThreshold = iterator->second->getLowerCriticalThreshold();
		sensorDesc->lowerWarningThreshold = iterator->second->getLowerWarningThreshold();
		sensorDesc->upperWarningThreshold = iterator->second->getUpperWarningThreshold();
		sensorDesc->upperCriticalThreshold = iterator->second->getUpperCriticalThreshold();
		sensorDesc->numberOfValues = htons(iterator->second->getNumberOfValues());
		sensorDesc->groupId = getGroupId(iterator->second->getGroup());
		sensorDesc->renderingType = iterator->second->getRenderingType();
		sensorDesc->entryLength = sizeof(Sensor_Description);

		++iterator;
		offset += sizeof(Sensor_Description);
	}

	Monitoring_Description_Header* header = (Monitoring_Description_Header*)buffer;
	header->header.type = Monitoring_Description;
	header->header.size = htons(offset);
	header->header.version = 0;
	header->currentPage = page;
	header->maxPages = *maxPages;
	header->sensorEntries = sensorsOnPage;
	header->startIndex = htons(startingSensor);

	return offset;
}

uint8_t SensorSet::getGroupId(const char* name) {
	if (name[0] == '\0') { // Empty string -> no group
		return 0;
	}

	string nameStr = string(name);
	if (mKnownGroups.find(nameStr) != mKnownGroups.end()) {
		return mKnownGroups[nameStr];
	} else {
		if (mKnownGroups.size() >= 255) {
			LOG4CXX_ERROR(logger, "Too many groups defined, available group IDs exhausted!");
			return 0;
		}
		uint8_t id = mKnownGroups.size() + 1;
		mKnownGroups[nameStr] = id;
		return id;
	}
}

void SensorSet::clear() {
	for (SensorMap::iterator iterator = mSensorMap.begin(); iterator != mSensorMap.end(); ++iterator) {
		delete iterator->second;
	}
	mSensorMap.clear();

	for (JSONParsersMap::iterator iterator = mJSONSensorsParsers.begin(); iterator != mJSONSensorsParsers.end(); ++iterator) {
		delete iterator->second;
	}
	mJSONSensorsParsers.clear();

	mKnownGroups.clear();
}

SensorSet::~SensorSet() {
	clear();
	free(mData);
}
