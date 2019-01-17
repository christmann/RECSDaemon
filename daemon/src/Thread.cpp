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

using namespace std;

#include <pthread.h> // Should be the first include
#include <iostream>
#include "Thread.h"

/* Linux with glibc:
 *   _REENTRANT to grab thread-safe libraries
 */
#ifndef WIN32
#  ifndef _REENTRANT
#    define _REENTRANT
#  endif
#endif

LoggerPtr Thread::logger(Logger::getLogger("Thread"));

Thread::Thread() : mArg(NULL) {
  running = false;
  mThreadId = pthread_self(); // Initialize mThreadId to a meaningful value
  if(pthread_mutex_init(&mRunningMutex, NULL)) {
    LOG_ERROR(logger, "Could not initialize mutex!");
  }
}

Thread::~Thread() {
  stop();
  mArg = NULL;
  pthread_mutex_destroy(&mRunningMutex);
}

int Thread::start(void *arg) {
  Arg(arg); // store user data
  //int code = thread_create(Thread::entryPoint, this, &mThreadId);

  int code = pthread_create(&mThreadId, NULL, &Thread::entryPoint, this);
  if (code > 0) {
	  LOG_ERROR(logger, "start: pthread create error");
	  return false;
  }
  return code;
}

void Thread::stop() {
  void* status;
  pthread_mutex_lock(&mRunningMutex);
  if (running) {
    running = false;
    pthread_mutex_unlock(&mRunningMutex);
    terminate();
    // wait for thread to terminate
    pthread_join(mThreadId, (void **)&status);
  } else {
	pthread_mutex_unlock(&mRunningMutex);
  }
}

void Thread::Run(void *arg) {
  pthread_mutex_lock(&mRunningMutex);
  if (!running) {
    if (setup()) {
    	running = true;
    	pthread_mutex_unlock(&mRunningMutex);
    	execute(arg);
    } else {
    	pthread_mutex_unlock(&mRunningMutex);
    }
  } else {
	  pthread_mutex_unlock(&mRunningMutex);
  }
}

bool Thread::IsRunning() {
	pthread_mutex_lock(&mRunningMutex);
	bool ret = running;
	pthread_mutex_unlock(&mRunningMutex);
	return ret;
}

/*static */
void* Thread::entryPoint(void *pthis) {
  Thread *pt = static_cast<Thread*>(pthis);

  pt->Run(pt->Arg());
  return NULL;
}

bool Thread::setup() {
	// Do any setup here
	return true;
}

void Thread::terminate() {
  // Do any termination here
}
