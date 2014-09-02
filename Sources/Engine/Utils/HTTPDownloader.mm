/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Utils/HTTPDownloader.h"
#include "Utils/Utils.h"
#include "FileSystem/FileSystem.h"

#if defined(__DAVAENGINE_MACOS__)
#import <Foundation/NSObject.h>
#import <Foundation/NSURLConnection.h>
#import <Foundation/NSData.h>
#import <Foundation/NSString.h>
#import <Foundation/NSURLRequest.h>
#import <Foundation/NSURL.h>
#endif //#if defined(__DAVAENGINE_MACOS__)

namespace DAVA 
{

bool DownloadFileFromURLToDocuments(const String & url, const FilePath & documentsPathname)
{
	NSString * localizedPath = [NSString stringWithUTF8String:url.c_str()];
//	NSError *error = nil;
//TODO: На маке эта хрень возвращает страницу с информацией о том, что такой файл не найден. Никаких ошибок при этом не генерится. Нужно найти решение.
    NSData * fileContents = [NSData dataWithContentsOfURL:[NSURL URLWithString:localizedPath]];
    if (fileContents) 
    {
        [fileContents writeToFile:[NSString stringWithCString: documentsPathname.GetAbsolutePathname().c_str() encoding:NSUTF8StringEncoding] atomically:true];
        return true;
    }
    return false;
}
};
