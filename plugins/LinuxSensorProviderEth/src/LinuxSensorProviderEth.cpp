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

#include "LinuxSensorProviderEth.h"

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
#include <sys/mman.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/reboot.h>
#include <time.h>

using namespace std;
using namespace log4cxx;

LoggerPtr LinuxSensorProviderEth::logger;

void * LinuxSensorProviderEth::create(PF_ObjectParams *) {
	return new LinuxSensorProviderEth();
}

int32_t LinuxSensorProviderEth::destroy(void * p) {
	if (!p)
		return -1;
	delete static_cast<LinuxSensorProviderEth*>(p);
	return 0;
}

LinuxSensorProviderEth::LinuxSensorProviderEth() :
	mSensors() {

	struct ifreq ifr;
	struct ifconf ifc;
	char buf[1024];

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock == -1) {
		LOG4CXX_ERROR(logger, "Could not create socket");
		return;
	};

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
		LOG4CXX_ERROR(logger, "Could not get list of interface addresses (SIOCGIFCONF)");
		return;
	}

	struct ifreq* it = ifc.ifc_req;
	const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

	for (; it != end; ++it) {
		strcpy(ifr.ifr_name, it->ifr_name);
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
			if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback

				string ifaceName(it->ifr_name);
				LOG4CXX_INFO(logger, "Found Ethernet interface " << ifaceName);

				AdapterInfo* tag = new AdapterInfo();
				tag->name = ifaceName;
				tag->lastUpdate.tv_nsec = 0;
				tag->lastUpdate.tv_sec = 0;
				tag->lastCountRx = 0;
				tag->lastCountTx = 0;

				SensorBean* sensor = new SensorBean(ifaceName + " link", TYPE_STR, 6 + 8, 1, UNIT_DIMENSIONLESS, false, false, 0, 0, 0, 0, "", RENDERING_TEXTUAL);
				sensor->setData((uint32_t)0);
				sensor->setUpdateCallback(&LinuxSensorProviderEth::updateLinkStatus);
				sensor->setDestroyCallback(&LinuxSensorProviderEth::destroySensor);
				sensor->mTag = tag;
				mSensors[ifaceName + " link"] = sensor;

				sensor = new SensorBean(ifaceName + " util. RX", TYPE_U32, 4, 1, UNIT_BYTE_SECOND, false, false, 0, 0, 0, 0, "", RENDERING_TEXTUAL);
				sensor->setData((uint32_t)0);
				sensor->setUpdateCallback(&LinuxSensorProviderEth::updateUtilization);
				sensor->mTag = tag;
				mSensors[ifaceName + " util. RX"] = sensor;

				sensor = new SensorBean(ifaceName + " util. TX", TYPE_U32, 4, 1, UNIT_BYTE_SECOND, false, false, 0, 0, 0, 0, "", RENDERING_TEXTUAL);
				sensor->setData((uint32_t)0);
				sensor->mTag = tag;
				mSensors[ifaceName + " util. TX"] = sensor;
				tag->txSensor = sensor;
			}
		} else {
			LOG4CXX_ERROR(logger, "Could not get interface flags (SIOCGIFFLAGS)");
		}
	}
}

LinuxSensorProviderEth::~LinuxSensorProviderEth() {
}

map<string, ISensor*> LinuxSensorProviderEth::getSensors(void) {
	return mSensors;
}

void LinuxSensorProviderEth::updateLinkStatus(SensorBean* sensor) {
	AdapterInfo* tag = static_cast<AdapterInfo*>(sensor->mTag);

	string line;
	sensor->setData("Unknown");

	ifstream myfile(("/sys/class/net/" + tag->name + "/carrier").c_str());
	if (myfile.is_open()) {
		getline(myfile, line);
		myfile.close();
	} else {
		LOG4CXX_ERROR(logger, "Could not open file '/sys/class/net/" << tag->name << "/carrier'");
		return;
	}

	if (line == "1") {
		ifstream myfile2(("/sys/class/net/" + tag->name + "/speed").c_str());
		if (myfile2.is_open()) {
			getline(myfile2, line);
			sensor->setData(line + " MBit/s");
			myfile2.close();
		} else {
			LOG4CXX_ERROR(logger, "Could not open file '/sys/class/net/" << tag->name << "/speed'");
			return;
		}
	} else {
		sensor->setData("Down");
	}
}

void LinuxSensorProviderEth::updateUtilization(SensorBean* sensor) {
	AdapterInfo* tag = static_cast<AdapterInfo*>(sensor->mTag);

	uint64_t rxBytes = 0;
	uint64_t txBytes = 0;

	string line;

	ifstream myfile(("/sys/class/net/" + tag->name + "/statistics/rx_bytes").c_str());
	if (myfile.is_open()) {
		getline(myfile, line);
		std::istringstream i(line);
		uint64_t value;
		if ((i >> value)) {
			rxBytes = value;
		} else {
			LOG4CXX_WARN(logger, "Could not parse rx_bytes");
			return;
		}
		myfile.close();
	} else {
		LOG4CXX_ERROR(logger, "Could not read file '/sys/class/net/" << tag->name << "/statistics/rx_bytes'");
		return;
	}

	ifstream myfile2(("/sys/class/net/" + tag->name + "/statistics/tx_bytes").c_str());
	if (myfile2.is_open()) {
		getline(myfile2, line);
		std::istringstream i(line);
		uint64_t value;
		if ((i >> value)) {
			txBytes = value;
		} else {
			LOG4CXX_WARN(logger, "Could not parse tx_bytes");
			return;
		}
		myfile2.close();
	} else {
		LOG4CXX_ERROR(logger, "Could not read file '/sys/class/net/" << tag->name << "/statistics/tx_bytes'");
		return;
	}

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);

	// On first update only get current time and values
	if (tag->lastUpdate.tv_sec == 0) {
		tag->lastUpdate = now;
		tag->lastCountRx = rxBytes;
		tag->lastCountTx = txBytes;
		return;
	}

	double diff = ( now.tv_sec - tag->lastUpdate.tv_sec )
	  + (( now.tv_nsec - tag->lastUpdate.tv_nsec )
	  / 1000000000.0);

	uint32_t rxDiff = ((double)(rxBytes - tag->lastCountRx) / diff);
	uint32_t txDiff = ((double)(txBytes - tag->lastCountTx) / diff);

	//LOG4CXX_DEBUG(logger, "RX: " << rxDiff << ", TX: " << txDiff);

	sensor->setData(rxDiff);
	tag->txSensor->setData(txDiff);

	tag->lastUpdate = now;
	tag->lastCountRx = rxBytes;
	tag->lastCountTx = txBytes;
}

void LinuxSensorProviderEth::destroySensor(SensorBean* sensor) {
	delete static_cast<AdapterInfo*>(sensor->mTag);
}
