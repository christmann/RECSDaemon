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
// Created on: 29.10.2014
////////////////////////////////////////////////////////////////////////////////

#ifndef TELNETSERVER_H_
#define TELNETSERVER_H_

#include <iostream>
#include <log4cxx/logger.h>
#include "../Thread.h"
#include "Network.h"
#include "../Daemon.h"
#include "../Node.h"

class CommandLineServer : public Thread {
public:
	CommandLineServer(Node* node, Daemon* daemon);
	virtual ~CommandLineServer();

protected:
	static log4cxx::LoggerPtr logger;

	Node* getNode();
	Daemon* getDaemon();

private:
	class CommandLineServerClient : public Thread {
	public:
		CommandLineServerClient(CommandLineServer* server, Network* client);
		virtual ~CommandLineServerClient();

	private:
		//lint -e(1704)
		CommandLineServerClient(const CommandLineServerClient& cSource);
		CommandLineServerClient& operator=(const CommandLineServerClient& cSource);

		void execute(void* arg);
		ssize_t readline(void* buffer, size_t len);

		CommandLineServer* mServer;
		Network* mClient;
	};

	//lint -e(1704)
	CommandLineServer(const CommandLineServer& cSource);
	CommandLineServer& operator=(const CommandLineServer& cSource);

	void execute(void* arg);

	Network* mNetwork;
	Node* mNode;
	Daemon* mDaemon;
};

#endif /* TELNETSERVER_H_ */
