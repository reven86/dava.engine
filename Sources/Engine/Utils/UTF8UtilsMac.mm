/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Utils/UTF8Utils.h"
#include "FileSystem/Logger.h"

// This code is identical both for MacOSX and iOS.
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)

#import <Foundation/Foundation.h>

namespace DAVA 
{

void UTF8Utils::EncodeToWideString(uint8 * string, int32 size, WideString & resultString)
{
	char* buf = new char[size + 1];
	memcpy(buf, string, size);
	buf[size] = 0;

	NSString* nsstring = [[NSString alloc] initWithUTF8String:buf];
	delete[] buf;

	NSStringEncoding encoding = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE);
	NSData* data = [nsstring dataUsingEncoding:encoding];

	resultString = WideString((wchar_t*)[data bytes], [data length] / sizeof(wchar_t));

	[nsstring release];
}

String UTF8Utils::EncodeToUTF8(const WideString& wstring)
{
	NSStringEncoding encoding = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE);
	NSString* nsstring = [[NSString alloc]
						  initWithBytes:(const char*)wstring.c_str()
						  length:wstring.length() * sizeof(wchar_t)
						  encoding:encoding];

	String res = [nsstring UTF8String];

	[nsstring release];

	return res;
}

};

#endif
