#include "NSStringUtils.h"

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)

namespace DAVA
{
NSString* NSStringFromString(const DAVA::String& str)
{
    NSStringEncoding encoding = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingASCII);
    NSString* nsstring = [[[NSString alloc] initWithBytes:str.c_str()
                                                   length:str.length()
                                                 encoding:encoding] autorelease];
    return nsstring;
}

NSString* NSStringFromWideString(const DAVA::WideString& str)
{
    NSStringEncoding encoding = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE);
    const char* string = reinterpret_cast<const char*>(str.c_str());

    NSString* nsstring = [[[NSString alloc] initWithBytes:string
                                                   length:str.length() * sizeof(wchar_t)
                                                 encoding:encoding] autorelease];
    return nsstring;
}

String StringFromNSString(NSString* string)
{
    if (string)
    {
        return String([string cStringUsingEncoding:NSASCIIStringEncoding]);
    }
    else
    {
        return "";
    }
}

WideString WideStringFromNSString(NSString* string)
{
    if (string)
    {
        NSStringEncoding encoding = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE);
        NSData* data = [string dataUsingEncoding:encoding];

        const wchar_t* stringData = reinterpret_cast<const wchar_t*>(data.bytes);
        return WideString(stringData, data.length / sizeof(wchar_t));
    }
    else
    {
        return L"";
    }
}
}

#endif //#if defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_IPHONE__)
