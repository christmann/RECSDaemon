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

#include "SensorProviderSystem.h"

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <object_model.h>
#include <SensorBean.h>
#include <fcntl.h>
#ifndef WIN32
#include <sys/mman.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <sys/statvfs.h>
#else
#define WINVER 0x0501
#include <Windows.h>
#endif

using namespace std;

LoggerPtr SensorProviderSystem::logger;
uint64_t SensorProviderSystem::mLastTotalTime = 0;
uint64_t SensorProviderSystem::mLastWorkTime = 0;

void * SensorProviderSystem::create(PF_ObjectParams *) {
	return new SensorProviderSystem();
}

int32_t SensorProviderSystem::destroy(void * p) {
	if (!p)
		return -1;
	delete static_cast<SensorProviderSystem*>(p);
	return 0;
}

SensorProviderSystem::SensorProviderSystem() :
	mSensors() {
	SensorBean* sensor = new SensorBean("CPU", TYPE_FLOAT, 8, 1, UNIT_PERCENT, false, false, 0, 0, 0, 0, "", RENDERING_TEXTUAL);
	sensor->setData((uint32_t)0);
	sensor->setUpdateCallback(&SensorProviderSystem::updateCpuUtilization);
	mSensors["CPU"] = sensor;

	sensor = new SensorBean("Memory free", TYPE_U64, 8, 1, UNIT_BYTE, false, false, 0, 0, 0, 0, "", RENDERING_TEXTUAL);
	sensor->setData((uint64_t)0);
	sensor->setUpdateCallback(&SensorProviderSystem::updateMemoryFree);
	mSensors["Memory free"] = sensor;

	sensor = new SensorBean("System disk free", TYPE_U64, 8, 1, UNIT_BYTE, false, false, 0, 0, 0, 0, "", RENDERING_TEXTUAL);
	sensor->setData((uint64_t)0);
	sensor->setUpdateCallback(&SensorProviderSystem::updateDiskFree);
	mSensors["System disk free"] = sensor;
}

SensorProviderSystem::~SensorProviderSystem() {
}

map<string, ISensor*> SensorProviderSystem::getSensors(void) {
	return mSensors;
}

#ifdef WIN32
uint64_t SensorProviderSystem::filetimeToUint64(const FILETIME &v) {
    LARGE_INTEGER lv;
    lv.LowPart = v.dwLowDateTime;
    lv.HighPart = v.dwHighDateTime;

    return lv.QuadPart;
}
#endif

void SensorProviderSystem::updateCpuUtilization(SensorBean* sensor) {
	uint64_t totalTime = 0;
	uint64_t workTime = 0;

#ifdef WIN32
	FILETIME idleTime;
	FILETIME kernelTime;
	FILETIME userTime;
	GetSystemTimes(&idleTime, &kernelTime, &userTime);

	totalTime = filetimeToUint64(kernelTime) + filetimeToUint64(userTime); // kernel includes idle
	workTime = totalTime - filetimeToUint64(idleTime);
#else
	string line;

	ifstream myfile("/proc/stat");
	if (myfile.is_open()) {
		getline(myfile, line);
		std::istringstream stream(line);

		// Skip "cpu"
		stream.ignore(3, ' ');

		for (uint8_t i = 0; i < 10; ++i) {
			uint64_t n;
			stream >> n;
			if (!stream) {
				break;
			}

			// Sum up work- and total jiffies spent. First 3 are user, nice and system
			if (i < 3) {
				workTime += n;
			}
			totalTime += n;
		}
		myfile.close();
	} else {
		LOG_ERROR(logger, "Could not read file '/proc/stat'");
		return;
	}
#endif
	// On first update only get current time and values
	if (mLastTotalTime == 0) {
		mLastTotalTime = totalTime;
		mLastWorkTime = workTime;
		return;
	}

    double work_over_period = workTime - mLastWorkTime;
    double total_over_period = totalTime - mLastTotalTime;

    double utilization = (work_over_period / total_over_period) * 100.0;

	//LOG_DEBUG(logger, "CPU: " << utilization << " %");

	sensor->setData(utilization);

	mLastTotalTime = totalTime;
	mLastWorkTime = workTime;
}

void SensorProviderSystem::updateMemoryFree(SensorBean* sensor) {
	uint64_t freeMem;
#ifdef WIN32
	  MEMORYSTATUSEX statex;

	  statex.dwLength = sizeof(statex);
	  GlobalMemoryStatusEx(&statex);
	  freeMem = statex.ullAvailPhys;
#else
	string line;

	ifstream myfile("/proc/meminfo");
	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			std::istringstream stream(line);

			string prefix;
			stream >> prefix;

			if (prefix == "MemAvailable:") {
				stream >> freeMem;
			}
		}
		myfile.close();
	} else {
		LOG_ERROR(logger, "Could not read file '/proc/meminfo'");
		return;
	}

	freeMem *= 1024;
#endif
	//LOG_DEBUG(logger, "Free Mem: " << freeMem << " bytes");

	sensor->setData(freeMem);
}

void SensorProviderSystem::updateDiskFree(SensorBean* sensor) {
	uint64_t freeDisk;
#ifdef WIN32
	ULARGE_INTEGER totalNumberOfFreeBytes;

	GetDiskFreeSpaceEx("C:\\", NULL, NULL, &totalNumberOfFreeBytes);
	freeDisk = (uint64_t)totalNumberOfFreeBytes.QuadPart;
#else
	struct statvfs stat;
	statvfs("/", &stat);
	freeDisk = stat.f_bsize * stat.f_bavail;
#endif
	//LOG_DEBUG(logger, "Free Disk: " << freeDisk << " bytes");

	sensor->setData(freeDisk);
}
