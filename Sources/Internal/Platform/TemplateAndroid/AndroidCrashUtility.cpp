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
    if (from == NULL || to == NULL)
        return;
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
    
size_t GetBacktrace(unw_context_t * uc, unw_word_t * outIpStack, int maxSize)
{
    unw_cursor_t cursor;
    unw_word_t ip, sp;
    
    size_t counter = 0;
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
    
size_t GetBacktrace(unw_word_t * ipStack, int maxSize)
{
    return 0;
}
    
void PrintBacktrace()
{
        
}
    
UnwindProcMaps::UnwindProcMaps()
{

    //loop through all info and ad them to the list
    unw_map_cursor mapCursor;
    unw_map_cursor_create(&mapCursor,getpid());
    
    // must be called even after create
    unw_map_cursor_reset(&mapCursor);
    
    unw_map_t map;
    //int log = unw_map_local_cursor_valid(&mapCursor);
    while(unw_map_cursor_get_next(&mapCursor,&map) > 0)
    {
        processMap.push_back(map);
    }
    
    
    unw_map_cursor_destroy(&mapCursor);

}
bool UnwindProcMaps::FindLocalAddresInfo(unw_word_t addres, char ** libName, unw_word_t * addresInLib)
{
    for( auto map = processMap.begin();map != processMap.end();map++)
    {
        if(map->start < addres && map->end > addres)
        {
            *libName = map->path;
            *addresInLib = addres - map->start;
            return true;
        }
    }
    *addresInLib = addres;
    *libName = NULL;
    return false;
}
UnwindProcMaps::~UnwindProcMaps()
{
    for(auto m = processMap.begin();m != processMap.end();m++)
    {
        // path is allocated using strdup see unw_map_cursor_get_next source
        free(m->path);
    }
}
    
}
#endif //#if defined(__arm__)
#endif //#if defined(__DAVAENGINE_ANDROID__)