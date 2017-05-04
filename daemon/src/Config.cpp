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
// Created on: 01.07.2009
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "Config.h"

using namespace std;
using namespace log4cxx;

Config* Config::mInstance = NULL;
LoggerPtr Config::logger(Logger::getLogger("Config"));

Config* Config::GetInstance() {
	if (mInstance == NULL) {
		mInstance = new Config();
	}
	return mInstance;
}

Config::Config() {
#ifdef CONFIG_FILE_ACCESS
	if (!ini_start(CONFIG_FILE)) {
		mNoSaveMode = true;
		LOG4CXX_WARN(logger, "Could not load config file, will not save configuration changes!");
	} else {
		LOG4CXX_INFO(logger, CONFIG_FILE << " loaded");
		mNoSaveMode = false;
	}
#else
	mNoSaveMode = true;
	LOG4CXX_WARN(logger, "Do not load config file, use default values!");
#endif
	mConfigChanged = false;
	mListeners = new list<ConfigChangeListener*>();
	mDisableChangeListeners = false;
}

Config::~Config() {
	LOG4CXX_DEBUG(logger, "Shutting down...");
	mListeners->clear();
	delete mListeners;
	if (mConfigChanged && !mNoSaveMode) {
		ini_end();
	}
}

void Config::shutdown() const {
	delete mInstance;
	mInstance = 0;
}

void Config::saveConfig() {
	if (!mNoSaveMode) {
		save();
		mConfigChanged = false;
	} else {
		LOG4CXX_WARN(logger, "NoSaveMode enabled, will not store configuration!");
	}
}

string Config::getConfigAsString() const {
#ifdef CONFIG_FILE_ACCESS
	return content_string();
#else
	return "[config]\nno_Access=Compile_with_CONFIG_FILE_ACCESS!\n";
#endif
}

void Config::registerConfigChangeHandler(ConfigChangeListener* handler) {
	mListeners->push_back(handler);
}

void Config::removeConfigChangeHandler(ConfigChangeListener* handler) {
	mListeners->remove(handler);
}

#ifdef CONFIG_FILE_ACCESS
void Config::notifyChangeHandlers() {
	mConfigChanged = true;
	if (!mDisableChangeListeners) {
		for (list<ConfigChangeListener*>::iterator it = mListeners->begin(); it != mListeners->end(); ++it) {
			(*it)->configChanged();
		}
	}
}
#endif

void Config::setDisableChangeListeners(bool state) {
	mDisableChangeListeners = state;
}

bool Config::GetBoolean(string Section, string Setting, bool Default) {
#ifdef CONFIG_FILE_ACCESS
	string buffer = get_value(Section.c_str(), Setting.c_str());
	if (buffer != "") {
		if (buffer == "wahr" || buffer == "Wahr" || buffer == "WAHR" || buffer == "true" || buffer == "True"  || buffer == "TRUE") {
			return true;
		} else {
			return false;
		}
	} else {
#ifdef WRITE_DEFAULTS_ON_ACCESS
		SetBoolean(Section, Setting, Default);
#endif
		return Default;
	}
#else
	return Default;
#endif
}

bool Config::SetBoolean(string Section, string Setting, bool Value) {
#ifdef CONFIG_FILE_ACCESS
	bool ret = set_value(Section.c_str(), Setting.c_str(), Value ? "true" : "false");
	GetInstance()->notifyChangeHandlers();
	return ret;
#else
	return false;
#endif
}

string Config::GetString(string Section, string Setting, string Default) {
#ifdef CONFIG_FILE_ACCESS
	string buffer = get_value(Section.c_str(), Setting.c_str());
	if (buffer != "") {
		return buffer;
	} else {
#ifdef WRITE_DEFAULTS_ON_ACCESS
		SetString(Section, Setting, Default);
#endif
		return Default;
	}
#else
	return Default;
#endif
}

bool Config::SetString(string Section, string Setting, string Value) {
#ifdef CONFIG_FILE_ACCESS
	bool ret = set_value(Section.c_str(), Setting.c_str(), Value.c_str());
	GetInstance()->notifyChangeHandlers();
	return ret;
#else
	return false;
#endif
}

int Config::GetInt(string Section, string Setting, int Default) {
#ifdef CONFIG_FILE_ACCESS
	string buffer = get_value(Section.c_str(), Setting.c_str());

	if (buffer != "") {
		int value;
	    istringstream strin;
	    strin.str(buffer);
	    strin >> value;
	    return value;
	} else
	{
#ifdef WRITE_DEFAULTS_ON_ACCESS
		SetInt(Section, Setting, Default);
#endif
		return Default;
	}
#else
	return Default;
#endif
}

bool Config::SetInt(string Section, string Setting, int Value) {
#ifdef CONFIG_FILE_ACCESS
    ostringstream strout;
    strout << Value;

    string val = strout.str();

	bool ret = set_value(Section.c_str(), Setting.c_str(), val.c_str());
	GetInstance()->notifyChangeHandlers();
	return ret;
#else
	return false;
#endif
}

double Config::GetDouble(string Section, string Setting, double Default) {
#ifdef CONFIG_FILE_ACCESS
	string buffer = get_value(Section.c_str(), Setting.c_str());

	if (buffer != "") {
		double value;
	    istringstream strin;
	    strin.str(buffer);
	    strin >> value;
	    return value;
	} else {
#ifdef WRITE_DEFAULTS_ON_ACCESS
		SetDouble(Section, Setting, Default);
#endif
		return Default;
	}
#else
	return Default;
#endif
}

bool Config::SetDouble(string Section, string Setting, double Value) {
#ifdef CONFIG_FILE_ACCESS
    ostringstream strout;
    strout << Value;

    string val = strout.str();

	bool ret = set_value(Section.c_str(), Setting.c_str(), val.c_str());
	GetInstance()->notifyChangeHandlers();
	return ret;
#else
	return false;
#endif
}
