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

#include "SensorProviderZynq.h"

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

using namespace std;
using namespace log4cxx;

LoggerPtr SensorProviderZynq::logger;
IConfig* SensorProviderZynq::config;

uint64_t SensorProviderZynq::mBaseAddress = 0;
int SensorProviderZynq::mMemoryFd = -1;
uint8_t* SensorProviderZynq::mMemory = NULL;
off_t SensorProviderZynq::mMemoryPageOffset = 0;

#define REGISTERSPACE_SIZE			0x1C

#define OFFSET_BUS_UTILIZATION		0x0
#define OFFSET_DATA_WITH			0x4
#define OFFSET_STATUS				0x8
#define OFFSET_SOFT_ERRROR_COUNT	0xC
#define OFFSET_FRAME_ERRROR_COUNT	0x10
#define OFFSET_BUS_FREQUENCY		0x14
#define OFFSET_BUS_WITH				0x18

void * SensorProviderZynq::create(PF_ObjectParams *) {
	return new SensorProviderZynq();
}

int32_t SensorProviderZynq::destroy(void * p) {
	if (!p)
		return -1;
	delete static_cast<SensorProviderZynq*>(p);
	return 0;
}

SensorProviderZynq::SensorProviderZynq() :
	mSensors() {

	mBaseAddress = config->GetInt("Plugins", "auroraMonitorBaseAddress", 0);
	if (mBaseAddress == 0) {
		LOG4CXX_ERROR(logger, "Base address of Aurora monitor not configured (Plugins->auroraMonitorBaseAddress)");
		return;
	}

	SensorBean* sensor = new SensorBean("Aurora link", TYPE_STR, 82, 1, UNIT_DIMENSIONLESS, false, false, 0, 0, 0, 0, "", RENDERING_TEXTUAL);
	sensor->setData((uint32_t)0);
	sensor->setUpdateCallback(&SensorProviderZynq::updateLink);
	mSensors["Aurora link"] = sensor;

	sensor = new SensorBean("Aurora util.", TYPE_U8, 1, 1, UNIT_PERCENT, false, false, 0, 0, 0, 0, "", RENDERING_TEXTUAL);
	sensor->setData((uint32_t)0);
	sensor->setUpdateCallback(&SensorProviderZynq::updateUtilization);
	mSensors["Aurora util."] = sensor;

	sensor = new SensorBean("Aurora frame err.", TYPE_U32, 4, 1, UNIT_DIMENSIONLESS, false, false, 0, 0, 0, 0, "", RENDERING_TEXTUAL);
	sensor->setData((uint32_t)0);
	sensor->setUpdateCallback(&SensorProviderZynq::updateFrameErr);
	mSensors["Aurora frame err."] = sensor;

	sensor = new SensorBean("Aurora soft err.", TYPE_U32, 4, 1, UNIT_DIMENSIONLESS, false, false, 0, 0, 0, 0, "", RENDERING_TEXTUAL);
	sensor->setData((uint32_t)0);
	sensor->setUpdateCallback(&SensorProviderZynq::updateSoftErr);
	mSensors["Aurora soft err."] = sensor;

	if (mMemory == NULL) {
		mapMemory();
	}
	if (mMemory != NULL) {
		sensor = new SensorBean("Aurora bandw.", TYPE_U32, 1, 1, UNIT_BYTE_SECOND, false, false, 0, 0, 0, 0, "", RENDERING_TEXTUAL);
		uint32_t bus_frequency = *(uint32_t*)(mMemory + mMemoryPageOffset + OFFSET_BUS_FREQUENCY);
		uint32_t bus_width = *(uint32_t*)(mMemory + mMemoryPageOffset + OFFSET_BUS_WITH);
		LOG4CXX_INFO(logger, "Aurora is " << bus_width << " bits wide @ " << bus_frequency << " Hz");
		sensor->setData((uint64_t)bus_frequency * (uint64_t)bus_width / 8); // /8 because of byte/s, not bit/s
		mSensors["Aurora bandw."] = sensor;
	}
}

SensorProviderZynq::~SensorProviderZynq() {
	if (mMemoryFd > 0) {
		close(mMemoryFd);
		mMemoryFd = -1;
	}
}

map<string, ISensor*> SensorProviderZynq::getSensors(void) {
	return mSensors;
}

