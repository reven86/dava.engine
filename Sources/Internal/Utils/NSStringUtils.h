#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)

#import <Foundation/Foundation.h>

namespace DAVA
{
NSString* NSStringFromString(const DAVA::String& str);

NSString* NSStringFromWideString(const DAVA::WideString& str);

String StringFromNSString(NSString* string);

WideString WideStringFromNSString(NSString* string);
}

#endif //#if defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_IPHONE__)
