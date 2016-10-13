#include "ClipboardImplOsx.h"
#include "Utils/NSStringUtils.h"

#import <Foundation/Foundation.h>
#import <AppKit/NSPasteboard.h>

namespace DAVA
{
ClipboardImplOsx::ClipboardImplOsx()
{
}

ClipboardImplOsx::~ClipboardImplOsx()
{
}

bool ClipboardImplOsx::IsReadyToUse() const
{
    return [NSPasteboard generalPasteboard] != nil;
}

bool ClipboardImplOsx::ClearClipboard() const
{
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    if (pasteboard)
    {
        [pasteboard clearContents];
        return true;
    }
    return false;
}

bool ClipboardImplOsx::HasText() const
{
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    if (pasteboard)
    {
        NSArray* classes = [[NSArray alloc] initWithObjects:[NSString class], nil];
        BOOL ok = [pasteboard canReadObjectForClasses:classes options:nil];
        return ok == YES;
    }
    return false;
}

bool ClipboardImplOsx::SetText(const WideString& str)
{
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    if (pasteboard)
    {
        NSString* stringToWrite = NSStringFromWideString(str);
        [pasteboard declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];
        BOOL ok = [pasteboard setString:stringToWrite forType:NSStringPboardType];
        return ok == YES;
    }
    return false;
}

WideString ClipboardImplOsx::GetText() const
{
    WideString outPut;
    if (HasText())
    {
        NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
        NSString* s = [pasteboard stringForType:NSStringPboardType];
        WideString wstr = WideStringFromNSString(s);
        return wstr;
    }
    return outPut;
}
}
