#include "TArc/Utils/Private/CrashDumpHandler.h"
#include "TArc/Utils/DebuggerDetection.h"

#include <Base/Platform.h>
#if defined(__DAVAENGINE_WIN32__)
    #include <imagehlp.h>
#endif

namespace DAVA
{
namespace TArc
{
#if defined(__DAVAENGINE_WIN32__)
PTOP_LEVEL_EXCEPTION_FILTER prevFilter = nullptr;
void MakeMinidump(EXCEPTION_POINTERS* e)
{
    HMODULE hDbgHelp = LoadLibraryA("dbghelp");
    if (hDbgHelp == nullptr)
    {
        return;
    }

    auto pMiniDumpWriteDump = (decltype(&MiniDumpWriteDump))GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
    if (pMiniDumpWriteDump == nullptr)
    {
        return;
    }

    char name[1024];
    {
        char* nameEnd = name + GetModuleFileNameA(GetModuleHandleA(0), name, MAX_PATH);
        SYSTEMTIME t;
        GetSystemTime(&t);
        wsprintfA(nameEnd - strlen(".exe"),
                  "_%4d%02d%02d_%02d%02d.dmp",
                  t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute);
    }

    HANDLE hFile = CreateFileA(name, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
        exceptionInfo.ThreadId = GetCurrentThreadId();
        exceptionInfo.ExceptionPointers = e;
        exceptionInfo.ClientPointers = FALSE;

        pMiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        MINIDUMP_TYPE(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory),
        e ? &exceptionInfo : nullptr,
        nullptr,
        nullptr);

        CloseHandle(hFile);
    }
}

LONG CALLBACK UnhandledHandler(EXCEPTION_POINTERS* e)
{
    MakeMinidump(e);
    SetUnhandledExceptionFilter(prevFilter);
    return EXCEPTION_CONTINUE_SEARCH;
}

void InitCrashDumpHandler()
{
    if (!IsDebuggerPresent())
    {
        prevFilter = SetUnhandledExceptionFilter(&UnhandledHandler);
    }
}
#else
void InitCrashDumpHandler()
{
}
#endif

} // namespace TArc
} // namespace DAVA
