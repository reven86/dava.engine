#if defined(ENABLE_CEF_WEBVIEW)

#include "UI/Private/CEFDavaResourceHandler.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{
CEFDavaResourceHandler::CEFDavaResourceHandler(const FilePath& path)
    : davaPath(path)
{
    DVASSERT_MSG(FileSystem::Instance()->IsFile(davaPath),
                 "CefDavaResourceHandler handles only exist files");
}

bool CEFDavaResourceHandler::ProcessRequest(CefRefPtr<CefRequest> request,
                                            CefRefPtr<CefCallback> callback)
{
    callback->Continue();
    return true;
}

void CEFDavaResourceHandler::GetResponseHeaders(CefRefPtr<CefResponse> response,
                                                int64& response_length,
                                                CefString& redirectUrl)
{
    // All is OK
    response->SetStatus(200);

    uint32 fileSize = 0;
    FileSystem::Instance()->GetFileSize(davaPath, fileSize);
    response_length = fileSize;
}

void CEFDavaResourceHandler::Cancel()
{
}

bool CEFDavaResourceHandler::ReadResponse(void* data_out,
                                          int bytes_to_read,
                                          int& bytes_read,
                                          CefRefPtr<CefCallback> callback)
{
    if (!file)
    {
        file.Set(File::Create(davaPath, File::OPEN | File::READ));
        if (!file)
        {
            DVASSERT_MSG(false, "Cannot open file");
            return false;
        }
    }

    uint32 bytesRealRead = file->Read(data_out, static_cast<uint32>(bytes_to_read));
    bytes_read = static_cast<int>(bytesRealRead);
    return true;
}

CefRefPtr<CefResourceHandler> CEFDavaResourceHandlerFactory::Create(CefRefPtr<CefBrowser> browser,
                                                                    CefRefPtr<CefFrame> frame,
                                                                    const CefString& scheme_name,
                                                                    CefRefPtr<CefRequest> request)
{
    String url = request->GetURL().ToString();
    // path after dava:/
    return new CEFDavaResourceHandler(FilePath(url.substr(6)));
}

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW