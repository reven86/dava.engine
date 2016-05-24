#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)

#import <Foundation/Foundation.h>

namespace DAVA
{
BOOL NSStringModified(const NSRange& origRange, const NSString* origStr, DAVA::int32 maxLength, NSString** replStr);

NSString* NSStringSafeCut(const NSString* inStr, NSUInteger newLength);

NSString* NSStringFromString(const DAVA::String& str);

NSString* NSStringFromWideString(const DAVA::WideString& str);

String StringFromNSString(NSString* string);

WideString WideStringFromNSString(NSString* string);
}

#endif //#if defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_IPHONE__)
