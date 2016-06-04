#ifndef __DAVAENGINE_OPENUDIDMACOS_H__
#define __DAVAENGINE_OPENUDIDMACOS_H__

#include "Base/BaseTypes.h"
#include "Platform/OpenUDIDApple.h"

#if defined(__DAVAENGINE_MACOS__)

#import <Foundation/Foundation.h>

@interface OpenUDIDMacOS : OpenUDID
{
}

- (void)setDict:(id)dict forPasteboard:(id)pboard;
- (id)getDataForPasteboard:(id)pboard;
- (id)getPasteboardWithName:(NSString*)name shouldCreate:(BOOL)create setPersistent:(BOOL)persistent;

@end


#endif // #if defined(__DAVAENGINE_MACOS__)

#endif //__DAVAENGINE_OPENUDIDMACOS_H__
