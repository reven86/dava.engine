#pragma once
#if defined(ENABLE_CEF_WEBVIEW)

#include "Base/RefPtr.h"

namespace DAVA
{
class CEFController
{
public:
    CEFController();
    ~CEFController();

    void Update();

    static uint32 GetCacheLimitSize();
    static void SetCacheLimitSize(uint32 size);

private:
    RefPtr<class CEFControllerImpl> cefControllerImpl;
};

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW
