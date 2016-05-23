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
    void SetUpdateRate(uint32 n);

private:
    RefPtr<class CEFControllerImpl> cefControllerImpl;
};

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW
