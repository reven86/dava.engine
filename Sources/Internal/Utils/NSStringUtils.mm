/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "NSStringUtils.h"
#include "UTF8Utils.h"

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)

namespace DAVA
{
// for example
// maxLength  6
// origStr    q123we
// newStr     q7777we
// replStr    7777
// remove 3, insert 4
// charsToInsert 6 - 6 + (6 + 4 - 7) = 3 //with cut
// return YES - if need apply changes, or NO
BOOL NSStringCheck(const NSRange* origRange, const NSString* origStr, DAVA::int32 maxLength, NSString** replStr)
{
    BOOL stringModify = NO;
    NSUInteger replStrLength = [*replStr length];
    NSUInteger origStrLength = [origStr length];
    NSUInteger removeSymbols = origRange->length;
    NSUInteger insertSymbols = replStrLength;
    NSUInteger finalStrLength = replStrLength + origStrLength - removeSymbols;
    NSUInteger cutSymbols = 0;
    // only if need cut text
    if (maxLength > 0 && replStrLength > 0 && finalStrLength > maxLength)
    {
        NSUInteger charsToInsert = maxLength - origStrLength + removeSymbols;
        stringModify = YES;
        *replStr = DAVA::NSStringSafeCut(*replStr, charsToInsert);
        insertSymbols = [*replStr length];
        cutSymbols = replStrLength - insertSymbols;
    }
    return stringModify;
}

// replString input string
// newLength - sought length, may be less
// example replString = 1234(emoji > 2), newLength = 6
// outString = 1234
NSString* NSStringSafeCut(const NSString* replString, NSUInteger newLength)
{
    if (nullptr == replString || [replString length] == 0 || [replString length] <= newLength)
    {
        return @"";
    }
    NSUInteger charsToInsert = newLength;
    NSUInteger position = 0;
    NSRange rangeCharacter;
    NSInteger index = 0;
    do
    {
        rangeCharacter = [replString rangeOfComposedCharacterSequenceAtIndex:index];
        if ((rangeCharacter.location + rangeCharacter.length) > charsToInsert)
        {
            position = rangeCharacter.location;
            break;
        }
        position = rangeCharacter.location + rangeCharacter.length;
        index++;
    }
    while ((rangeCharacter.location + rangeCharacter.length) < charsToInsert);
    NSString* outString = [replString substringWithRange:NSMakeRange(0, position)];
    return outString;
}

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
