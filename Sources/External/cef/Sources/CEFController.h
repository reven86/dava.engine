#pragma once
#if defined(ENABLE_CEF_WEBVIEW)

#include <cef/include/internal/cef_ptr.h>
#include "Base/RefPtr.h"

class CefClient;

namespace DAVA
{
class CEFController
{
public:
    CEFController(const CefRefPtr<CefClient>& client);
    ~CEFController();

    static bool IsCEFInitialized();
    void Update();

    static uint32 GetCacheLimitSize();
    static void SetCacheLimitSize(uint32 size);

private:
    RefPtr<class CEFControllerImpl> cefControllerImpl;
};

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW
