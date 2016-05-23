#pragma once

#if defined(ENABLE_CEF_WEBVIEW)

#include <cef/include/cef_scheme.h>
#include "Base/RefPtr.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
class CEFDavaResourceHandler : public CefResourceHandler
{
public:
    CEFDavaResourceHandler(const FilePath& path);

    bool ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) override;

    void GetResponseHeaders(CefRefPtr<CefResponse> response,
                            int64& response_length,
                            CefString& redirectUrl) override;

    void Cancel() override;
    bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read,
                      CefRefPtr<CefCallback> callback) override;

private:
    IMPLEMENT_REFCOUNTING(CEFDavaResourceHandler);
    FilePath davaPath;
    RefPtr<class File> file;
};

class CEFDavaResourceHandlerFactory : public CefSchemeHandlerFactory
{
    CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         const CefString& scheme_name,
                                         CefRefPtr<CefRequest> request) override;
    IMPLEMENT_REFCOUNTING(CEFDavaResourceHandlerFactory);
};

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW