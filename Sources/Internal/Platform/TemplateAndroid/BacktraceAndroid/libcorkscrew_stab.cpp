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


#include "libcorkscrew_stab.h"
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>

#include "Base/BaseTypes.h"
#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"
#include "FileSystem/File.h"

t_unwind_backtrace_signal_arch unwind_backtrace_signal_arch = nullptr;
t_acquire_my_map_info_list acquire_my_map_info_list = nullptr;
t_release_my_map_info_list release_my_map_info_list = nullptr;
t_get_backtrace_symbols get_backtrace_symbols = nullptr;
t_free_backtrace_symbols free_backtrace_symbols = nullptr;
t_unwind_backtrace unwind_backtrace = nullptr;

bool DynLoadLibcorkscrew()
{
    LOGE("FRAME_STACK loading corkscrew");
    if (unwind_backtrace_signal_arch != nullptr)
        return true;

    void* libcorkscrew = dlopen("/system/lib/libcorkscrew.so", RTLD_NOW);
    if (libcorkscrew)
    {
        unwind_backtrace_signal_arch = (t_unwind_backtrace_signal_arch) dlsym(
                libcorkscrew, "unwind_backtrace_signal_arch");
        if (!unwind_backtrace_signal_arch)
        {
            LOGE("FRAME_STACK unwind_backtrace_signal_arch not found");
            return false;
        }
        

        acquire_my_map_info_list = (t_acquire_my_map_info_list) dlsym(
                libcorkscrew, "acquire_my_map_info_list");
        if (!acquire_my_map_info_list)
        {
            LOGE("FRAME_STACK acquire_my_map_info_list not found");
            return false;
        }
        
        get_backtrace_symbols = (t_get_backtrace_symbols) dlsym(libcorkscrew,
                "get_backtrace_symbols");
        if (!get_backtrace_symbols)
        {
            LOGE("FRAME_STACK get_backtrace_symbols not found");
            return false;
        }

        free_backtrace_symbols = (t_free_backtrace_symbols) dlsym(libcorkscrew,
                "free_backtrace_symbols");
        if (!free_backtrace_symbols)
        {
            LOGE("FRAME_STACK free_backtrace_symbols not found");
            return false;
        }

        release_my_map_info_list = (t_release_my_map_info_list) dlsym(
                libcorkscrew, "release_my_map_info_list");
        if (!release_my_map_info_list)
        {
            LOGE("FRAME_STACK release_my_map_info_list not found");
            return false;
        }
        
        //optional for now so no check
        unwind_backtrace = (t_unwind_backtrace)dlsym(
                libcorkscrew, "unwind_backtrace");
        
        return true;
    }
    else
    {
        LOGE("FRAME_STACK libcorkscrew not found");

    }
    return false;
}

