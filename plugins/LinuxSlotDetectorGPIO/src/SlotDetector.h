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

#ifndef SLOTDETECTOR_H
#define SLOTDETECTOR_H

#include <object_model.h>
#include <IConfig.h>
#include <string>
#include <log4cxx/logger.h>

struct PF_ObjectParams;

class SlotDetector: public ISlotDetector {
public:

	// static plugin interface
	static void * create(PF_ObjectParams *);
	static int32_t destroy(void *);
	~SlotDetector();

	// ISlotDetector methods
	virtual int8_t getSlot(void);

	static log4cxx::LoggerPtr logger;
	static IConfig* config;

private:
	SlotDetector();
	int exportGPIO(int pinNr);
	int setDirection(int pinNr, int direction);
	FILE* openGPIO(int pinNr);
	int8_t getGPIO(FILE* fp);
};

#endif
