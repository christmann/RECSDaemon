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

#include "JSONSensorsParser.h"
#include "SensorBean.h"
#include <daemon_msgs.h>

using namespace std;

LoggerPtr JSONSensorsParser::logger(Logger::getLogger("JSONSensorsParser"));

JSONSensorsParser::JSONSensorsParser(IJSONSensorProvider* sensorProvider, string name) {
	mSensorProvider = sensorProvider;
	mName = name;
}

JSONSensorsParser::~JSONSensorsParser() {
	// Sensor instances will be deleted by SensorSet
	mSensors.clear();
	mSensorsOrdered.clear();
	delete mSensorProvider;
}

JSONSensorsParser::SensorsMap JSONSensorsParser::getSensors(void) {
	for (SensorsMap::iterator iterator = mSensors.begin(); iterator != mSensors.end(); ++iterator) {
		delete iterator->second;
	}
	mSensors.clear();
	mSensorsOrdered.clear();

	string sensorsDescriptionString = mSensorProvider->getSensorsDescription();
	json::Value sensorsDescription = json::Deserialize(sensorsDescriptionString);
	if (sensorsDescription.GetType() == json::NULLVal) {
		LOG_ERROR(logger, "Could not parse sensors description for JSONSensorProvider " << mName);
		return mSensors;
	}

	LOG_DEBUG(logger, "Found " << sensorsDescription.size() << " sensors in description");
	for (uint16_t i = 0; i < sensorsDescription.size(); ++i) {
		json::Value sensor = sensorsDescription[i];
		if (sensor.GetType() == json::ObjectVal) {
			if (sensor.HasKey("name") && sensor.HasKey("dataType")) {
				string name = sensor["name"];
				if (name.length() > SENSOR_NAME_LENGTH) {
					LOG_ERROR(logger, "sensorName '" << name << "' too long, max. " << SENSOR_NAME_LENGTH << " chars allowed!");
					continue;
				}

				ISensorDataType dataType = TYPE_NONE;
				uint8_t maxDataSize = 0;
				string dataTypeStr = sensor["dataType"];
				if (dataTypeStr == "U8") {
					dataType = TYPE_U8;
					maxDataSize = 1;
				} else if (dataTypeStr == "U16") {
					dataType = TYPE_U16;
					maxDataSize = 2;
				} else if (dataTypeStr == "U32") {
					dataType = TYPE_U32;
					maxDataSize = 4;
				} else if (dataTypeStr == "U64") {
					dataType = TYPE_U64;
					maxDataSize = 8;
				} else if (dataTypeStr == "double") {
					dataType = TYPE_FLOAT;
					maxDataSize = 8;
				} else if (dataTypeStr == "string") {
					dataType = TYPE_STR;
					if (sensor.HasKey("maxDataSize")) {
						std::istringstream ss(sensor["maxDataSize"]);
						ss >> maxDataSize;
					} else {
						LOG_ERROR(logger, "Required property 'maxDataSize' missing");
						continue;
					}
				} else {
					LOG_ERROR(logger, "Invalid 'type' property: Unknown type '" << dataTypeStr << "'");
					continue;
				}
				ISensorUnit unit = UNIT_DIMENSIONLESS;
				if (sensor.HasKey("unit")) {
					string unitStr = sensor["unit"];
					if (unitStr == "W") {
						unit = UNIT_POWER;
					} else if (unitStr == "A") {
						unit = UNIT_CURRENT;
					} else if (unitStr == "V") {
						unit = UNIT_VOLTAGE;
					} else if (unitStr == "°C") {
						unit = UNIT_TEMPERATURE;
					} else if (unitStr == "RPM") {
						unit = UNIT_ROTATIONAL_SPEED;
					}
				}
				bool useLowerThresholds = false;
				double lowerCriticalThreshold = 0.0;
				double lowerWarningThreshold = 0.0;
				bool useUpperThresholds = false;
				double upperWarningThreshold = 0.0;
				double upperCriticalThreshold = 0.0;
				if (sensor.HasKey("lowerThresholds")) {
					json::Value thresholds = sensor["lowerThresholds"];
					if (thresholds.GetType() == json::ArrayVal && thresholds.size() >= 2) {
						useLowerThresholds = true;
						lowerCriticalThreshold = thresholds[(size_t)0].ToDouble(0.0);
						lowerWarningThreshold = thresholds[(size_t)1].ToDouble(0.0);
					} else {
						LOG_ERROR(logger, "Property 'lowerThresholds' is not an JSON array or smaller than two elements");
					}
				}
				if (sensor.HasKey("upperThresholds")) {
					json::Value thresholds = sensor["upperThresholds"];
					if (thresholds.GetType() == json::ArrayVal && thresholds.size() >= 2) {
						useUpperThresholds = true;
						upperWarningThreshold = thresholds[(size_t)0].ToDouble(0.0);
						upperCriticalThreshold = thresholds[(size_t)1].ToDouble(0.0);
					} else {
						LOG_ERROR(logger, "Property 'upperThresholds' is not an JSON array or smaller than two elements");
					}
				}

				uint16_t numberOfValues = 1;
				if (sensor.HasKey("numberOfValues")) {
					json::Value n = sensor["numberOfValues"];
					if (n.IsNumeric()) {
						numberOfValues = n.ToInt();
					}
				}

				string group = "";
				if (sensor.HasKey("group")) {
					group = sensor["group"].ToString();
				}

				IRenderingType renderingType = RENDERING_TEXTUAL;

				LOG_INFO(logger, "Adding sensor '" << name << "'");
				SensorBean* s = new SensorBean(name, dataType, maxDataSize, numberOfValues, unit, useLowerThresholds, useUpperThresholds, lowerCriticalThreshold, lowerWarningThreshold, upperWarningThreshold, upperCriticalThreshold, group, renderingType);
				mSensors.insert(pair<string, ISensor*>(name, s));
				mSensorsOrdered.push_back(s);
			} else {
				LOG_ERROR(logger, "Sensor description " << i << " is missing one or more of the required properties name and/or dataType");
			}
		} else {
			LOG_ERROR(logger, "Sensor description " << i << " is not a JSON object");
		}
	}

	return mSensors;
}

