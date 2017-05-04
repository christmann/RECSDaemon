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

#ifndef COMMUNICATOR_ADAPTER_H
#define COMMUNICATOR_ADAPTER_H

//----------------------------------------------------------------------

#include "include/object_model.h"

//----------------------------------------------------------------------

class CommunicatorAdapter: public ICommunicator {
public:
	CommunicatorAdapter(C_Communicator * communicator, PF_DestroyFunc destroyFunc) :
			communicator_(communicator), destroyFunc_(destroyFunc) {
	}

	~CommunicatorAdapter() {
		if (destroyFunc_)
			destroyFunc_(communicator_);
	}

	// ICommunicator implememntation
	bool initInterface(void) {
		return communicator_->initInterface(communicator_->handle);
	}

	size_t getMaxDataSize(void) {
		return communicator_->getMaxDataSize(communicator_->handle);
	}

	ssize_t readData(size_t offset, void* buf, size_t count) {
		return communicator_->readData(communicator_->handle, offset, buf, count);
	}

	ssize_t writeData(size_t offset, const void* buf, size_t count) {
		return communicator_->writeData(communicator_->handle, offset, buf, count);
	}

private:
	C_Communicator * communicator_;
	PF_DestroyFunc destroyFunc_;
};

#endif // COMMUNICATOR_ADAPTER_H
