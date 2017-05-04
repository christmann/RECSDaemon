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

#include "Node.h"
#include <sstream>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <cstdlib>
#include <iomanip>
#ifndef WIN32
	#include <sys/ioctl.h>
	#include <net/if.h>
	#include <netinet/in.h>
	#include <sys/reboot.h>
#else
	#include <winsock2.h>
	typedef int socklen_t;
	#include <Iphlpapi.h>
#endif

#include "version.h"

#define CHRISTMANN_MAC_1	0x70
#define CHRISTMANN_MAC_2	0xB3
#define CHRISTMANN_MAC_3	0xD5
#define CHRISTMANN_MAC_4	0x56

using namespace log4cxx;
using namespace std;

LoggerPtr Node::logger(Logger::getLogger("Node"));

Node::Node(string id, uint8_t baseboardID, uint8_t slot, NodeType nodeType) : mBaseboardID(baseboardID), mSlot(slot), mID(id), mNodeType(nodeType) {
	mSensors = new SensorSet();
	list<Node::AdapterInfo> adapters = getNetworkAdapters();
	LOG4CXX_DEBUG(logger, "Hostname: " << getHostName());
	LOG4CXX_DEBUG(logger, "Detected " << adapters.size() << " network adapters");
	for (list<Node::AdapterInfo>::iterator iterator = adapters.begin(); iterator != adapters.end(); ++iterator) {
		LOG4CXX_DEBUG(logger, 	"Addr: " << (int)iterator->ipAddress[0] <<
									"." << (int)iterator->ipAddress[1] <<
									"." << (int)iterator->ipAddress[2] <<
									"." << (int)iterator->ipAddress[3] <<
								" Mask: " << (int)iterator->netMask[0] <<
									"." << (int)iterator->netMask[1] <<
									"." << (int)iterator->netMask[2] <<
									"." << (int)iterator->netMask[3] <<
								", MAC: " << hex << setfill('0') << setw(2) << (int)iterator->macAddress[0] <<
									":" << setw(2) << (int)iterator->macAddress[1] <<
									":" << setw(2) << (int)iterator->macAddress[2] <<
									":" << setw(2) << (int)iterator->macAddress[3] <<
									":" << setw(2) << (int)iterator->macAddress[4] <<
									":" << setw(2) << (int)iterator->macAddress[5]);
	}
	Node::AdapterInfo compute = getComputeAdapter(adapters);
	if (compute.ipAddress[0] != 0) {
		LOG4CXX_DEBUG(logger, 	"Compute adapter: Addr: " << (int)compute.ipAddress[0] <<
									"." << (int)compute.ipAddress[1] <<
									"." << (int)compute.ipAddress[2] <<
									"." << (int)compute.ipAddress[3] <<
								" Mask: " << (int)compute.netMask[0] <<
									"." << (int)compute.netMask[1] <<
									"." << (int)compute.netMask[2] <<
									"." << (int)compute.netMask[3] <<
								", MAC: " << hex << setfill('0') << setw(2) << (int)compute.macAddress[0] <<
									":" << setw(2) << (int)compute.macAddress[1] <<
									":" << setw(2) << (int)compute.macAddress[2] <<
									":" << setw(2) << (int)compute.macAddress[3] <<
									":" << setw(2) << (int)compute.macAddress[4] <<
									":" << setw(2) << (int)compute.macAddress[5]);
	} else {
		LOG4CXX_DEBUG(logger, "No compute adapter found");
	}
	Node::AdapterInfo management = getManagementAdapter(adapters);
	if (management.ipAddress[0] != 0) {
		LOG4CXX_DEBUG(logger, 	"Management adapter: Addr: " << (int)management.ipAddress[0] <<
									"." << (int)management.ipAddress[1] <<
									"." << (int)management.ipAddress[2] <<
									"." << (int)management.ipAddress[3] <<
								" Mask: " << (int)management.netMask[0] <<
									"." << (int)management.netMask[1] <<
									"." << (int)management.netMask[2] <<
									"." << (int)management.netMask[3] <<
								", MAC: " << hex << setfill('0') << setw(2) << (int)management.macAddress[0] <<
									":" << setw(2) << (int)management.macAddress[1] <<
									":" << setw(2) << (int)management.macAddress[2] <<
									":" << setw(2) << (int)management.macAddress[3] <<
									":" << setw(2) << (int)management.macAddress[4] <<
									":" << setw(2) << (int)management.macAddress[5]);
	} else {
		LOG4CXX_DEBUG(logger, "No management adapter found");
	}
}

Node::~Node() {
	delete mSensors;
}

SensorSet* Node::getSensors() {
	return mSensors;
}

