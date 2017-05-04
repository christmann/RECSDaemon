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

#include <algorithm>
#include <string.h>
#include "TelnetServer.h"
#include "../Config.h"
#include "../StaticJSONSensorProvider.h"

#define UNUSED(x) (void)(x)

#define BUFFER_LENGTH		1024
#define CONNECTION_TIME_OUT	10 // s

using namespace log4cxx;

LoggerPtr CommandLineServer::logger(Logger::getLogger("TelnetServer"));

CommandLineServer::CommandLineServer(Node* node, Daemon* daemon)
	: mNode(node), mDaemon(daemon) {
	int port = Config::GetInstance()->GetInt("Telnet", "port", 2023);
	LOG4CXX_DEBUG(logger, "Listening on port " << port);
	mNetwork = new Network(port);
	this->start(this);
}

CommandLineServer::~CommandLineServer() {
	mNetwork->closeSocket();
	this->stop();
	delete mNetwork;
}

void CommandLineServer::execute(void* arg) {
	UNUSED(arg);
	// create a child for every client that connects
	while (IsRunning()) {
		Network* client = mNetwork->acceptConnection();
		if (client != NULL) {
			new CommandLineServerClient(this, client);
		}
	}
}

Node* CommandLineServer::getNode() {
	return mNode;
}

Daemon* CommandLineServer::getDaemon() {
	return mDaemon;
}

CommandLineServer::CommandLineServerClient::CommandLineServerClient(CommandLineServer* server, Network* client) : mServer(server), mClient(client) {
	this->start(this);
}

CommandLineServer::CommandLineServerClient::~CommandLineServerClient() {
	this->stop();
	mServer = NULL;
}

void CommandLineServer::CommandLineServerClient::execute(void* arg) {
	UNUSED(arg);
	LOG4CXX_DEBUG(CommandLineServer::logger, "Client handler started for client " << Network::printIP(mClient->getRemoteIP()) << ":" << mClient->getRemotePort());

	char cmdBuffer[BUFFER_LENGTH] = { 0 };
	while (true) {
		if (readline(cmdBuffer, sizeof(cmdBuffer) - 1) == -1) {
				break;
		}
		string cmd(cmdBuffer);
		transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
		//LOG4CXX_DEBUG(CommandLineServer::logger, "Received new command: '" << cmd << "'");

		if (cmd == "getnodeid") {
			mClient->sendData(mServer->getNode()->getID());
		} else if (cmd.substr(0, 7) == "monitor") {
			mClient->sendData(mServer->getNode()->getJSONMonitoringData(mServer->getDaemon()));
		} else if (cmd.substr(0, 11) == "addsensors ") {
			size_t firstSpace = cmd.find(" ");
			size_t secondSpace = cmd.find(" ", firstSpace + 1);
			if (firstSpace == string::npos || secondSpace == string::npos) {
				mClient->sendData("Invalid parameters, expected addsensors <group name> <JSON data>\n");
				continue;
			}
			string name = cmd.substr(firstSpace + 1, secondSpace - firstSpace - 1);
			string description = cmd.substr(secondSpace + 1);
			IJSONSensorProvider* provider = new StaticJSONSensorProvider(description);
			if (mServer->getNode()->getSensors()->addJSONSensorProvider(provider, name)) {
				mServer->getDaemon()->resetStatemachine();
			} else {
				delete provider;
				mClient->sendData("Could not add sensors group '" + name + "', group already exists!\n");
			}
		} else if (cmd.substr(0, 14) == "updatesensors ") {
			size_t firstSpace = cmd.find(" ");
			size_t secondSpace = cmd.find(" ", firstSpace + 1);
			if (firstSpace == string::npos || secondSpace == string::npos) {
				mClient->sendData("Invalid parameters, expected updatesensors <group name> <JSON data>\n");
				continue;
			}
			string name = cmd.substr(firstSpace + 1, secondSpace - firstSpace - 1);
			string data = cmd.substr(secondSpace + 1);
			IJSONSensorProvider* provider = mServer->getNode()->getSensors()->getJSONSensorProvider(name);
			if (provider == NULL) {
				mClient->sendData("Could not update sensors group '" + name + "', group does not exist!\n");
			} else {
				if(StaticJSONSensorProvider* v = dynamic_cast<StaticJSONSensorProvider*>(provider)) {
					v->updateSensorsData(data);
				} else {
					mClient->sendData("Could not update sensors group '" + name + "', group was not added via this interface!\n");
				}
			}
		} else if (cmd == "exit") {
			mClient->sendData("Closing connection\n");
			break;
		} else {
			mClient->sendData("Unknown command\n");
		}

	}
	LOG4CXX_DEBUG(CommandLineServer::logger, "Client handler exited for client " << Network::printIP(mClient->getRemoteIP()) << ":" << mClient->getRemotePort());
	delete mClient;
}

ssize_t CommandLineServer::CommandLineServerClient::readline(void* buffer, size_t len) {
	char c = '\0', *out = (char*) buffer;
	size_t i;

	memset(buffer, 0, len);

	for (i = 0; i < len && c != '\n'; ++i) {
		if (mClient->receiveData(&c, (size_t)1) <= 0) {
			// timeout or error occured
			return -1;
		}
		if (c == '\r' || c == '\n') {
			i--;
		} else {
			*out++ = c;
		}
	}
	*out = '\0';

	return (ssize_t)i;
}
