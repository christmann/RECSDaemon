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

#include "Signature.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#ifndef WIN32
#include <openssl/err.h>
#include <openssl/rsa.h>
#else
#ifndef CRYPT_STRING_BASE64HEADER
#define CRYPT_STRING_BASE64HEADER 0x0
#endif
#endif

using namespace log4cxx;
using namespace std;

LoggerPtr Signature::logger(Logger::getLogger("Signature"));

//-----BEGIN RSA PUBLIC KEY-----
//MIGJAoGBAKmTViSANl5jh5o/J+Wsm6RHSDqLW2J2QXFf9ezkDqEtPX8UdTGQDSjv
//MF6wo/+KDFzQFlM2WGAP/j3w4uBvf/ibDHqit98kYjIP0i7DF+vqRIDmp+qxGwu1
//FdpjkcLkleoIf7g8wvR5ur/QwjuTe3pE5/0VipMJr0DyvVdjv2A5AgMBAAE=
//-----END RSA PUBLIC KEY-----
uint8_t Signature::publicKeyData[] = {	0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xa9, 0x93, 0x56, 0x24, 0x80, 0x36, 0x5e, 0x63, 0x87,
										0x9a, 0x3f, 0x27, 0xe5, 0xac, 0x9b, 0xa4, 0x47, 0x48, 0x3a, 0x8b, 0x5b, 0x62, 0x76, 0x41, 0x71,
										0x5f, 0xf5, 0xec, 0xe4, 0x0e, 0xa1, 0x2d, 0x3d, 0x7f, 0x14, 0x75, 0x31, 0x90, 0x0d, 0x28, 0xef,
										0x30, 0x5e, 0xb0, 0xa3, 0xff, 0x8a, 0x0c, 0x5c, 0xd0, 0x16, 0x53, 0x36, 0x58, 0x60, 0x0f, 0xfe,
										0x3d, 0xf0, 0xe2, 0xe0, 0x6f, 0x7f, 0xf8, 0x9b, 0x0c, 0x7a, 0xa2, 0xb7, 0xdf, 0x24, 0x62, 0x32,
										0x0f, 0xd2, 0x2e, 0xc3, 0x17, 0xeb, 0xea, 0x44, 0x80, 0xe6, 0xa7, 0xea, 0xb1, 0x1b, 0x0b, 0xb5,
										0x15, 0xda, 0x63, 0x91, 0xc2, 0xe4, 0x95, 0xea, 0x08, 0x7f, 0xb8, 0x3c, 0xc2, 0xf4, 0x79, 0xba,
										0xbf, 0xd0, 0xc2, 0x3b, 0x93, 0x7b, 0x7a, 0x44, 0xe7, 0xfd, 0x15, 0x8a, 0x93, 0x09, 0xaf, 0x40,
										0xf2, 0xbd, 0x57, 0x63, 0xbf, 0x60, 0x39, 0x02, 0x03, 0x01, 0x00, 0x01 };

Signature::Signature() {
#ifndef WIN32
    unsigned char *p;
    p = &publicKeyData[0];
    RSA* rsa_pubkey = d2i_RSAPublicKey(NULL, (const unsigned char**)&p, sizeof(publicKeyData));
    if (rsa_pubkey == NULL) {
    	LOG4CXX_ERROR(logger, "Could parse public key, error 0x" << hex << ERR_get_error());
    	return;
    }

    mPublicKey = EVP_PKEY_new();
    if(!EVP_PKEY_assign_RSA(mPublicKey, rsa_pubkey)) {
    	LOG4CXX_ERROR(logger, "Could not assign public key, error 0x" << hex << ERR_get_error());
        return;
    }

    mContext = NULL;
    mContext = EVP_MD_CTX_create();
    if(mContext == NULL) {
        LOG4CXX_ERROR(logger, "EVP_MD_CTX_create failed, error 0x" << hex << ERR_get_error());
        return;
    }
#else
	unsigned char *publicKeyBlob;
	DWORD publicKeyBlobLen;

	if (!CryptDecodeObjectEx(X509_ASN_ENCODING, RSA_CSP_PUBLICKEYBLOB, publicKeyData, sizeof(publicKeyData), CRYPT_ENCODE_ALLOC_FLAG, NULL, &publicKeyBlob, &publicKeyBlobLen)) {
		LOG4CXX_ERROR(logger, "Could not decode DER public key, error 0x" << hex << GetLastError());
		return;
	}

	if (!CryptAcquireContext(&mCryptProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
		LOG4CXX_ERROR(logger, "Could not acquire crypto context, error 0x" << hex << GetLastError());
		return;
	}

	//if (!CryptImportPublicKeyInfo(mCryptProvider, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, publicKeyInfo, &mPublicKey)) {
	if (!CryptImportKey(mCryptProvider, publicKeyBlob, publicKeyBlobLen, 0, 0, &mPublicKey)) {
		LOG4CXX_ERROR(logger, "Could not import public key, error 0x" << hex << GetLastError());
		return;
	}
	LocalFree(publicKeyBlob);
#endif
}

Signature::~Signature() {
#ifndef WIN32
	EVP_PKEY_free(mPublicKey);
    EVP_MD_CTX_destroy(mContext);
#else
	if (mCryptProvider) {
	   CryptReleaseContext(mCryptProvider, 0);
	}
#endif
}

bool Signature::checkSignature(uint8_t* data, size_t dataLength, uint8_t* signature, size_t signatureLength) {
#ifndef WIN32
    if (EVP_DigestVerifyInit(mContext, NULL, EVP_sha1(), NULL, mPublicKey) != 1) {
    	LOG4CXX_ERROR(logger, "Could not initialize verify, error 0x" << hex << ERR_get_error());
    	return false;
    }

    if (EVP_DigestVerifyUpdate(mContext, data, dataLength) != 1) {
    	LOG4CXX_ERROR(logger, "Could not update data, error 0x" << hex << ERR_get_error());
    	return false;
    }

    if(EVP_DigestVerifyFinal(mContext, signature, signatureLength) == 1) {
    	return true;
    }

	return false;
#else
	HCRYPTHASH hash;
	if (!CryptCreateHash(mCryptProvider, CALG_SHA1, 0, 0, &hash)) {
		LOG4CXX_ERROR(logger, "Could not create hash, error 0x" << hex << GetLastError());
		return false;
	}
	if (!CryptHashData(hash, data, dataLength, 0)) {
		LOG4CXX_ERROR(logger, "Could not hash data, error 0x" << hex << GetLastError());
		return false;
	}

	// Reverse signature as CryptoAPI expects little-endian
	uint8_t* sigRev = (uint8_t*)malloc(signatureLength);
	for (size_t i = 0; i < signatureLength; ++i) {
		sigRev[i] = signature[signatureLength-1-i];
	}

	if (!CryptVerifySignature(hash, sigRev, signatureLength, mPublicKey, NULL, 0)) {
		free(sigRev);
		CryptDestroyHash(hash);
		LOG4CXX_ERROR(logger, "Could not verify signature, error 0x" << hex << GetLastError());
		return false;
	}
	free(sigRev);
	CryptDestroyHash(hash);
	return true;
#endif
}
