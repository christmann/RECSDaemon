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

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>

#include <logger.h>

#include "version.h"
#include "Daemon.h"

using namespace std;

int main(int argc, char **argv) {
	LoggerPtr logger(Logger::getLogger("main"));

	LOG_INFO(logger, "RECS daemon " << DAEMON_VERSION_MAJOR << "." << DAEMON_VERSION_MINOR << "." << DAEMON_VERSION_REVISION << " starting");

    // Loop over command-line args
	vector<string> args(argv + 1, argv + argc);
	int exitAfter = 0;
    for (vector<string>::iterator i = args.begin(); i != args.end(); ++i) {
        if (*i == "-h" || *i == "--help") {
            cout << "Syntax: RECSDaemon [-exitAfter n]" << endl;
            return 0;
        } else if (*i == "-exitAfter") {
        	++i;
        	if (i != args.end()) {
				istringstream(*i) >> exitAfter;
				LOG_INFO(logger, "exitAfter set, will exit after " << exitAfter << " update iterations");
        	}
        }
    }

	int ret = (new Daemon())->run(exitAfter);
	return ret;
}
