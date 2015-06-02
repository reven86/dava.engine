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


#ifndef LIBCORKSCREW_STAB_H_
#define LIBCORKSCREW_STAB_H_

#include <dlfcn.h>
#include <unistd.h>
///------------------------------------------------------------------------
///--- libcorkscrew.h is not provided in NDK-10 and will never be provided
///--- libcorkscrew was replaced by libunwind in android 5.0
///--- begin stab from libunwind-arm.h
///------------------------------------------------------------------------
//libcorkscrew definition
typedef struct map_info {
    struct map_info* next;
    uintptr_t start;
    uintptr_t end;
    bool is_readable;
    bool is_executable;
    void* data; // arbitrary data associated with the map by the user, initially NULL
    char name[];
} map_info_t;

typedef struct {
    uintptr_t absolute_pc;
    uintptr_t stack_top;
    size_t stack_size;
} backtrace_frame_t;

typedef struct {
    uintptr_t relative_pc;
    uintptr_t relative_symbol_addr;
    char* map_name;
    char* symbol_name;
    char* demangled_name;
} backtrace_symbol_t;

typedef ssize_t (*t_unwind_backtrace_signal_arch)(siginfo_t* si, void* sc, const map_info_t* lst, backtrace_frame_t* bt, size_t ignore_depth, size_t max_depth);
extern t_unwind_backtrace_signal_arch unwind_backtrace_signal_arch;

typedef map_info_t* (*t_acquire_my_map_info_list)();
extern t_acquire_my_map_info_list acquire_my_map_info_list;

typedef void (*t_release_my_map_info_list)(map_info_t* milist);
extern t_release_my_map_info_list release_my_map_info_list;

typedef void (*t_get_backtrace_symbols)(const backtrace_frame_t* backtrace, size_t frames, backtrace_symbol_t* symbols);
extern t_get_backtrace_symbols get_backtrace_symbols;

typedef void (*t_free_backtrace_symbols)(backtrace_symbol_t* symbols, size_t frames);
extern t_free_backtrace_symbols free_backtrace_symbols;

typedef ssize_t (*t_unwind_backtrace)(backtrace_frame_t* backtrace, size_t ignore_depth, size_t max_depth);
extern t_unwind_backtrace unwind_backtrace;
	
///------------------------------------------------------------------------
///--- end stab libcorkscrew
///------------------------------------------------------------------------
///------------------------------------------------------------------------
///--- function to load libcorkscrew dynamicly
///------------------------------------------------------------------------
bool DynLoadLibcorkscrew();

#endif /* LIBCORKSCREW_STAB_H_ */
