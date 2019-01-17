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
// Created on: 11.12.2015
////////////////////////////////////////////////////////////////////////////////

#ifndef DAEMON_H_
#define DAEMON_H_

#include <list>
#include <logger.h>
#include <stdint.h>
#include <pthread.h>
#include "object_model.h"

class Daemon {
public:
	Daemon();
	virtual ~Daemon();

	int run(int exitAfter);
	void resetStatemachine(void);
	ssize_t doRead(size_t offset, void* buf, size_t count);
	LoggerPtr* addPluginLogger(std::string name);
	int8_t getSlot();
	void shutdown(void);

private:
	enum State {
		State_MonitoringData,
		State_MonitoringDescription,
		State_BasicInformation
	};

	static void* InvokeService(const uint8_t * serviceName, void * serviceParams);
	static void signal_handler(int sig);

	std::list<LoggerPtr>* mPluginLoggers;
	bool mShutdown;
	bool mFirstWriteDone;
	State mState;
	uint8_t mCurrentPage;
	ICommunicator* mComm;
	pthread_mutex_t mCommMutex;
	int8_t mSlot;

	static LoggerPtr logger;
	static Daemon* instance;
};

#endif /* DAEMON_H_ */