void SensorProviderZynq::updateLink(SensorBean* sensor) {
	if (mMemory == NULL) {
		mapMemory();
	}
	if (mMemory != NULL) {
		//LOG4CXX_DEBUG(logger, "Reading address " << hex << ((uint32_t*)(mMemory + mMemoryPageOffset + OFFSET_STATUS)));
		uint32_t status = *(uint32_t*)(mMemory + mMemoryPageOffset + OFFSET_STATUS);
		//LOG4CXX_DEBUG(logger, "Got value 0x" << hex << status);

		std::stringstream ss;
		uint8_t channel = (status & 0x20) >> 5;
		if (channel == 1) {
			ss << "Channel up";
		} else {
			uint8_t lanes_avail = (status & 0xff000000) >> 24;
			uint8_t lanes = (status & 0xff0000) >> 16;
			if (lanes == lanes_avail) {
				ss << "Channel down (all lanes up)";
			} else if (lanes == 0x0) {
				ss << "Channel down (all lanes down)";
			} else {
				ss << "Channel down (lanes "; // 20 + 19 + 4 = 43 characters max.
				uint8_t first = 1;
				for (uint8_t i = 0; i < 8; ++i) {
					if ((lanes_avail & (1 << i)) && (lanes & (1 << i))) {
						if (!first) {
							ss << ", ";
						}
						ss << (i + 1);
						first = 0;
					}
				}
				ss << " up)";
			}
		}
		uint8_t hard_err = (status & 0x10) >> 4;
		if (hard_err) {
			ss << ", HardErr"; // 9 characters max.
		}
		uint8_t pll_not_locked = (status & 0x8) >> 3;
		if (pll_not_locked) {
			ss << ", PLL not locked"; // 16 characters max.
		}
		ss << " (0x" << hex << status << ")"; // 5 + 8 = 13 characters max.
		sensor->setData(ss.str());
	}
}

void SensorProviderZynq::updateUtilization(SensorBean* sensor) {
	if (mMemory == NULL) {
		mapMemory();
	}
	if (mMemory != NULL) {
		uint32_t bus_utilization = *(uint32_t*)(mMemory + mMemoryPageOffset + OFFSET_BUS_UTILIZATION);
		uint32_t data_width = *(uint32_t*)(mMemory + mMemoryPageOffset + OFFSET_DATA_WITH);

		double utilization = (double)bus_utilization / (double)(1 << data_width);
		sensor->setData((uint32_t)(utilization * 100.0));
	}
}

void SensorProviderZynq::updateFrameErr(SensorBean* sensor) {
	if (mMemory == NULL) {
		mapMemory();
	}
	if (mMemory != NULL) {
		uint32_t frame_err = *(uint32_t*)(mMemory + mMemoryPageOffset + OFFSET_FRAME_ERRROR_COUNT);

		sensor->setData((uint32_t)frame_err);
	}
}

void SensorProviderZynq::updateSoftErr(SensorBean* sensor) {
	if (mMemory == NULL) {
		mapMemory();
	}
	if (mMemory != NULL) {
		uint32_t soft_err = *(uint32_t*)(mMemory + mMemoryPageOffset + OFFSET_SOFT_ERRROR_COUNT);

		sensor->setData((uint32_t)soft_err);
	}
}

void SensorProviderZynq::mapMemory() {
    // Truncate offset to a multiple of the page size, or mmap will fail.
    size_t pagesize = sysconf(_SC_PAGE_SIZE);
    off_t page_base = (mBaseAddress / pagesize) * pagesize;
    mMemoryPageOffset = mBaseAddress - page_base;

    mMemoryFd = open("/dev/mem", O_SYNC);
    if (mMemoryFd < 0) {
    	LOG4CXX_ERROR(logger, "Could not open /dev/mem: Error " << errno);
    	mMemoryFd = -1;
    	return;
    }
	//LOG4CXX_DEBUG(logger, "Mapping 0x" << hex << (mMemoryPageOffset + DATA_2_RO_SIZE) << " bytes from page 0x" << hex << page_base << " to read address 0x" << hex << offset );
    mMemory = (uint8_t*)mmap(NULL, mMemoryPageOffset + REGISTERSPACE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, mMemoryFd, page_base);
	//LOG4CXX_DEBUG(logger, "Memory mapped to 0x" << hex << mMemory);
    if (mMemory == MAP_FAILED) {
    	LOG4CXX_ERROR(logger, "Could not mmap memory address 0x" << hex << mBaseAddress << " (page 0x" << hex << page_base << "): Error " << errno);
        mMemory = NULL;
    }
}
