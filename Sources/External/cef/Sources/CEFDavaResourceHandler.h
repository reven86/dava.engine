#pragma once

#include <cef/include/cef_scheme.h>
#include "Base/RefPtr.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
class CEFDavaResourceHandler : public CefResourceHandler
{
    IMPLEMENT_REFCOUNTING(CEFDavaResourceHandler);

public:
    CEFDavaResourceHandler(const FilePath& path);

    static String FilePathToDavaUrl(const FilePath& path);
    static FilePath DavaUrlToFilePath(const String& url);

private:
    bool ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) override;

    void GetResponseHeaders(CefRefPtr<CefResponse> response,
                            int64& response_length,
                            CefString& redirectUrl) override;

    void Cancel() override;
    bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read,
                      CefRefPtr<CefCallback> callback) override;

    FilePath davaPath;
    RefPtr<class File> file;
};

class CEFDavaResourceHandlerFactory : public CefSchemeHandlerFactory
{
    IMPLEMENT_REFCOUNTING(CEFDavaResourceHandlerFactory);

    CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         const CefString& scheme_name,
                                         CefRefPtr<CefRequest> request) override;
};

} // namespace DAVA
