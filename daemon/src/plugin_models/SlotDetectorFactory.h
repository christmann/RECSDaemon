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

#ifndef SLOTDETECTOR_FACTORY_H
#define SLOTDETECTOR_FACTORY_H

#include "../plugin_framework/PluginManager.h"
#include "../plugin_framework/ObjectAdapter.h"
#include "SlotDetectorAdapter.h"

struct SlotDetectorFactory: public ObjectAdapter<SlotDetectorAdapter, C_SlotDetector> {
	static SlotDetectorFactory & getInstance() {
		static SlotDetectorFactory instance;

		return instance;
	}

	static ISlotDetector * createSlotDetector(const std::string & objectType) {
		void * SlotDetector = PluginManager::getInstance().createObject(objectType, getInstance());
		return static_cast<ISlotDetector*>(SlotDetector);
	}
};

#endif // SLOTDETECTOR_FACTORY_H
