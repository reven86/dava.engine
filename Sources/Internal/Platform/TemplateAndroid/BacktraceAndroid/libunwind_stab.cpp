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

#include "libunwind_stab.h"
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>

#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_ANDROID__)
//__arm__ should only be defined when compiling on arm32
#if defined(__arm__)

t_unw_init_local unw_init_local = nullptr;
t_unw_step unw_step = nullptr;
t_unw_get_reg unw_get_reg = nullptr;
t_unw_backtrace unw_backtrace = nullptr;
t_unw_get_proc_info unw_get_proc_info = nullptr;
t_unw_map_local_create unw_map_local_create = nullptr;
t_unw_map_local_destroy unw_map_local_destroy = nullptr;
t_unw_is_signal_frame unw_is_signal_frame = nullptr;
t_unw_handle_signal_frame unw_handle_signal_frame = nullptr;
t_unw_getcontext  unw_getcontext = nullptr;
t_unw_map_local_cursor_valid unw_map_local_cursor_valid = nullptr;
t_unw_map_local_cursor_get unw_map_local_cursor_get = nullptr;
t_unw_map_local_cursor_get_next unw_map_local_cursor_get_next = nullptr;
t_unw_map_cursor_reset unw_map_cursor_reset = nullptr;
t_unw_map_cursor_create unw_map_cursor_create = nullptr;
t_unw_map_cursor_destroy unw_map_cursor_destroy = nullptr;
t_unw_map_cursor_get_next unw_map_cursor_get_next = nullptr;
t_unw_get_proc_name unw_get_proc_name = nullptr;

bool DynLoadLibunwind()
{
    if (unw_init_local != nullptr)
    {
        return true;
    }
    void * libunwind = dlopen("/system/lib/libunwind.so",RTLD_NOW);
    if (libunwind == nullptr)
    {
        return false;
    }
    
    unw_init_local =  (t_unw_init_local)dlsym(libunwind, "_ULarm_init_local");
    if (unw_init_local == nullptr)
    {
        return false;
    }
    
    unw_step = (t_unw_step) dlsym(libunwind, "_ULarm_step");
    if (unw_step == nullptr)
    {
        return false;
    }
    
    unw_get_reg = (t_unw_get_reg) dlsym(libunwind, "_ULarm_get_reg");
    if (unw_get_reg == nullptr)
    {
        return false;
    }
    
    unw_get_proc_info = (t_unw_get_proc_info) dlsym(libunwind, "_ULarm_get_proc_info");
    if(unw_get_proc_info == nullptr)
    {
        return false;
    }
    
    unw_map_local_destroy = (t_unw_map_local_destroy) dlsym(libunwind, "unw_map_local_destroy");
    if(unw_map_local_destroy == nullptr)
    {
        return false;
    }
    
    unw_map_local_create = (t_unw_map_local_create) dlsym(libunwind, "unw_map_local_create");
    if(unw_map_local_create == nullptr)
    {
        return false;
    }
    
    unw_is_signal_frame = (t_unw_is_signal_frame) dlsym(libunwind, "_Uarm_is_signal_frame");
    if(unw_is_signal_frame == nullptr)
    {
        return false;
    }
    
    unw_handle_signal_frame = (t_unw_handle_signal_frame) dlsym(libunwind, "_ULarm_handle_signal_frame");
    if(unw_handle_signal_frame == nullptr)
    {
        return false;
    }
    
    unw_getcontext = (t_unw_getcontext) dlsym(libunwind, "_Uarm_getcontext");
    if(unw_getcontext == nullptr)
    {
        return false;
    }
    
    unw_backtrace = (t_unw_backtrace)dlsym(libunwind, "unw_backtrace");
    if(unw_backtrace == nullptr)
    {
        return false;
    }
    
    unw_map_local_cursor_valid = (t_unw_map_local_cursor_valid) dlsym(libunwind, "unw_map_local_cursor_valid");
    if(unw_map_local_cursor_valid == nullptr)
    {
        return false;
    }
    
    unw_map_local_cursor_get = (t_unw_map_local_cursor_get) dlsym(libunwind, "unw_map_local_cursor_get");
    if(unw_map_local_cursor_get == nullptr)
    {
        return false;
    }
    
    unw_map_local_cursor_get_next = (t_unw_map_local_cursor_get_next) dlsym(libunwind, "unw_map_local_cursor_get_next");
    if(unw_map_local_cursor_get_next == nullptr)
    {
        return false;
    }
    
    unw_map_cursor_reset = (t_unw_map_cursor_reset) dlsym(libunwind, "unw_map_cursor_reset");
    if(unw_map_cursor_reset == nullptr)
    {
        return false;
    }
    
    unw_map_cursor_create  = (t_unw_map_cursor_create) dlsym(libunwind, "unw_map_cursor_create");
    if(unw_map_cursor_create == nullptr)
    {
        return false;
    }
    
    unw_map_cursor_destroy  = (t_unw_map_cursor_destroy) dlsym(libunwind, "unw_map_cursor_destroy");
    if(unw_map_cursor_destroy == nullptr)
    {
        return false;
    }
    
    unw_map_cursor_get_next  = (t_unw_map_cursor_get_next) dlsym(libunwind, "unw_map_cursor_get_next");
    if(unw_map_cursor_get_next == nullptr)
    {
        return false;
    }
    unw_get_proc_name  = (t_unw_get_proc_name) dlsym(libunwind, "_Uarm_get_proc_name");
    if(unw_get_proc_name == nullptr)
    {
        return false;
    }
    
    return true;
}
#endif //#if defined(__arm__)
#endif //#if defined(__DAVAENGINE_ANDROID__)

