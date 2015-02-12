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

#include "AndroidCrashUtility.h"

#include <sys/types.h>
#include <unistd.h>

#include "ExternC/AndroidLayer.h"
#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"


#if defined(__DAVAENGINE_ANDROID__)

//__arm__ should only be defined when compiling on arm32
#if defined(__arm__)

namespace DAVA
{
void ConvertContextARM(ucontext_t * from,unw_context_t * to)
{
    if (from == nullptr || to == nullptr)
    {
        return;
    }
    to->regs[0] = from->uc_mcontext.arm_r0;
    to->regs[1] = from->uc_mcontext.arm_r1;
    to->regs[2] = from->uc_mcontext.arm_r2;
    to->regs[3] = from->uc_mcontext.arm_r3;
    to->regs[4] = from->uc_mcontext.arm_r4;
    to->regs[5] = from->uc_mcontext.arm_r5;
    to->regs[6] = from->uc_mcontext.arm_r6;
    to->regs[7] = from->uc_mcontext.arm_r7;
    to->regs[8] = from->uc_mcontext.arm_r8;
    to->regs[9] = from->uc_mcontext.arm_r9;
    to->regs[10] = from->uc_mcontext.arm_r10;
    to->regs[11] = from->uc_mcontext.arm_fp;
    to->regs[12] = from->uc_mcontext.arm_ip;
    to->regs[13] = from->uc_mcontext.arm_sp;
    to->regs[14] = from->uc_mcontext.arm_lr;
    to->regs[15] = from->uc_mcontext.arm_pc;
}
    
DAVA::int32 GetAndroidBacktrace(unw_context_t * uc, unw_word_t * outIpStack, int maxSize)
{
    unw_cursor_t cursor;
    unw_word_t ip, sp;
    
    int counter = 0;
    unw_init_local(&cursor, uc);
    do
    {
        if (counter < maxSize)
        {
            if(unw_is_signal_frame(&cursor) > 0)
            {
                unw_handle_signal_frame(&cursor);
            }
            
            unw_get_reg(&cursor, UNW_REG_IP, &ip);
            outIpStack[counter] = ip;
            counter++;
        }
        else
        {
            return counter;
        }
       
    }while (unw_step(&cursor) > 0);
    return counter;
}
    
DAVA::int32 GetAndroidBacktrace(unw_word_t * ipStack, DAVA::int32 maxSize)
{
    unw_context_t uc;
    
    unw_tdep_getcontext(&uc);
    return GetAndroidBacktrace(&uc,ipStack,maxSize);
}
    
void PrintAndroidBacktrace()
{
    if(!DynLoadLibunwind())
        return;
    unw_word_t ipStack[256];
    DAVA::int32 sizeOut = GetAndroidBacktrace(ipStack,255);
    
    UnwindProcMaps processMap;
    
    char * libName;
    for (DAVA::int32 i=0;i < sizeOut; i++)
    {
        unw_word_t resultAddres;
        processMap.FindLocalAddresInfo(ipStack[i],&libName,&resultAddres);
        LOGE("CALL-STACK-TRACE %s 0x%08x",libName,resultAddres);
    }
}
    
UnwindProcMaps::UnwindProcMaps()
{
    //loop through all info and ad them to the list
    unw_map_cursor_create(&mapCursor,getpid());
    
    // must be called even after create
    unw_map_cursor_reset(&mapCursor);
    unw_map_t map;
    //int log = unw_map_local_cursor_valid(&mapCursor);
    while(unw_map_cursor_get_next(&mapCursor,&map) > 0)
    {
        processMap.push_back(map);
    }
}
bool UnwindProcMaps::FindLocalAddresInfo(unw_word_t addres, char ** libName, unw_word_t * addresInLib)
{
    for( std::list<unw_map_t>::const_iterator map = processMap.begin();map != processMap.end();++map)
    {
        if(map->start < addres && map->end > addres)
        {
            *libName = map->path;
            *addresInLib = addres - map->start;
            return true;
        }
    }
    *addresInLib = addres;
    *libName = nullptr;
    return false;
}
UnwindProcMaps::~UnwindProcMaps()
{
    unw_map_cursor_destroy(&mapCursor);
    processMap.clear();
}
    
}
#endif //#if defined(__arm__)
#endif //#if defined(__DAVAENGINE_ANDROID__)