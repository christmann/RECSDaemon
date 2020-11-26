////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2019 christmann informationstechnik + medien GmbH & Co. KG
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

#ifndef LOGGER_H_
#define LOGGER_H_

#include <ctime>
#include <iostream>
#include <string>

using namespace std;

class Logger;
typedef string LoggerPtr;

#define LOG_WRITE(logger, message, level, stream) { \
	    time_t     now = time(0); \
	    struct tm  tstruct; \
	    char       buf[10]; \
	    tstruct = *localtime(&now); \
	    strftime(buf, sizeof(buf), "%H:%M:%S ", &tstruct); \
        stream << buf << level << " " << logger << " - " << dec << message << endl << flush; }

#define LOG_ERROR(logger, message) LOG_WRITE(logger, message, "ERROR", cerr)
#define LOG_WARN(logger, message)  LOG_WRITE(logger, message, "WARN ", cout)
#define LOG_INFO(logger, message)  LOG_WRITE(logger, message, "INFO ", cout)
#define LOG_DEBUG(logger, message) LOG_WRITE(logger, message, "DEBUG", cout)

class Logger {
public:
	static LoggerPtr getLogger(string name) {
		return name;
	}
};

#endif /* LOGGER_H_ */
