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

#ifndef SensorProvider_ADAPTER_H
#define SensorProvider_ADAPTER_H

//----------------------------------------------------------------------

#include "include/object_model.h"
#include <map>

//----------------------------------------------------------------------

class SensorProviderAdapter: public ISensorProvider {
public:
	SensorProviderAdapter(C_SensorProvider * SensorProvider, PF_DestroyFunc destroyFunc) :
		SensorProvider_(SensorProvider), destroyFunc_(destroyFunc) {
	}

	~SensorProviderAdapter() {
		if (destroyFunc_)
			destroyFunc_(SensorProvider_);
	}

	// ISensorProvider implementation
	std::map<std::string, ISensor*> getSensors(void) {
		uint32_t sensorsCnt = SensorProvider_->getSensorsCount(SensorProvider_->handle);
		std::map<std::string, ISensor*> sensors;
		for (uint32_t i = 0; i < sensorsCnt; ++i) {
			const char* name = SensorProvider_->getSensorName(SensorProvider_->handle, i);
			void* sensor = SensorFactory::getInstance().adapt(SensorProvider_->getSensor(SensorProvider_->handle, i), NULL);
			sensors[string(name)] = static_cast<ISensor*>(sensor);
		}
		return sensors;
	}

private:
	C_SensorProvider * SensorProvider_;
	PF_DestroyFunc destroyFunc_;
};

#endif // SensorProvider_ADAPTER_H
