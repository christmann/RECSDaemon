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

#include "LinuxSensorIP.h"

#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include "daemon_msgs.h"

using namespace std;
using namespace log4cxx;

LoggerPtr LinuxSensorIP::logger;

void * LinuxSensorIP::create(PF_ObjectParams *) {
	return new LinuxSensorIP();
}

int32_t LinuxSensorIP::destroy(void * p) {
	if (!p)
		return -1;
	delete static_cast<LinuxSensorIP*>(p);
	return 0;
}

LinuxSensorIP::LinuxSensorIP() {
}

LinuxSensorIP::~LinuxSensorIP() {
}

bool LinuxSensorIP::configure(const char* data) {
	BaseSensor::configure(data);

    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            //printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
            mIPs += string(addressBuffer) + ", ";
        } else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            //printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
            mIPs += string(addressBuffer) + ", ";
        }
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);

    if (mIPs.length() > 0) {
    	mIPs = mIPs.substr(0, mIPs.length() - 2);
    }

	return true;
}

ISensorDataType LinuxSensorIP::getDataType(void) {
	return TYPE_STR;
}

size_t LinuxSensorIP::getMaxDataSize(void) {
	return mIPs.length();
}

bool LinuxSensorIP::getData(uint8_t* data) {
	memcpy(data, mIPs.c_str(), mIPs.length());
	return true;
}

const char* LinuxSensorIP::getDescription(void) {
	return "Returns IP addresses of NICs";
}

ISensorUnit LinuxSensorIP::getUnit(void) {
	return UNIT_DIMENSIONLESS;
}

log4cxx::LoggerPtr LinuxSensorIP::getLogger(void) {
	return logger;
}
