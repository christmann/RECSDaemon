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
// Created on: 24.02.2015
////////////////////////////////////////////////////////////////////////////////

#ifndef NODE_H_
#define NODE_H_

#include <string>
#include <list>
#include <logger.h>
#include "Daemon.h"
#include "SensorSet.h"
#include "../include/daemon_msgs.h"

using namespace std;

class Node {
public:
	class AdapterInfo {
		public:
		uint8_t ipAddress[16];
		uint8_t netMask[4];
		uint8_t macAddress[6];
	};

	enum NodeType {
		NODE_APALIS,
		NODE_CXP
	};

	Node(string id, uint8_t baseboardID, uint8_t slot, NodeType nodeType);
	virtual ~Node();

	SensorSet* getSensors();
	string getID();
	uint8_t getBaseboardID();
	uint8_t getSlot();

	string executeCommand(string command, string parameters);
	list<AdapterInfo> getNetworkAdapters();
	AdapterInfo getComputeAdapter(list<AdapterInfo> adapters);
	AdapterInfo getManagementAdapter(list<AdapterInfo> adapters);
	string getHostName();

	size_t getBasicInformationBlock(uint8_t* buffer, size_t bufferSize);

	string getJSONMonitoringData(Daemon* daemon);
	uint16_t findHeaderDataChunkOffset(Daemon* daemon, uint8_t chunkType);
private:
	string shutdown(bool poweroff);
	uint16_t fromLitteEndian(uint16_t value);

	uint8_t mBaseboardID;
	uint8_t mSlot;
	string mID;
	SensorSet* mSensors;
	NodeType mNodeType;
	uint16_t mMonitoringDataOffset;

	static LoggerPtr logger;
};

#endif /* NODE_H_ */
