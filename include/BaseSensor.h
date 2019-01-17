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

#ifndef BASESENSOR_H
#define BASESENSOR_H

#include <object_model.h>
#include <string>
#include <sstream>
#include <logger.h>

using namespace std;

class BaseSensor: public ISensor {
public:
	string getOption(string options, string name) {
		return extract_param(options, name, " ,");
	}

	// ISensor methods
	virtual ISensorDataType getDataType(void) = 0;
	virtual size_t getMaxDataSize(void) = 0;
	virtual bool getData(uint8_t* data) = 0;
	virtual const char* getDescription(void) = 0;
	virtual ISensorUnit getUnit(void) = 0;

	virtual LoggerPtr getLogger(void) = 0;

	virtual bool configure(const char* data) {
		string dataStr(data);
		string ret = getOption(dataStr, "lowerCriticalThreshold");
		if (!ret.empty()) {
			std::istringstream i(ret);
			if ((i >> mLowerCriticalThreshold)) {
				mUseLowerThresholds = true;
			} else {
				LOG_WARN(getLogger(), "Could not parse lowerCriticalThreshold '" << ret << "' as double");
			}
		}
		ret = getOption(dataStr, "lowerWarningThreshold");
		if (!ret.empty()) {
			std::istringstream i(ret);
			if ((i >> mLowerWarningThreshold)) {
				mUseLowerThresholds = true;
			} else {
				LOG_WARN(getLogger(), "Could not parse lowerWarningThreshold '" << ret << "' as double");
			}
		}

		ret = getOption(dataStr, "upperWarningThreshold");
		if (!ret.empty()) {
			std::istringstream i(ret);
			if ((i >> mUpperWarningThreshold)) {
				mUseUpperThresholds = true;
			} else {
				LOG_WARN(getLogger(), "Could not parse upperWarningThreshold '" << ret << "' as double");
			}
		}
		ret = getOption(dataStr, "upperCriticalThreshold");
		if (!ret.empty()) {
			std::istringstream i(ret);
			if ((i >> mUpperCriticalThreshold)) {
				mUseUpperThresholds = true;
			} else {
				LOG_WARN(getLogger(), "Could not parse upperCriticalThreshold '" << ret << "' as double");
			}
		}
		return true;
	}

	virtual bool getUseLowerThresholds(void) {
		return mUseLowerThresholds;
	}

	virtual bool getUseUpperThresholds(void) {
		return mUseUpperThresholds;
	}

	virtual double getLowerCriticalThreshold(void) {
		return mLowerCriticalThreshold;
	}

	virtual double getLowerWarningThreshold(void) {
		return mLowerWarningThreshold;
	}

	virtual double getUpperWarningThreshold(void) {
		return mUpperWarningThreshold;
	}

	virtual double getUpperCriticalThreshold(void) {
		return mUpperCriticalThreshold;
	}

	virtual uint16_t getNumberOfValues(void) {
		return mNumberOfValues;
	}

	virtual IRenderingType getRenderingType(void) {
		return mRendering;
	}

	virtual const char* getGroup(void) {
		return mGroup.c_str();
	}

protected:
	BaseSensor() {
		mUseLowerThresholds = false;
		mUseUpperThresholds = false;
		mLowerCriticalThreshold = 0.0;
		mLowerWarningThreshold = 0.0;
		mUpperWarningThreshold = 0.0;
		mUpperCriticalThreshold = 0.0;
		mGroup = "";
		mRendering = RENDERING_TEXTUAL;
		mNumberOfValues = 1;
	}

	bool mUseLowerThresholds;
	bool mUseUpperThresholds;
	double mLowerCriticalThreshold;
	double mLowerWarningThreshold;
	double mUpperWarningThreshold;
	double mUpperCriticalThreshold;
	string mGroup;
	IRenderingType mRendering;
	uint16_t mNumberOfValues;

private:
	string extract_param(const string haystack, const string needle, const string delim) {
		size_t needlelen = needle.length();
		if (!needlelen) {
			return "";
		}
		if (haystack.length() == 0)
			return "";

		size_t param_pos = haystack.find(needle);
		do {
			if (param_pos == string::npos)
				return "";
			// Needle followed by '='?
			if ((param_pos + needlelen) < haystack.length() && haystack.at(param_pos + needlelen) == '=') {
				// Beginning of the string?
				if (param_pos == 0)
					break;

				// After a delimiter?
				if (haystack.find_first_of(delim, param_pos - 1) != string::npos)
					break;
			}
			// Continue searching.
			param_pos++;
			param_pos = haystack.find(needle, param_pos);
		} while (1);

		if (param_pos != string::npos) {
			// Get the string after needle and '='.
			size_t opt_pos = param_pos + needlelen + 1;
			size_t opt_len = haystack.find_first_of(delim, opt_pos);
			if (opt_len == string::npos) {
				opt_len = haystack.length();
			}
			if (opt_len > opt_pos) {
				opt_len -= opt_pos;
				return haystack.substr(opt_pos, opt_len);
			}
		}

		return "";
	}
};

#endif
