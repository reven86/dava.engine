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
BOOL NSStringCheck(NSUInteger insertLocation, const NSString* origStr, const NSString* newStr, DAVA::UITextField* cppTextField, NSString** replStr, BOOL& clientApply)
{
    BOOL applyChanges = YES;
    NSUInteger replStrLength = [*replStr length];
    NSUInteger origStrLength = [origStr length];
    NSUInteger finalStrLength = [newStr length];
    // only if (maxLength > 0), we need cut the replStr. if it's really need
    DAVA::int32 maxLength = cppTextField->GetMaxLength();
    NSUInteger removeSymbols = Max(finalStrLength, origStrLength + replStrLength) - Min(finalStrLength, origStrLength + replStrLength);
    NSUInteger insertSymbols = replStrLength;
    NSUInteger cutSymbols = 0;
    // only if need cut text
    if (maxLength > 0 && replStrLength > 0 && finalStrLength > maxLength)
    {
        NSUInteger charsToInsert = maxLength - origStrLength + removeSymbols;
        applyChanges = NO;
        *replStr = DAVA::NSStringSafeCut(*replStr, charsToInsert);
        insertSymbols = [*replStr length];
        cutSymbols = replStrLength - insertSymbols;
    }
    // Length check OK, continue with the delegate.
    DAVA::WideString clientString = L"";
    const char* cutfstr = [*replStr cStringUsingEncoding:NSUTF8StringEncoding];
    if (nullptr != cutfstr) //cause strlen(nullptr) will crash
    {
        DAVA::int32 len = static_cast<DAVA::int32>(strlen(cutfstr));
        const DAVA::uint8* str = reinterpret_cast<const DAVA::uint8*>(cutfstr);
        DAVA::UTF8Utils::EncodeToWideString(str, len, clientString);
    }
    else
    {
        const char* finalString = [newStr cStringUsingEncoding:NSUTF8StringEncoding];
        DAVA::Logger::Error("NSStringUtils::CheckText: Error! cStringUsingEncoding:NSUTF8StringEncoding with string %s, info - m_kiselev", finalString);
        return NO;
    }
    clientApply = cppTextField->GetDelegate()->TextFieldKeyPressed(cppTextField, static_cast<DAVA::int32>(insertLocation), static_cast<DAVA::int32>(removeSymbols), clientString);
    if (!clientApply)
    {
        return NO;
    }
    return applyChanges;
}

// replString input string
// newLength - sought length, may be less
// example replString = 1234👨‍👩‍👧‍👦, newLength = 6
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
    // change replacement string and final string, if needs
    // removeSumbols - length characters for remove from replString
    //    removeSumbols = replStrLength - position;
    NSString* outString = [replString substringWithRange:NSMakeRange(0, position)];
    //    replStrLength = position;
    //    replRange.length = position;
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
