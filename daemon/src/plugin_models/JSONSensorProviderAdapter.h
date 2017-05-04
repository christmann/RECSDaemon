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

#ifndef JSONSensorProvider_ADAPTER_H
#define JSONSensorProvider_ADAPTER_H

//----------------------------------------------------------------------

#include "include/object_model.h"

//----------------------------------------------------------------------

class JSONSensorProviderAdapter: public IJSONSensorProvider {
public:
	JSONSensorProviderAdapter(C_JSONSensorProvider * JSONSensorProvider, PF_DestroyFunc destroyFunc) :
		JSONSensorProvider_(JSONSensorProvider), destroyFunc_(destroyFunc) {
	}

	~JSONSensorProviderAdapter() {
		if (destroyFunc_)
			destroyFunc_(JSONSensorProvider_);
	}

	// IJSONSensorProvider implementation
	const char* getSensorsDescription(void) {
		return JSONSensorProvider_->getSensorsDescription(JSONSensorProvider_->handle);
	}

	const char* getSensorsData(void) {
		return JSONSensorProvider_->getSensorsData(JSONSensorProvider_->handle);
	}

private:
	C_JSONSensorProvider * JSONSensorProvider_;
	PF_DestroyFunc destroyFunc_;
};

#endif // JSONSensorProvider_ADAPTER_H
