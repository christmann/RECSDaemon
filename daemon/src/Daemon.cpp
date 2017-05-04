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
// Created on: 11.12.2015
////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <iostream>
#include <cstring>
#include <vector>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#ifdef WIN32
#include <Windows.h>
#endif

#include <log4cxx/xml/domconfigurator.h>

#include "plugin_framework/Directory.h"
#include "plugin_framework/Path.h"
#include "plugin_framework/PluginManager.h"
#include "../include/object_model.h"
#include "../include/daemon_msgs.h"
#include "plugin_models/CommunicatorFactory.h"
#include "plugin_models/SlotDetectorFactory.h"

#include "network/TelnetServer.h"

#include "Signature.h"

#include "version.h"
#include "Config.h"
#include "Node.h"
#include "Daemon.h"

using namespace std;
using namespace log4cxx;
using namespace log4cxx::xml;

#define MAX_UPDATEINTERVAL	30000 // ms

LoggerPtr Daemon::logger(Logger::getLogger("Daemon"));
Daemon* Daemon::instance;

Daemon::Daemon() :
	mPluginLoggers(new list<LoggerPtr>()),
	mShutdown(false),
	mFirstWriteDone(false),
	mState(State_BasicInformation),
	mCurrentPage(1),
	mComm(NULL),
	mSlot(0) {
	instance = this;
	pthread_mutex_init(&mCommMutex, NULL);
}

Daemon::~Daemon() {
	pthread_mutex_destroy(&mCommMutex);
}

void Daemon::resetStatemachine(void) {
	mState = State_BasicInformation;
	mCurrentPage = 1;
	mFirstWriteDone = false;
}

int8_t Daemon::getSlot() {
	return mSlot;
}

void* Daemon::InvokeService(const uint8_t * serviceName, void * serviceParams) {
	if (strcmp((char*)serviceName, "getLogger") == 0) {
		return instance->addPluginLogger(string((char*)serviceParams));
		// 	return (void*)&(*log);
	} else if (strcmp((char*)serviceName, "getConfig") == 0) {
		return (void*)Config::GetInstance();
	} else if (strcmp((char*)serviceName, "resetStatemachine") == 0) {
		LOG4CXX_INFO(logger, "Resetting state machine");
		instance->resetStatemachine();
	} else if (strcmp((char*)serviceName, "getSlot") == 0) {
		if (serviceParams != NULL) {
			*((int8_t*)serviceParams) = instance->getSlot();
		}
	}
	return 0;
}

void Daemon::signal_handler(int sig) {
	switch (sig) {
#ifndef WIN32
		case SIGHUP:
			LOG4CXX_INFO(logger, "HangUp signal catched");
			break;
#endif
		case SIGINT:
		case SIGTERM:
			LOG4CXX_INFO(logger, "Terminate signal catched, shutting down...");
			instance->shutdown();
			break;
	}
}