void JSONSensorsParser::updateSensors(void) {
	string sensorsDataString = mSensorProvider->getSensorsData();
	if (sensorsDataString == "") {
		return;
	}
	json::Value sensorsData = json::Deserialize(sensorsDataString);
	if (sensorsData.GetType() == json::NULLVal) {
		LOG_ERROR(logger, "Could not parse sensors data for JSONSensorProvider " << mName);
		return;
	}

	if (sensorsData.size() < mSensorsOrdered.size()) {
		LOG_ERROR(logger, "Data does not contain enough values: Found " << sensorsData.size() << " fields, expected " << mSensorsOrdered.size());
		return;
	}

	uint16_t i = 0;
	for (vector<SensorBean*>::iterator iterator = mSensorsOrdered.begin(); iterator != mSensorsOrdered.end(); ++iterator) {
		json::Value sensorValue = sensorsData[i];
		++i;

		if (sensorValue.GetType() == json::NULLVal || sensorValue.GetType() == json::ObjectVal || sensorValue.GetType() == json::ArrayVal ) {
			LOG_ERROR(logger, "Value for sensor " << (*iterator)->getName() << " of " << mName << "is of invalid type NULL, object or array");
			continue;
		}

		if ((*iterator)->getDataType() != TYPE_STR) {
			if (sensorValue.GetType() == json::DoubleVal) {
				(*iterator)->setData(sensorValue.ToDouble());
			} else if (sensorValue.IsNumeric()) {
				(*iterator)->setData((uint32_t)sensorValue.ToInt());
			} else {
				LOG_ERROR(logger, "Value for sensor " << (*iterator)->getName() << " of " << mName << "is not numeric");
				continue;
			}
		} else {
			(*iterator)->setData(sensorValue.ToString(""));
		}
	}
}

IJSONSensorProvider* JSONSensorsParser::getProvider() {
	return mSensorProvider;
}