string Node::getID() {
	return mID;
}

uint8_t Node::getBaseboardID() {
	return mBaseboardID;
}

uint8_t Node::getSlot() {
	return mSlot;
}

string Node::executeCommand(string command, string parameters) {
	LOG4CXX_DEBUG(logger, "Executing command: '" << command << "', parameters: '" << parameters << "'");

	if (command == "shutdown") {
		return shutdown();
	} else {

	}

	return "Unknown command";
}

string Node::shutdown() {
	LOG4CXX_INFO(logger, "Shutting down system...");
#ifndef WIN32
	int ret = system("shutdown -P now");
	if (ret) {
		LOG4CXX_ERROR(logger, "Could not execute shutdown, maybe missing permission?");
		return "failed";
	}
	// Hardcore version: Directly shut down kernel
	//sync();
	//reboot(RB_POWER_OFF);
	return "success";
#else
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	// Get a token for this process.
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		return ("Could not get process token");
	}

	// Get the LUID for the shutdown privilege.
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

	tkp.PrivilegeCount = 1;  // one privilege to set
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	// Get the shutdown privilege for this process.
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);
	if (GetLastError() != ERROR_SUCCESS) {
		return "Could not get shutdown privilege";
	}

	// Shut down the system and force all applications to close.

	if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE | EWX_POWEROFF, 0)) {
		return "Could not shutdown system";
	}

	//shutdown was successful
	return "success";
#endif
}

size_t Node::getBasicInformationBlock(uint8_t* buffer, size_t bufferSize) {
	if (bufferSize < sizeof(Basic_Information_Block)) return 0;

	Basic_Information_Block* bib = (Basic_Information_Block*)buffer;
	bib->header.type = Basic_Information;
	bib->header.size = htons(sizeof(Basic_Information_Block));
	bib->daemonVersion[0] = DAEMON_VERSION_MAJOR;
	bib->daemonVersion[1] = DAEMON_VERSION_MINOR;
	bib->daemonVersion[2] = DAEMON_VERSION_REVISION;
	list<Node::AdapterInfo> adapters = getNetworkAdapters();
	Node::AdapterInfo management = getManagementAdapter(adapters);
	// Not copying whole structure because of possible padding
	memcpy(bib->management.ip, management.ipAddress, 16);
	memcpy(bib->management.netmask, management.netMask, 4);
	memcpy(bib->management.mac, management.macAddress, 6);
	Node::AdapterInfo compute = getComputeAdapter(adapters);
	memcpy(bib->compute.ip, compute.ipAddress, 16);
	memcpy(bib->compute.netmask, compute.netMask, 4);
	memcpy(bib->compute.mac, compute.macAddress, 6);
	string hostName = getHostName();
	strncpy((char*)&(bib->hostname[0]), hostName.c_str(), HOSTNAME_LENGTH);

	return sizeof(Basic_Information_Block);
}

Node::AdapterInfo Node::getManagementAdapter(list<Node::AdapterInfo> adapters) {
	if (adapters.size() == 1) { // APLS modules with only one NIC are connected to compute
		return Node::AdapterInfo();
	}
	if (adapters.size() >= 2) {
		uint8_t christmannAdapters = 0;
		Node::AdapterInfo nonChristmannAdapter;
		// Iterate through all adapters, checking whether MAC is one of our's
		for (list<Node::AdapterInfo>::iterator iterator = adapters.begin(); iterator != adapters.end(); ++iterator) {
			if (	iterator->macAddress[0] == CHRISTMANN_MAC_1 &&
					iterator->macAddress[1] == CHRISTMANN_MAC_2 &&
					iterator->macAddress[2] == CHRISTMANN_MAC_3 &&
					iterator->macAddress[3] == CHRISTMANN_MAC_4) {
				christmannAdapters++;
			} else {
				nonChristmannAdapter = *iterator;
			}
		}
		if (christmannAdapters == 1) {
			// For CXP modules, the compute adapter is either on baseboard or netboard.
			// Both of them will have a Christmann MAC.
			return nonChristmannAdapter;
		} else if (christmannAdapters > 1) {
			// On Exynos both MACs are Christmann MACs. Even = Compute, Odd = Management
			for (list<Node::AdapterInfo>::iterator iterator = adapters.begin(); iterator != adapters.end(); ++iterator) {
				if (	iterator->macAddress[0] == CHRISTMANN_MAC_1 &&
						iterator->macAddress[1] == CHRISTMANN_MAC_2 &&
						iterator->macAddress[2] == CHRISTMANN_MAC_3 &&
						iterator->macAddress[3] == CHRISTMANN_MAC_4 &&
						(iterator->macAddress[5] & 0x01) == 1) { // Lowest bit 1 -> Odd
					return *iterator;
				}
			}
		} else {
			// No Cristmann adapters at all? Should only happen on non-RECS hardware.
			// Return first adapter
			return adapters.front();
		}
	}
	return Node::AdapterInfo();
}

