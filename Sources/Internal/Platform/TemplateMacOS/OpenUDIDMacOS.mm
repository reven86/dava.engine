#include "OpenUDIDMacOS.h"

#if defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSPasteboard.h>

@implementation OpenUDIDMacOS

- (void)setDict:(id)dict forPasteboard:(id)pboard
{
    [pboard setData:[NSKeyedArchiver archivedDataWithRootObject:dict] forType:kOpenUDIDDomain];
}

- (id)getDataForPasteboard:(id)pboard
{
    return [pboard dataForType:kOpenUDIDDomain];
}

- (id)getPasteboardWithName:(NSString*)name shouldCreate:(BOOL)create setPersistent:(BOOL)persistent
{
    NSPasteboard* slotPB = [NSPasteboard pasteboardWithName:name];
    return reinterpret_cast<id>(slotPB);
}
@end

#endif //#if defined(__DAVAENGINE_MACOS__)
