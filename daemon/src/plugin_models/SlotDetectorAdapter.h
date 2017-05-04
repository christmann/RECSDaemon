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

#ifndef SLOTDETECTOR_ADAPTER_H
#define SLOTDETECTOR_ADAPTER_H

//----------------------------------------------------------------------

#include "include/object_model.h"

//----------------------------------------------------------------------

class SlotDetectorAdapter: public ISlotDetector {
public:
	SlotDetectorAdapter(C_SlotDetector * SlotDetector, PF_DestroyFunc destroyFunc) :
			SlotDetector_(SlotDetector), destroyFunc_(destroyFunc) {
	}

	~SlotDetectorAdapter() {
		if (destroyFunc_)
			destroyFunc_(SlotDetector_);
	}

	// ISlotDetector implememntation
	int8_t getSlot(void) {
		return SlotDetector_->getSlot(SlotDetector_->handle);
	}

private:
	C_SlotDetector * SlotDetector_;
	PF_DestroyFunc destroyFunc_;
};

#endif // SLOTDETECTOR_ADAPTER_H