Node::AdapterInfo Node::getComputeAdapter(list<Node::AdapterInfo> adapters) {
	if (adapters.size() == 1) { // APLS modules with only one NIC are connected to compute
		return adapters.front();
	}
	if (adapters.size() >= 2) {
		uint8_t christmannAdapters = 0;
		Node::AdapterInfo christmannAdapter;
		// Iterate through all adapters, checking whether MAC is one of our's
		for (list<Node::AdapterInfo>::iterator iterator = adapters.begin(); iterator != adapters.end(); ++iterator) {
			if (	iterator->macAddress[0] == CHRISTMANN_MAC_1 &&
					iterator->macAddress[1] == CHRISTMANN_MAC_2 &&
					iterator->macAddress[2] == CHRISTMANN_MAC_3 &&
					iterator->macAddress[3] == CHRISTMANN_MAC_4) {
				christmannAdapter = *iterator;
				christmannAdapters++;
			}
		}
		if (christmannAdapters == 1) {
			// For CXP modules, the compute adapter is either on baseboard or netboard.
			// Both of them will have a Christmann MAC.
			return christmannAdapter;
		} else if (christmannAdapters > 1) {
			// On Exynos both MACs are Christmann MACs. Even = Compute, Odd = Management
			for (list<Node::AdapterInfo>::iterator iterator = adapters.begin(); iterator != adapters.end(); ++iterator) {
				if (	iterator->macAddress[0] == CHRISTMANN_MAC_1 &&
						iterator->macAddress[1] == CHRISTMANN_MAC_2 &&
						iterator->macAddress[2] == CHRISTMANN_MAC_3 &&
						iterator->macAddress[3] == CHRISTMANN_MAC_4 &&
						(iterator->macAddress[5] & 0x01) == 0) { // Lowest bit 0 -> Even
					return *iterator;
				}
			}
		}
	}
	return Node::AdapterInfo();
}

list<Node::AdapterInfo> Node::getNetworkAdapters() {
	list<Node::AdapterInfo> adapters;
#ifndef WIN32
	struct ifreq ifr;
	struct ifconf ifc;
	char buf[1024];

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock == -1) {
		LOG4CXX_ERROR(logger, "Could not create socket");
		return adapters;
	};

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
		LOG4CXX_ERROR(logger, "Could not get list of interface addresses (SIOCGIFCONF)");
		return adapters;
	}

	struct ifreq* it = ifc.ifc_req;
	const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

	for (; it != end; ++it) {
		strcpy(ifr.ifr_name, it->ifr_name);
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
			if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
				Node::AdapterInfo info;
				if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
					memcpy(&info.macAddress, ifr.ifr_hwaddr.sa_data, 6);
				}
                if (ioctl(sock, SIOCGIFADDR, &ifr) == 0) {
			    	memcpy(&info.ipAddress[0], &(((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr.s_addr), 4);
                }
                if (ioctl(sock, SIOCGIFNETMASK, &ifr) == 0) {
			    	memcpy(&info.netMask[0], &(((struct sockaddr_in *)(&ifr.ifr_netmask))->sin_addr.s_addr), 4);
                }
				adapters.push_back(info);
			}
		} else {
			LOG4CXX_ERROR(logger, "Could not get interface flags (SIOCGIFFLAGS)");
		}
	}
#else
    PIP_ADAPTER_INFO AdapterInfo;
    DWORD dwBufLen = sizeof(AdapterInfo);

    AdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
    if (AdapterInfo == NULL) {
    	LOG4CXX_ERROR(logger, "Could not allocate IP_ADAPTER_INFO");
    	return adapters;
    }

	// Make an initial call to GetAdaptersInfo to get the necessary size
	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
		free(AdapterInfo);
		AdapterInfo = (IP_ADAPTER_INFO *)malloc(dwBufLen);
		if (AdapterInfo == NULL) {
			LOG4CXX_ERROR(logger, "Could not allocate IP_ADAPTER_INFO (2nd)");
			return adapters;
		}
	}

	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo; // Contains pointer to current adapter info
		do {
			Node::AdapterInfo info;
			string ip = string(pAdapterInfo->IpAddressList.IpAddress.String);
			if (ip != "0.0.0.0") { // Skip adapters without IP
			    struct sockaddr_in  si4;
			    INT s = sizeof(si4);
			    si4.sin_family = AF_INET;
			    if (!WSAStringToAddress(const_cast<char *>(ip.c_str()), AF_INET, NULL, (LPSOCKADDR) &si4, &s)) {
			    	memcpy(&info.ipAddress[0], &si4.sin_addr, sizeof(si4.sin_addr));
				    string netmask = string(pAdapterInfo->IpAddressList.IpMask.String);
				    INT s = sizeof(si4);
				    si4.sin_family = AF_INET;
				    if (!WSAStringToAddress(const_cast<char *>(netmask.c_str()), AF_INET, NULL, (LPSOCKADDR) &si4, &s)) {
				    	memcpy(&info.netMask[0], &si4.sin_addr, sizeof(si4.sin_addr));
						memcpy(&info.macAddress, pAdapterInfo->Address, 6);
						adapters.push_back(info);
				    }
			    }
			}

			pAdapterInfo = pAdapterInfo->Next;
		} while (pAdapterInfo);
	}
	free(AdapterInfo);
