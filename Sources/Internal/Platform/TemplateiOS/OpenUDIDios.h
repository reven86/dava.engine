#ifndef __DAVAENGINE_OPENUDID_iOS_H__
#define __DAVAENGINE_OPENUDID_iOS_H__

#include "Platform/OpenUDIDApple.h"

#if defined(__DAVAENGINE_IPHONE__)

@interface OpenUDIDiOS : OpenUDID
{
}

- (void)setDict:(id)dict forPasteboard:(id)pboard;
- (id)getDataForPasteboard:(id)pboard;
- (id)getPasteboardWithName:(NSString*)name shouldCreate:(BOOL)create setPersistent:(BOOL)persistent;

@end

#endif // #if defined(__DAVAENGINE_IPHONE__)

#endif //__DAVAENGINE_OPENUDID_iOS_H__
