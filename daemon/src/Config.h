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
// Created on: 01.07.2009
// Author: Stefan Krupop <stefan.krupop@christmann.info>
////////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <sstream>
#include <list>
#include <log4cxx/logger.h>
#include <IConfig.h>
#include "ConfigChangeListener.h"
#include "ini_manage.h"

//#define WRITE_DEFAULTS_ON_ACCESS

using namespace std;

#ifndef JENKINS
	#define CONFIG_FILE_ACCESS
#endif

#define CONFIG_FILE		"conf/recsdaemon.ini"

class Config : IConfig{
public:
    static Config* GetInstance();
    void saveConfig();
    void shutdown() const;
    string getConfigAsString() const;

    void registerConfigChangeHandler(ConfigChangeListener* handler);
    void removeConfigChangeHandler(ConfigChangeListener* handler);
    void setDisableChangeListeners(bool state);

    bool GetBoolean(string Section, string Setting, bool Default);
    bool SetBoolean(string Section, string Setting, bool Value);
    string GetString(string Section, string Setting, string Default);
    bool SetString(string Section, string Setting, string Value);
    int GetInt(string Section, string Setting, int Default);
    bool SetInt(string Section, string Setting, int Value);
    double GetDouble(string Section, string Setting, double Default);
    bool SetDouble(string Section, string Setting, double Value);

protected:
	Config();
	~Config();

private:
	//lint -e(1704)
	Config(const Config& cSource);
	Config& operator=(const Config& cSource);

#ifdef CONFIG_FILE_ACCESS
	void notifyChangeHandlers();
#endif
	static void addDescription(string Section, string Setting, string Description);

    static Config* mInstance;
    bool mNoSaveMode;
    bool mConfigChanged;

	list<ConfigChangeListener*>* mListeners;
	bool mDisableChangeListeners;

	static log4cxx::LoggerPtr logger;
};

#endif /* CONFIG_H_ */
