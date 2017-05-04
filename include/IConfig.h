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
// Created on: 23.10.2014
////////////////////////////////////////////////////////////////////////////////

#ifndef ICONFIG_H_
#define ICONFIG_H_

#include <string>

using namespace std;

class IConfig {
public:
	virtual ~IConfig() {};
	virtual bool GetBoolean(string Section, string Setting, bool Default) = 0;
	virtual bool SetBoolean(string Section, string Setting, bool Value) = 0;
	virtual string GetString(string Section, string Setting, string Default) = 0;
	virtual bool SetString(string Section, string Setting, string Value) = 0;
	virtual int GetInt(string Section, string Setting, int Default) = 0;
	virtual bool SetInt(string Section, string Setting, int Value) = 0;
	virtual double GetDouble(string Section, string Setting, double Default) = 0;
	virtual bool SetDouble(string Section, string Setting, double Value) = 0;
};

#endif /* ICONFIG_H_ */
