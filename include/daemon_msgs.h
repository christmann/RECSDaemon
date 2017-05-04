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
// Created on: 15.10.2014
////////////////////////////////////////////////////////////////////////////////

#ifndef DAEMON_MSGS_H_
#define DAEMON_MSGS_H_

typedef struct __attribute__((__packed__)) {
	uint8_t		length[2];
	uint16_t	fanSpeed[6];
	uint16_t	voltage[8];
	uint16_t	current[8];
	uint8_t		temperature[1 + 4];
	uint8_t		powerState[4];
	uint16_t	flags;
} BB_Monitoring;

typedef struct __attribute__((__packed__)) {
	uint8_t		magic[4]; // "RECS"
	uint16_t	size; // Total size of shared memory
	uint8_t		baseboardID; // Baseboard ID (Bit 7 set = I2C address instead of logical ID)
	uint8_t		maxSlots; // Number of slots on this baseboard
	uint8_t		version; // Header version
	uint8_t		reserved;
	uint16_t	chunksOffset; // Begin of chunks area
	uint16_t	dynamicAreaOffset; // Begin of "dynamic area" (shared memory used by nodes)
} Daemon_Header; // 14 bytes

typedef struct __attribute__((__packed__)) {
	uint8_t		chunkType;
	uint16_t	length;
} Chunk_Header; // 3 bytes

#define CHUNK_TYPE_MONITORING	1

typedef struct __attribute__((__packed__)) {
	uint8_t		type;
	uint16_t	size;
	uint8_t		version;
} Message_Header;

typedef struct __attribute__((__packed__)) {
	Message_Header	header;
	uint16_t		sensorCnt;
	uint8_t			flags;
} Monitoring_Data_Header;

typedef struct __attribute__((__packed__)) {
	Message_Header	header;
	uint8_t			currentPage;
	uint8_t			maxPages;
	uint8_t			sensorEntries;
	uint16_t		startIndex;
} Monitoring_Description_Header;

#define SENSOR_NAME_LENGTH	29

typedef struct __attribute__((__packed__)) {
	uint8_t			entryLength;				// 0
	uint8_t			name[SENSOR_NAME_LENGTH];	// 1
	uint8_t			dataType;					// SENSOR_NAME_LENGTH + 1
	uint8_t			unit;						// SENSOR_NAME_LENGTH + 2
	uint16_t		maxDataSize;				// SENSOR_NAME_LENGTH + 3
	uint8_t			useLowerThresholds;			// SENSOR_NAME_LENGTH + 5
	uint8_t			useUpperThresholds;			// SENSOR_NAME_LENGTH + 6
	double			lowerCriticalThreshold; 	// SENSOR_NAME_LENGTH + 7 // Typically 8 bytes
	double			lowerWarningThreshold;		// SENSOR_NAME_LENGTH + 15
	double 			upperWarningThreshold;		// SENSOR_NAME_LENGTH + 23
	double			upperCriticalThreshold;		// SENSOR_NAME_LENGTH + 31
	uint16_t		numberOfValues;				// SENSOR_NAME_LENGTH + 39
	uint8_t			groupId;					// SENSOR_NAME_LENGTH + 41
	uint8_t			renderingType;				// SENSOR_NAME_LENGTH + 42
} Sensor_Description;

#define HOSTNAME_LENGTH		256

typedef struct __attribute__((__packed__)) {
	uint8_t			mac[6];
	uint8_t			ip[16];
	uint8_t			netmask[4];
} Network_Interface;

typedef struct __attribute__((__packed__)) {
	Message_Header	header;
	uint8_t			daemonVersion[3];
	Network_Interface management;
	Network_Interface compute;
	uint8_t			hostname[HOSTNAME_LENGTH];
} Basic_Information_Block;

#define COMMAND_MAX_LENGTH	64
#define SIGNATURE_LENGTH	128 // 1024 bits key length

typedef struct __attribute__((__packed__)) {
	Message_Header	header;
	uint8_t			baseboard;
	uint8_t			slot;
	uint32_t		timestamp;
	uint8_t			command[COMMAND_MAX_LENGTH];
	uint8_t			signature[SIGNATURE_LENGTH];
	uint16_t		parametersLength;
} Command_Header;

enum Message_Type {
	Empty = 0,
	Monitoring_Description = 1,
	Monitoring_Data = 2,
	Command = 3,
	Command_Result = 4,
	Basic_Information = 5
};

#endif /* DAEMON_MSGS_H_ */
