#if !defined(__DAVAENGINE_COREV2__)

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
        unwind_backtrace_signal_arch = reinterpret_cast<t_unwind_backtrace_signal_arch>(dlsym(
        libcorkscrew, "unwind_backtrace_signal_arch"));
        if (!unwind_backtrace_signal_arch)
        {
            LOGE("FRAME_STACK unwind_backtrace_signal_arch not found");
            return false;
        }

        acquire_my_map_info_list = reinterpret_cast<t_acquire_my_map_info_list>(dlsym(
        libcorkscrew, "acquire_my_map_info_list"));
        if (!acquire_my_map_info_list)
        {
            LOGE("FRAME_STACK acquire_my_map_info_list not found");
            return false;
        }

        get_backtrace_symbols = reinterpret_cast<t_get_backtrace_symbols>(dlsym(libcorkscrew,
                                                                                "get_backtrace_symbols"));
        if (!get_backtrace_symbols)
        {
            LOGE("FRAME_STACK get_backtrace_symbols not found");
            return false;
        }

        free_backtrace_symbols = reinterpret_cast<t_free_backtrace_symbols>(dlsym(libcorkscrew,
                                                                                  "free_backtrace_symbols"));
        if (!free_backtrace_symbols)
        {
            LOGE("FRAME_STACK free_backtrace_symbols not found");
            return false;
        }

        release_my_map_info_list = reinterpret_cast<t_release_my_map_info_list>(dlsym(
        libcorkscrew, "release_my_map_info_list"));
        if (!release_my_map_info_list)
        {
            LOGE("FRAME_STACK release_my_map_info_list not found");
            return false;
        }

        //optional for now so no check
        unwind_backtrace = reinterpret_cast<t_unwind_backtrace>(dlsym(
        libcorkscrew, "unwind_backtrace"));

        return true;
    }
    else
    {
        LOGE("FRAME_STACK libcorkscrew not found");
    }
    return false;
}

#endif // !__DAVAENGINE_COREV2__
