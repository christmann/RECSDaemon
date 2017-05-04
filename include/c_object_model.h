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

#ifndef C_OBJECT_MODEL
#define C_OBJECT_MODEL

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct C_CommunicatorHandle_ {
	char c;
}* C_CommunicatorHandle;

typedef struct C_Communicator_ {
	bool (*initInterface)(C_CommunicatorHandle handle);
	size_t (*getMaxDataSize)(C_CommunicatorHandle handle);
	ssize_t (*readData)(C_CommunicatorHandle handle, size_t offset, const void* buf, size_t count);
	ssize_t (*writeData)(C_CommunicatorHandle handle, size_t offset, const void* buf, size_t count);

	C_CommunicatorHandle handle;
} C_Communicator;

typedef struct C_SlotDetectorHandle_ {
	char c;
}* C_SlotDetectorHandle;

typedef struct C_SlotDetector_ {
	int8_t (*getSlot)(C_SlotDetectorHandle handle);

	C_SlotDetectorHandle handle;
} C_SlotDetector;

typedef struct C_SensorHandle_ {
	char c;
}* C_SensorHandle;

enum ISensorDataType {
	TYPE_NONE = 0,
	TYPE_U8 = 1,
	TYPE_U16 = 2,
	TYPE_U32 = 3,
	TYPE_S8 = 4,
	TYPE_S16 = 5,
	TYPE_S32 = 6,
	TYPE_BOOL = 7,
	TYPE_FLOAT = 8,
	TYPE_STR = 9,
	TYPE_BUF = 10,
	TYPE_LST = 11,
	TYPE_U64 = 12
};

enum ISensorUnit {
	UNIT_POWER,
	UNIT_CURRENT,
	UNIT_VOLTAGE,
	UNIT_TEMPERATURE,
	UNIT_ROTATIONAL_SPEED,
	UNIT_DIMENSIONLESS,
	UNIT_BYTE_SECOND,
	UNIT_BYTE,
	UNIT_PERCENT
};

enum IRenderingType {
	RENDERING_TEXTUAL
};

typedef struct C_Sensor_ {
	bool (*configure)(C_SensorHandle handle, const char* options);
	ISensorDataType (*getDataType)(C_SensorHandle handle);
	size_t (*getMaxDataSize)(C_SensorHandle handle);
	bool (*getData)(C_SensorHandle handle, uint8_t* data);
	const char* (*getDescription)(C_SensorHandle handle);

	ISensorUnit (*getUnit)(C_SensorHandle handle);
	bool (*getUseLowerThresholds)(C_SensorHandle handle);
	bool (*getUseUpperThresholds)(C_SensorHandle handle);
	double (*getLowerCriticalThreshold)(C_SensorHandle handle);
	double (*getLowerWarningThreshold)(C_SensorHandle handle);
	double (*getUpperWarningThreshold)(C_SensorHandle handle);
	double (*getUpperCriticalThreshold)(C_SensorHandle handle);

	uint16_t (*getNumberOfValues)(C_SensorHandle handle);
	IRenderingType (*getRenderingType)(C_SensorHandle handle);
	const char* (*getGroup)(C_SensorHandle handle);

	C_SensorHandle handle;
} C_Sensor;

typedef struct C_JSONSensorProviderHandle_ {
	char c;
}* C_JSONSensorProviderHandle;

typedef struct C_JSONSensorProvider_ {
	const char* (*getSensorsDescription)(C_JSONSensorProviderHandle handle);
	const char* (*getSensorsData)(C_JSONSensorProviderHandle handle);

	C_JSONSensorProviderHandle handle;
} C_JSONSensorProvider;

typedef struct C_SensorProviderHandle_ {
	char c;
}* C_SensorProviderHandle;

typedef struct C_SensorProvider_ {
	uint32_t (*getSensorsCount)(C_SensorProviderHandle handle);
	const char* (*getSensorName)(C_SensorProviderHandle handle, uint32_t id);
	C_Sensor* (*getSensor)(C_SensorProviderHandle handle, uint32_t id);

	C_SensorProviderHandle handle;
} C_SensorProvider;

#endif
