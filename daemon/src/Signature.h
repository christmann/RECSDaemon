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
// Created on: 24.04.2015
////////////////////////////////////////////////////////////////////////////////

#ifndef SIGNATURE_H_
#define SIGNATURE_H_

#include <stdint.h>
#include <string>
#include <list>
#include <logger.h>

#ifndef WIN32
#include <openssl/evp.h>
#else
#include <Windows.h>
#include <Wincrypt.h>
#endif

using namespace std;

class Signature {
public:
	Signature();
	virtual ~Signature();

	bool checkSignature(uint8_t* data, size_t dataLength, uint8_t* signature, size_t signatureLength);
private:
#ifndef WIN32
	EVP_MD_CTX *mContext;
	EVP_PKEY *mPublicKey;
#else
	HCRYPTPROV mCryptProvider;
	HCRYPTKEY mPublicKey;
#endif

	static LoggerPtr logger;
	static uint8_t publicKeyData[];
};

#endif /* SIGNATURE_H_ */
