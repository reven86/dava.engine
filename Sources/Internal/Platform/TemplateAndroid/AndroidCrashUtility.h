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


size_t GetBacktrace(unw_context_t * context, unw_word_t * outIpStack, int maxSize);

size_t GetBacktrace(unw_word_t * outIpStack, int maxSize);

void PrintBacktrace();
 
//uses added to libunwind functionality to create memory map of
// process
class UnwindProcMaps
{
public:
    UnwindProcMaps();
    bool  FindLocalAddresInfo(unw_word_t addres, char ** libName, unw_word_t * addresInLib);
    ~UnwindProcMaps();
    
private:
    std::list<unw_map_t> processMap;
    
};

}
#endif //#if defined(__arm__)
#endif //#if defined(__DAVAENGINE_ANDROID__)
#endif /* #ifndef __DAVAENGINE_ANDROID_CRASH_UTILITY_H__ */