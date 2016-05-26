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
typedef struct map_info
{
    struct map_info* next;
    uintptr_t start;
    uintptr_t end;
    bool is_readable;
    bool is_executable;
    void* data; // arbitrary data associated with the map by the user, initially NULL
    char name[];
} map_info_t;

typedef struct
{
    uintptr_t absolute_pc;
    uintptr_t stack_top;
    size_t stack_size;
} backtrace_frame_t;

typedef struct
{
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
