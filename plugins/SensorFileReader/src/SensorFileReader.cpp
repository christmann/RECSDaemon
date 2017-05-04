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

#include "SensorFileReader.h"

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "daemon_msgs.h"

using namespace std;
using namespace log4cxx;

LoggerPtr SensorFileReader::logger;

void * SensorFileReader::create(PF_ObjectParams *) {
	return new SensorFileReader();
}

int32_t SensorFileReader::destroy(void * p) {
	if (!p)
		return -1;
	delete static_cast<SensorFileReader*>(p);
	return 0;
}

SensorFileReader::SensorFileReader() : mPath(""), mDataType(TYPE_NONE), mUnit(UNIT_DIMENSIONLESS), mMultiplier(1.0) {
}

SensorFileReader::~SensorFileReader() {
}

bool SensorFileReader::configure(const char* data) {
	BaseSensor::configure(data);

	string options(data);
	mPath = getOption(options, "path");
	if (mPath == "") {
		LOG4CXX_ERROR(logger, "'path' attribute empty, can not initialize");
		return false;
	}
	string dataType = getOption(options, "type");
	if (dataType == "U8") {
		mDataType = TYPE_U8;
	} else if (dataType == "U16") {
		mDataType = TYPE_U16;
	} else if (dataType == "double") {
		mDataType = TYPE_FLOAT;

		string multiplier = getOption(options, "multiplier");
		if (multiplier != "") {
			std::istringstream i(multiplier);
			double value;
			if ((i >> value)) {
				mMultiplier = value;
			} else {
				LOG4CXX_WARN(logger, "Invalid 'multiplier' attribute: Could not parse '" << multiplier << "' as double");
			}
		}
	} else {
		LOG4CXX_ERROR(logger, "Invalid 'type' attribute: Unknown type '" << dataType << "'");
		return false;
	}

	string unit = getOption(options, "unit");
	if (unit == "°C") {
		mUnit = UNIT_TEMPERATURE;
	}

	return true;
}

ISensorDataType SensorFileReader::getDataType(void) {
	return mDataType;
}

size_t SensorFileReader::getMaxDataSize(void) {
	switch (mDataType) {
		case TYPE_U8: return 1;
		case TYPE_U16: return 2;
		case TYPE_FLOAT: return 8;
		default: return 0;
	}
}

bool SensorFileReader::getData(uint8_t* data) {
	string fileContent = "";

	string line;
	ifstream myfile(mPath.c_str());
	bool first = true;
	if (myfile.is_open()) {
		while ( getline (myfile, line) ) {
			if (first) {
				fileContent += line;
				first = false;
			} else {
				fileContent += "/n" + line;
			}
		}
		myfile.close();
	} else {
		LOG4CXX_ERROR(logger, "Could not read file '" << mPath << "'");
		return false;
	}

	if (mDataType != TYPE_STR) {
		std::istringstream i(fileContent);

		if (mDataType == TYPE_U8) {
			int value;
			if ((i >> value)) {
				data[0] = (uint8_t)(value & 0xff);
				return true;
			} else {
				LOG4CXX_WARN(logger, "Could not parse file content as uint8_t");
				return false;
			}
		} else if (mDataType == TYPE_U16) {
			uint16_t value;
			if ((i >> value)) {
				data[0] = (value >> 8) & 0xff;
				data[1] = value & 0xff;
				return true;
			} else {
				LOG4CXX_WARN(logger, "Could not parse file content as uint16_t");
				return false;
			}
		} else if (mDataType == TYPE_FLOAT) {
			double value;
			if ((i >> value)) {
				value *= mMultiplier;

				double* pToDouble = &value;
				char* bytes = reinterpret_cast<char*>(pToDouble);
				memcpy(data, bytes, 8);
				return true;
			} else {
				LOG4CXX_WARN(logger, "Could not parse file content as double");
				return false;
			}
		}
	}

	return false;
}

const char* SensorFileReader::getDescription(void) {
	return "Sensor that reads data from a file and parses it";
}

ISensorUnit SensorFileReader::getUnit(void) {
	return mUnit;
}

log4cxx::LoggerPtr SensorFileReader::getLogger(void) {
	return logger;
}
