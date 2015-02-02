#include "libunwind_stab.h"
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>

#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_ANDROID__)
//__arm__ should only be defined when compiling on arm32
#if defined(__arm__)
t_unw_init_local unw_init_local = NULL;
t_unw_step unw_step;
t_unw_get_reg unw_get_reg;
t_unw_backtrace unw_backtrace;
t_unw_get_proc_info unw_get_proc_info;
t_unw_map_local_create unw_map_local_create;
t_unw_map_local_destroy unw_map_local_destroy;
t_unw_is_signal_frame unw_is_signal_frame;
t_unw_handle_signal_frame unw_handle_signal_frame;
t_unw_getcontext  unw_getcontext;
t_unw_map_local_cursor_valid unw_map_local_cursor_valid;
t_unw_map_local_cursor_get unw_map_local_cursor_get;
t_unw_map_local_cursor_get_next unw_map_local_cursor_get_next;
t_unw_map_cursor_reset unw_map_cursor_reset;
t_unw_map_cursor_create unw_map_cursor_create;
t_unw_map_cursor_destroy unw_map_cursor_destroy;
t_unw_map_cursor_get_next unw_map_cursor_get_next;


bool DynLoadLibunwind()
{
    if (unw_init_local != NULL)
        return true;
    void * libunwind = dlopen("/system/lib/libunwind.so",RTLD_NOW);
    if (libunwind == NULL)
        return false;
    
    unw_init_local =  (t_unw_init_local)dlsym(libunwind, "_ULarm_init_local");
    if (unw_init_local == NULL)
        return false;
    
    unw_step = (t_unw_step) dlsym(libunwind, "_ULarm_step");
    if (unw_step == NULL)
        return false;
    
    unw_get_reg = (t_unw_get_reg) dlsym(libunwind, "_ULarm_get_reg");
    if (unw_get_reg == NULL)
        return false;
    
    unw_get_proc_info = (t_unw_get_proc_info) dlsym(libunwind, "_ULarm_get_proc_info");
    if(unw_get_proc_info == NULL)
        return false;
    
    unw_map_local_destroy = (t_unw_map_local_destroy) dlsym(libunwind, "unw_map_local_destroy");
    if(unw_map_local_destroy == NULL)
        return false;
    
    unw_map_local_create = (t_unw_map_local_create) dlsym(libunwind, "unw_map_local_create");
    if(unw_map_local_create == NULL)
        return false;
    
    unw_is_signal_frame = (t_unw_is_signal_frame) dlsym(libunwind, "_Uarm_is_signal_frame");
    if(unw_is_signal_frame == NULL)
        return false;
    
    unw_handle_signal_frame = (t_unw_handle_signal_frame) dlsym(libunwind, "_ULarm_handle_signal_frame");
    if(unw_handle_signal_frame == NULL)
        return false;
    
    unw_getcontext = (t_unw_getcontext) dlsym(libunwind, "_Uarm_getcontext");
    if(unw_getcontext == NULL)
        return false;
    
    unw_backtrace = (t_unw_backtrace)dlsym(libunwind, "unw_backtrace");
    if(unw_backtrace == NULL)
        return false;
    
    unw_map_local_cursor_valid = (t_unw_map_local_cursor_valid) dlsym(libunwind, "unw_map_local_cursor_valid");
    if(unw_map_local_cursor_valid == NULL)
        return false;
    
    unw_map_local_cursor_get = (t_unw_map_local_cursor_get) dlsym(libunwind, "unw_map_local_cursor_get");
    if(unw_map_local_cursor_get == NULL)
        return false;
    
    unw_map_local_cursor_get_next = (t_unw_map_local_cursor_get_next) dlsym(libunwind, "unw_map_local_cursor_get_next");
    if(unw_map_local_cursor_get_next == NULL)
        return false;
    
    unw_map_cursor_reset = (t_unw_map_cursor_reset) dlsym(libunwind, "unw_map_cursor_reset");
    if(unw_map_cursor_reset == NULL)
        return false;
    
    unw_map_cursor_create  = (t_unw_map_cursor_create) dlsym(libunwind, "unw_map_cursor_create");
    if(unw_map_cursor_create == NULL)
        return false;
    
    unw_map_cursor_destroy  = (t_unw_map_cursor_destroy) dlsym(libunwind, "unw_map_cursor_destroy");
    if(unw_map_cursor_destroy == NULL)
        return false;
    
    unw_map_cursor_get_next  = (t_unw_map_cursor_get_next) dlsym(libunwind, "unw_map_cursor_get_next");
    if(unw_map_cursor_get_next == NULL)
        return false;
    
    return true;
}
#endif //#if defined(__arm__)
#endif //#if defined(__DAVAENGINE_ANDROID__)