#endif
	return adapters;
}

string Node::getHostName() {
#ifdef WIN32
	char name[256];
	memset(name, 0, 250);
	DWORD bufCharCount = 256;
	if (GetComputerName(name, &bufCharCount)) {
		return string(name);
	} else {
		return "";
	}
#else
	char name[256];
	memset(name, 0, 256);
	gethostname(name, 255);
	return string(name);
#endif
}

string Node::getJSONMonitoringData(Daemon* daemon) {
	if (mMonitoringDataOffset == 0) {
		uint16_t offset = findHeaderDataChunkOffset(daemon, CHUNK_TYPE_MONITORING);
		if (offset > 2) {
			mMonitoringDataOffset = offset - 2; // Because of shared length-field between Chunk_Header and BB_Monitoring
		}
	}
	if (mMonitoringDataOffset == 0) { // Still not found
		LOG4CXX_ERROR(logger, "Monitoring data chunk not found!");
		return "";
	}
	BB_Monitoring mon;
	ssize_t read = daemon->doRead(mMonitoringDataOffset, &mon, sizeof(BB_Monitoring));
	if (read) {
		std::ostringstream bytes;
		/*bytes << setfill('0');
		for (uint8_t i = 0; i < read; ++i) {
			bytes << std::setw(2) << std::hex << (uint16_t)(((char *)&mon)[i] & 0xff) << " ";
		}
		LOG4CXX_DEBUG(logger, "Read " << read << " bytes of monitoring data: " << bytes.str()); */
		std::ostringstream oss;
		oss << "[\r\n";
		oss << "{\"name\": \"nodeCurrent\", \"value\": " << (fromLitteEndian(mon.current[2 + mSlot]) / 1000.0) << ", \"unit\": \"A\"},\r\n";
		if (mNodeType == NODE_CXP) {
			oss << "{\"name\": \"pegCurrent\", \"value\": " << (fromLitteEndian(mon.current[1]) / 1000.0) << ", \"unit\": \"A\"},\r\n";
		}
		oss << "{\"name\": \"12vSupply\", \"value\": " << (fromLitteEndian(mon.voltage[0]) / 1000.0) << ", \"unit\": \"V\"},\r\n";
		oss << "{\"name\": \"temperatures\", \"values\": [";
		for (uint8_t i = 0; i < 5; ++i) {
			if (i > 0) {
				oss << ", ";
			}
			oss << (uint16_t)mon.temperature[i];
		}
		oss << "], \"unit\": \"°C\"}\r\n";
		oss << "]\r\n";
		return oss.str();
	}
	return "";
}

uint16_t Node::fromLitteEndian(uint16_t value) {
    union {
        uint32_t i;
        char c[4];
    } test = {0x01020304};

    if (test.c[0] == 1) {
    	return ((value & 0xff) << 8) | ((value & 0xff00) >> 8);
    }
    return value;
}

uint16_t Node::findHeaderDataChunkOffset(Daemon* daemon, uint8_t chunkType) {
	Daemon_Header hdr;
	if (daemon->doRead(0, &hdr, sizeof(Daemon_Header))) {
		if (hdr.version < 3) {
			if (chunkType == CHUNK_TYPE_MONITORING) {
				return 9; // Old header length
			}
			return 0;
		}

		uint16_t pos = hdr.chunksOffset;
		do {
			Chunk_Header chunkHeader;
			if (!daemon->doRead(pos, &chunkHeader, sizeof(Chunk_Header))) {
				return 0;
			}
			if (chunkHeader.chunkType == chunkType) {
				return pos + sizeof(Chunk_Header);
			}
			pos += sizeof(Chunk_Header) + chunkHeader.length;
		} while (pos < hdr.dynamicAreaOffset);
	}
	return 0;
}
