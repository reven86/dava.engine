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


#ifndef __DAVAENGINE_ANDROID_CRASH_UTILITY_H__
#define __DAVAENGINE_ANDROID_CRASH_UTILITY_H__

#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_ANDROID__)

//__arm__ should only be defined when compiling on arm32
#if defined(__arm__)

#include "libunwind_stab.h"
namespace DAVA
{

///! On ARM context returened by kernel to signal handler is not the same data
///! structure as context used by libunwind so we need to convert
void ConvertContextARM(ucontext_t * from,unw_context_t * to);


DAVA::int32 GetAndroidBacktrace(unw_context_t * context, unw_word_t * outIpStack, DAVA::int32 maxSize);

DAVA::int32 GetAndroidBacktrace(unw_word_t * outIpStack, DAVA::int32 maxSize);

void PrintAndroidBacktrace();
 
//uses added to libunwind functionality to create memory map of
// process
class UnwindProcMaps
{
public:
    UnwindProcMaps();
    bool  FindLocalAddresInfo(unw_word_t addres, char ** libName, unw_word_t * addresInLib);
    ~UnwindProcMaps();
    
private:
    unw_map_cursor mapCursor;
    DAVA::List<unw_map_t> processMap;
    
};

}
#endif //#if defined(__arm__)
#endif //#if defined(__DAVAENGINE_ANDROID__)
#endif /* #ifndef __DAVAENGINE_ANDROID_CRASH_UTILITY_H__ */