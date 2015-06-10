
#include "Library.h"
#include "Debug.h"

#if NV_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif



void * nvLoadLibrary(const char * name)
{
#if NV_OS_WIN32
#  ifndef POSH_OS_WIN_STORE
	return (void *)LoadLibraryExA( name, NULL, 0 );
#  else
    return NULL;
#  endif
#else
	return dlopen(name, RTLD_LAZY);
#endif
}

void nvUnloadLibrary(void * handle)
{
	nvDebugCheck(handle != NULL);
#if NV_OS_WIN32
#  ifndef POSH_OS_WIN_STORE
    FreeLibrary((HMODULE)handle);
#  endif
#else
	dlclose(handle);
#endif
}

void * nvBindSymbol(void * handle, const char * symbol)
{
#if NV_OS_WIN32
	return (void *)GetProcAddress((HMODULE)handle, symbol);
#else
	return (void *)dlsym(handle, symbol);
#endif	
}
