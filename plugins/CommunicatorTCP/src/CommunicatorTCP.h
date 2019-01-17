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

#ifndef COMMUNICATORTCP_H
#define COMMUNICATORTCP_H

#include <object_model.h>
#include <IConfig.h>
#include <string>
#include <logger.h>
#include "NetworkClient.h"

typedef void* (*PF_InvokeServiceFunc)(const uint8_t * serviceName, void * serviceParams);

struct PF_ObjectParams;

class CommunicatorTCP: public ICommunicator {
public:

	// static plugin interface
	static void * create(PF_ObjectParams *);
	static int32_t destroy(void *);
	~CommunicatorTCP();

	// ICommunicator methods
	virtual bool initInterface(void);
	virtual size_t getMaxDataSize(void);
	virtual ssize_t readData(size_t offset, void* buf, size_t count);
	virtual ssize_t writeData(size_t offset, const void* buf, size_t count);

	static LoggerPtr logger;
	static IConfig* config;
	static PF_InvokeServiceFunc invokeService;

private:
	CommunicatorTCP();
	bool tryConnect();

	int8_t mBaseboardID;
	NetworkClient* mClient;
	uint8_t* mData;
	uint8_t* mReceiveBuffer;
	size_t mReceiveBufferPos;


	typedef struct __attribute__((__packed__)) {
		uint8_t		magic[4];
		uint8_t		baseboard;
		uint8_t		node;
		uint16_t	size;
	} TCP_Message_Header;
};

#endif
