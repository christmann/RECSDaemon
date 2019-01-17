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

#ifndef SENSORSET_H_
#define SENSORSET_H_

#include <iostream>
#include <logger.h>
#include <map>
#include <vector>
#include "../include/object_model.h"

class JSONSensorsParser;

class SensorSet {
	typedef std::map<std::string, ISensor* > SensorMap;
	typedef std::map<std::string, JSONSensorsParser* > JSONParsersMap;

public:
	SensorSet();
	virtual ~SensorSet();

	bool addJSONSensorProvider(IJSONSensorProvider* provider, std::string name);
	IJSONSensorProvider* getJSONSensorProvider(std::string name);

	size_t getSize();
	uint8_t* getMessage();
	size_t getDescriptionPage(uint8_t* buffer, size_t bufferSize, uint8_t page, uint8_t* maxPages);
	void clear();

private:
	//lint -e(1704)
	SensorSet(const SensorSet& cSource);
	SensorSet& operator=(const SensorSet& cSource);

	uint8_t getGroupId(const char* name);

	SensorMap mSensorMap;
	JSONParsersMap mJSONSensorsParsers;
	std::map<std::string, int> mKnownGroups;
	size_t mSize;
	uint8_t* mData;

	static LoggerPtr logger;
};

#endif /* TELNETSERVER_H_ */