int Daemon::run(int exitAfter) {
	LOG4CXX_INFO(logger, "Loading config file");
	Config::GetInstance();

	// Initialization
	PluginManager & pm = PluginManager::getInstance();
	pm.getPlatformServices().invokeService = InvokeService;
	LOG4CXX_INFO(logger, "Loading plugins...");
	if (pm.loadAll(Path::makeAbsolute(Directory::getCWD() + "/plugins"), InvokeService) < 0) {
		LOG4CXX_ERROR(logger, "Could not open plugins directory!");
	}
	LOG4CXX_INFO(logger, pm.getRegistrationMap().size() << " plugins loaded");

	string communicatorPluginName = Config::GetInstance()->GetString("Comm", "PluginName", "");
	if (communicatorPluginName == "") {
		LOG4CXX_ERROR(logger, "No communicator plugin selected. Please set entry PluginName in section Comm in " << CONFIG_FILE << "!");
		pm.shutdown();

		mPluginLoggers->clear();
		delete mPluginLoggers;

		Config::GetInstance()->shutdown();

		return -1;
	}

	LOG4CXX_DEBUG(logger, "Creating instance of plugin " << communicatorPluginName);
	mComm = CommunicatorFactory::createCommunicator(communicatorPluginName);

	if (mComm == NULL) {
		LOG4CXX_ERROR(logger, "Could not instantiate plugin " << communicatorPluginName << "!");
		pm.shutdown();

		mPluginLoggers->clear();
		delete mPluginLoggers;

		Config::GetInstance()->shutdown();

		return -1;
	}

	if (!mComm->initInterface()) {
		LOG4CXX_ERROR(logger, "Could not initialize communicator interface!");

		delete mComm;
		pm.shutdown();

		mPluginLoggers->clear();
		delete mPluginLoggers;

		Config::GetInstance()->shutdown();

		return -1;
	}

	ssize_t size = mComm->getMaxDataSize();
	LOG4CXX_INFO(logger, "Maximum data size is " << size << " bytes");

	Daemon_Header hdr;
	if (mComm->readData(0, &hdr, sizeof(Daemon_Header))) {
		if (hdr.magic[0] == 'R' && hdr.magic[1] == 'E' && hdr.magic[2] == 'C' && hdr.magic[3] == 'S') {
			if (hdr.baseboardID & 0x80) {
				LOG4CXX_INFO(logger, "Running on baseboard with I2C address 0x" << hex << (hdr.baseboardID & 0x7f));
			} else {
				LOG4CXX_INFO(logger, "Running on baseboard with ID " << (int)hdr.baseboardID);
			}
			if (hdr.maxSlots > 0) {
				LOG4CXX_INFO(logger, "Number of slots on baseboard: " << (int)hdr.maxSlots);
			} else {
				LOG4CXX_ERROR(logger, "Number of slots (0) invalid, try updating firmware");
				delete mComm;
				pm.shutdown();

				mPluginLoggers->clear();
				delete mPluginLoggers;

				Config::GetInstance()->shutdown();

				return -1;
			}
		} else {
			LOG4CXX_ERROR(logger, "No valid header signature found");
			delete mComm;
			pm.shutdown();

			mPluginLoggers->clear();
			delete mPluginLoggers;

			Config::GetInstance()->shutdown();

			return -1;
		}
	} else {
		LOG4CXX_ERROR(logger, "Could not read memory header");
		delete mComm;
		pm.shutdown();

		mPluginLoggers->clear();
		delete mPluginLoggers;

		Config::GetInstance()->shutdown();

		return -1;
	}

	mSlot = Config::GetInstance()->GetInt("Slot", "defaultSlot", 0);
	string slotDetectorPluginName = Config::GetInstance()->GetString("Slot", "slotPluginName", "");
	if (slotDetectorPluginName != "") {
		LOG4CXX_DEBUG(logger, "Creating instance of plugin " << slotDetectorPluginName);
		ISlotDetector * slotDetector = SlotDetectorFactory::createSlotDetector(slotDetectorPluginName);

		if (slotDetector == NULL) {
			LOG4CXX_ERROR(logger, "Could not instantiate plugin " << slotDetectorPluginName << "!");
			delete mComm;
			pm.shutdown();

			mPluginLoggers->clear();
			delete mPluginLoggers;

			Config::GetInstance()->shutdown();

			return -1;
		}

		mSlot = slotDetector->getSlot();
		if (mSlot < 0) {
			LOG4CXX_ERROR(logger, "Could not determine module slot!");

			delete slotDetector;
			delete mComm;
			pm.shutdown();

			mPluginLoggers->clear();
			delete mPluginLoggers;

			Config::GetInstance()->shutdown();

			return -1;
		}
		delete slotDetector;

		LOG4CXX_INFO(logger, "Detected module slot " << (int)mSlot);
	} else {
		LOG4CXX_INFO(logger, "No SlotDetector plugin selected (entry slotPluginName in section Slot), assuming slot " << (int)mSlot);
	}

	uint8_t baseboardID = 0;
	string nodeID = "";
	if (!(hdr.baseboardID & 0x80)) {
		baseboardID = hdr.baseboardID;
		stringstream nodeIDStream;
		nodeIDStream << (int)hdr.baseboardID;
		if (hdr.maxSlots > 1) {
			nodeIDStream << "-" << (mSlot + 1);
		}
		nodeID = nodeIDStream.str();
		LOG4CXX_INFO(logger, "ID of this node is " << nodeID);
	} else {
		LOG4CXX_INFO(logger, "Could not determine node ID, baseboard ID not set");
	}

	size_t messageOffset = hdr.dynamicAreaOffset;
	if (hdr.version < 3) {
		messageOffset = 9 + 57; // Old result of sizeof(Daemon_Header) + sizeof(BB_Monitoring);
	}

	size_t messageMaxSize = hdr.size - (messageOffset);
	if (hdr.maxSlots > 1) {
		// Determine my memory segment
		messageMaxSize /= hdr.maxSlots;
		messageOffset += (mSlot * messageMaxSize);
	}
	LOG4CXX_INFO(logger, "Reserved memory for messages at 0x" << hex << messageOffset << ", " << dec << messageMaxSize << " bytes");

	if (messageMaxSize < sizeof(Message_Header)) {
		LOG4CXX_ERROR(logger, "Maximum memory size too small for header");

		delete mComm;
		pm.shutdown();

		mPluginLoggers->clear();

		Config::GetInstance()->shutdown();

		delete mPluginLoggers;
		return -1;
	}

	LOG4CXX_INFO(logger, "Initializing sensors...");
	Node::NodeType type = hdr.maxSlots == 4 ? Node::NODE_APALIS : Node::NODE_CXP;
	Node* node = new Node(nodeID, baseboardID, mSlot, type);

	size_t sensorSize = node->getSensors()->getSize();
	if (sensorSize <= messageMaxSize) {
		LOG4CXX_INFO(logger, "Sensor message will use " << sensorSize << " of " << messageMaxSize << " bytes available");
	} else {
		LOG4CXX_ERROR(logger, "Sensor message size of " << sensorSize << " bytes too big for allocated memory!");

		delete node;
		delete mComm;
		pm.shutdown();

		mPluginLoggers->clear();
		delete mPluginLoggers;

		Config::GetInstance()->shutdown();

		return -1;
	}

	LOG4CXX_INFO(logger, "Starting server...");
	CommandLineServer* server = new CommandLineServer(node, this);

	LOG4CXX_INFO(logger, "Initializing crypto...");
	Signature* signature = new Signature();

	LOG4CXX_INFO(logger, "READY.");

#ifndef WIN32
	signal(SIGCHLD, SIG_IGN); // ignore child
	signal(SIGTSTP, SIG_IGN); // ignore tty signals
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGHUP, signal_handler); // catch hangup signal
#endif
	signal(SIGINT, signal_handler); // catch kill signal
	signal(SIGTERM, signal_handler); // catch kill signal
	mShutdown = false;
	resetStatemachine();
	int updateInterval = Config::GetInstance()->GetInt("Update", "updateInterval", 1000);
	if (updateInterval > MAX_UPDATEINTERVAL) {
		LOG4CXX_WARN(logger, "Update interval too large, maximum interval is " << MAX_UPDATEINTERVAL << " ms");
		updateInterval = MAX_UPDATEINTERVAL;
	}
	timeval lastLoopTime;
	gettimeofday(&lastLoopTime, 0);
	while (!mShutdown) {
		// Read message header
		Message_Header msg;
		pthread_mutex_lock(&mCommMutex);
		ssize_t read = mComm->readData(messageOffset, &msg, sizeof(Message_Header));
		pthread_mutex_unlock(&mCommMutex);
		if (read) {
			enum Message_Type type = static_cast<Message_Type>(msg.type);
			uint16_t msgSize = ntohs(msg.size);
			//LOG4CXX_DEBUG(logger, "Current message size=" << msg.size << " bytes, type=" << type);
			if (msgSize <= messageMaxSize) {
				if (type == Command) {
					// Handle command
					uint8_t* data = (uint8_t*)malloc(msgSize);
					if (data != NULL) {
						pthread_mutex_lock(&mCommMutex);
						read = mComm->readData(messageOffset + sizeof(Message_Header), data + sizeof(Message_Header), msgSize - sizeof(Message_Header));
						pthread_mutex_unlock(&mCommMutex);
						if (read) {
							// Copy together complete message for signature check to pass
							memcpy(data, &msg, sizeof(Message_Header));
							Command_Header* header = (Command_Header*)data;
							if (header->baseboard == node->getBaseboardID() && header->slot == node->getSlot()) {
								// Save signature and set to 0 in message
								uint8_t cmdSignature[SIGNATURE_LENGTH];
								memcpy(&cmdSignature[0], header->signature, SIGNATURE_LENGTH);
								memset(header->signature, 0, SIGNATURE_LENGTH);
								if (signature->checkSignature(data, msgSize, &cmdSignature[0], sizeof(cmdSignature))) {
									LOG4CXX_DEBUG(logger, "Signature successfully verified");
									//TODO: Optionally check timestamp +- given time frame
									size_t commandLen = min((size_t)COMMAND_MAX_LENGTH, strlen((char *)header->command));
									size_t parametersLen = min((size_t)ntohs(header->parametersLength), min((size_t)msgSize, messageMaxSize));
									string command((char*)(header->command), commandLen);
									string parameters((char*)(data + sizeof(Command_Header)), parametersLen);
									node->executeCommand(command, parameters);
								} else {
									LOG4CXX_ERROR(logger, "Signature verification failed");
								}
							} else {
								LOG4CXX_ERROR(logger, "Received command was for slot " << header->slot << " on baseboard " << header->baseboard << ", ignoring!");
							}

							// Clear command message
							uint8_t type = 0;
							pthread_mutex_lock(&mCommMutex);
							mComm->writeData(messageOffset, &type, 1);
							pthread_mutex_unlock(&mCommMutex);
						} else {
							LOG4CXX_ERROR(logger, "Could not read rest of command message");
						}
					} else {
						LOG4CXX_ERROR(logger, "Could not allocate memory for rest of command message");
					}
				} else if ((type == Monitoring_Description || type == Basic_Information || type == Command_Result) && mFirstWriteDone) {
					LOG4CXX_DEBUG(logger, "Waiting for management to pick up message...");
					// Reply not yet read by management, keep data
				} else {
					if (mState == State_BasicInformation) {
						uint8_t* desc = (uint8_t*)malloc(messageMaxSize);
						size_t size = node->getBasicInformationBlock(desc, messageMaxSize);
						LOG4CXX_DEBUG(logger, "Writing basic information block (" << size << " bytes)");
						pthread_mutex_lock(&mCommMutex);
						mComm->writeData(messageOffset, desc, size);
						pthread_mutex_unlock(&mCommMutex);
						free(desc);
						mState = State_MonitoringDescription;
					} else if (mState == State_MonitoringData) {
						//LOG4CXX_DEBUG(logger, "Writing sensor data (" << sensorSize << " bytes)");
						pthread_mutex_lock(&mCommMutex);
						mComm->writeData(messageOffset, node->getSensors()->getMessage(), sensorSize);
						pthread_mutex_unlock(&mCommMutex);
					} else if (mState == State_MonitoringDescription) {
						uint8_t* desc = (uint8_t*)malloc(messageMaxSize);
						uint8_t maxPages = 0;
						size_t size = node->getSensors()->getDescriptionPage(desc, messageMaxSize, mCurrentPage, &maxPages);
						LOG4CXX_DEBUG(logger, "Writing description page " << (int)mCurrentPage << " of " << (int)maxPages << " (" << size << " bytes)");
						pthread_mutex_lock(&mCommMutex);
						mComm->writeData(messageOffset, desc, size);
						pthread_mutex_unlock(&mCommMutex);
						free(desc);

						mCurrentPage++;
						if (mCurrentPage > maxPages) {
							mCurrentPage = 1;
							mState = State_MonitoringData;
						}
					}
					mFirstWriteDone = true;
				}
			} else {
				LOG4CXX_ERROR(logger, "Given message size (" << msgSize << ") too large");
			}
		} else {
			LOG4CXX_WARN(logger, "Could not read message header");
		}

		// Wait for next update
		timeval endTime;
		int loopTime = 0;
		for (int loops = 0; loops < ((updateInterval / 100) * 2); ++loops) {
			gettimeofday(&endTime, 0);

			long seconds = endTime.tv_sec - lastLoopTime.tv_sec;
			long nseconds = endTime.tv_usec - lastLoopTime.tv_usec;
			loopTime = ((seconds * 1000) + (nseconds / 1000));
			if (loopTime < updateInterval) {
#ifdef WIN32
				Sleep(100);
#else
				usleep(100 * 1000);
#endif
			} else {
				break;
			}
		}
		lastLoopTime = endTime;

		if (exitAfter > 0) {
			exitAfter--;
			if (exitAfter == 0) {
				mShutdown = true;
			}
		}
	}

	LOG4CXX_INFO(logger, "RECS daemon quitting, writing empty sensor description page");
	node->getSensors()->clear();
	uint8_t* desc = (uint8_t*)malloc(messageMaxSize);
	uint8_t maxPages = 0;
	size = node->getSensors()->getDescriptionPage(desc, messageMaxSize, 1, &maxPages);
	pthread_mutex_lock(&mCommMutex);
	mComm->writeData(messageOffset, desc, size);
	pthread_mutex_unlock(&mCommMutex);
	free(desc);

	LOG4CXX_INFO(logger, "Shutting down services");
	delete signature;
	delete server;
	delete node;
	delete mComm;
	pm.shutdown();

	mPluginLoggers->clear();
	delete mPluginLoggers;

	Config::GetInstance()->shutdown();

	return 0;
}

LoggerPtr Daemon::addPluginLogger(string name) {
	LoggerPtr log = Logger::getLogger(name);
	mPluginLoggers->push_back(log);
	return log;
}

ssize_t Daemon::doRead(size_t offset, void* buf, size_t count) {
	ssize_t read = -1;
	pthread_mutex_lock(&mCommMutex);
	if (mComm != NULL) {
		read = mComm->readData(offset, buf, count);
	}
	pthread_mutex_unlock(&mCommMutex);
	return read;
}

void Daemon::shutdown() {
	mShutdown = true;
}
