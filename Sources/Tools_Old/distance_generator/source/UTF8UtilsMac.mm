#include "UTF8Utils.h"

#import <Foundation/Foundation.h>

using namespace std;

void UTF8Utils::EncodeToWideString(const char* str, int size, std::wstring& resultString)
{
	char* buf = new char[size + 1];
	memcpy(buf, str, size);
	buf[size] = 0;

	NSString* nsstring = [[NSString alloc] initWithUTF8String:buf];
	delete[] buf;

	NSStringEncoding encoding = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE);
	NSData* data = [nsstring dataUsingEncoding:encoding];

	resultString = wstring((wchar_t*)[data bytes], [data length] / sizeof(wchar_t));

	[nsstring release];
}

string UTF8Utils::EncodeToUTF8(const wstring& wstring)
{
	NSStringEncoding encoding = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE);
	NSString* nsstring = [[NSString alloc]
						  initWithBytes:(const char*)wstring.c_str()
						  length:wstring.length() * sizeof(wchar_t)
						  encoding:encoding];

	string res = [nsstring UTF8String];

	[nsstring release];

	return res;
}
