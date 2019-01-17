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

#include <cstddef>
#include "Communicator.h"

#include <logger.h>

#ifdef WIN32
#define PLUGIN_API __declspec(dllexport)
#endif
#include "plugin.h"

extern "C" PLUGIN_API int32_t ExitFunc() {
	return 0;
}

extern "C" PLUGIN_API PF_ExitFunc PF_initPlugin(const PF_PlatformServices * params) {
	int res = 0;

	PF_RegisterParams rp;
	rp.version.major = 1;
	rp.version.minor = 0;
	rp.programmingLanguage = PF_ProgrammingLanguage_CPP;

	// Register
	rp.createFunc = Communicator::create;
	rp.destroyFunc = Communicator::destroy;
	res = params->registerObject((const uint8_t *)"WindowsCommunicatorI2CTinyUSB", &rp);
	if (res < 0) {
		return NULL;
	}
	Communicator::logger = *((LoggerPtr*)params->invokeService((const uint8_t *)"getLogger", (void*)"WindowsCommunicatorI2CTinyUSB"));

	return ExitFunc;
}

