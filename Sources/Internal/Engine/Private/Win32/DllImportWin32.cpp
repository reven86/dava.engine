#include "Engine/Private/Win32/DllImportWin32.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN32__)

namespace DAVA
{
namespace Private
{
// clang-format off
BOOL(WINAPI* DllImport::fnEnableMouseInPointer)(BOOL fEnable);
BOOL(WINAPI* DllImport::fnIsMouseInPointerEnabled)();
BOOL(WINAPI* DllImport::fnGetPointerInfo)(UINT32 pointerId, POINTER_INFO* pointerInfo);
HRESULT(STDAPICALLTYPE* DllImport::fnGetDpiForMonitor)(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT* dpiX, UINT* dpiY);
// clang-format on

void DllImport::Initialize()
{
    HMODULE huser32 = ::LoadLibraryW(L"user32");
    if (huser32 != nullptr)
    {
        fnEnableMouseInPointer = reinterpret_cast<decltype(fnEnableMouseInPointer)>(::GetProcAddress(huser32, "EnableMouseInPointer"));
        fnIsMouseInPointerEnabled = reinterpret_cast<decltype(fnIsMouseInPointerEnabled)>(::GetProcAddress(huser32, "IsMouseInPointerEnabled"));
        fnGetPointerInfo = reinterpret_cast<decltype(fnGetPointerInfo)>(::GetProcAddress(huser32, "GetPointerInfo"));
    }

    HMODULE hshcore = ::LoadLibraryW(L"shcore");
    if (hshcore != nullptr)
    {
        fnGetDpiForMonitor = reinterpret_cast<decltype(fnGetDpiForMonitor)>(::GetProcAddress(hshcore, "GetDpiForMonitor"));
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
