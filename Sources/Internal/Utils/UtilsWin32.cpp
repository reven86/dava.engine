#include "Utils.h"

#if defined(__DAVAENGINE_WINDOWS__)
#include <Objbase.h>
#include <AtlConv.h>

#if defined(__DAVAENGINE_WIN32__)
#include <ShellAPI.h>
#elif defined(__DAVAENGINE_WIN_UAP__)
#include <ppltasks.h>
#endif

namespace DAVA
{
String GenerateGUID()
{
    //create new GUID
    GUID guid;
    if (FAILED(CoCreateGuid(&guid)))
        return "";

    //get string representation of GUID
    Array<OLECHAR, 64> guidString{};
    ::StringFromGUID2(guid, guidString.data(), static_cast<int>(guidString.size()));

    //convert to normal string
    USES_CONVERSION;
    return OLE2CA(guidString.data());
}

#if defined(__DAVAENGINE_WIN_UAP__)

void OpenURL(const String& url)
{
    auto platform_string = ref new Platform::String(StringToWString(url).c_str());
    auto uri = ref new Windows::Foundation::Uri(platform_string);
    concurrency::task<bool> launchUriOperation(Windows::System::Launcher::LaunchUriAsync(uri));
    launchUriOperation.get();
}

#elif defined(__DAVAENGINE_WIN32__)

void OpenURL(const String& url)
{
    WideString urlWide = StringToWString(url);
    ShellExecute(NULL, L"open", urlWide.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

#endif //  __DAVAENGINE_WIN32__

} //  namespace DAVA

#endif //  __DAVAENGINE_WINDOWS__