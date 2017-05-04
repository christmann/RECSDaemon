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

#ifndef JSONSENSORSPARSER_H_
#define JSONSENSORSPARSER_H_

#include <log4cxx/logger.h>
#include "object_model.h"
#include "json.h"
#include "SensorSet.h"
#include "SensorBean.h"

class JSONSensorsParser {
public:
	typedef std::map<std::string, ISensor* > SensorsMap;

	JSONSensorsParser(IJSONSensorProvider* sensorProvider, std::string name);
	virtual ~JSONSensorsParser();

	SensorsMap getSensors(void);
	void updateSensors(void);

	IJSONSensorProvider* getProvider();

private:
	IJSONSensorProvider* mSensorProvider;
	std::string mName;
	SensorsMap mSensors;
	std::vector<SensorBean*> mSensorsOrdered;

	static log4cxx::LoggerPtr logger;
};

#endif /* JSONSENSORSPARSER_H_ */
