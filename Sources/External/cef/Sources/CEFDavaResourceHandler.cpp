#include <cef/include/cef_parser.h>

#include "CEFDavaResourceHandler.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Utils/Utils.h"

namespace DAVA
{
CEFDavaResourceHandler::CEFDavaResourceHandler(const FilePath& path)
    : davaPath(path)
{
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
    FileSystem* fs = FileSystem::Instance();
    uint32 fileSize = 0;

    if (!fs->Exists(davaPath))
    {
        // Not found
        response->SetStatus(404);
        return;
    }
    else if (!fs->IsFile(davaPath) || !fs->GetFileSize(davaPath, fileSize))
    {
        // Bad request
        response->SetStatus(400);
        return;
    }

    // All is OK
    response->SetStatus(200);
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

String CEFDavaResourceHandler::FilePathToDavaUrl(const FilePath& path)
{
    Vector<String> tokens;
    Split(path.GetStringValue(), "/\\", tokens);
    for (String& token : tokens)
    {
        token = CefURIEncode(token, false);
    }

    String result;
    Merge(tokens, '/', result);

    if (result.back() != '/' && path.IsDirectoryPathname())
    {
        result += '/';
    }

    return "dava:/" + result;
}

FilePath CEFDavaResourceHandler::DavaUrlToFilePath(const String& url)
{
    int rule = UU_NORMAL | UU_SPACES | UU_URL_SPECIAL_CHARS | UU_CONTROL_CHARS;
    // path after dava:/
    return CefURIDecode(url.substr(6), true, static_cast<cef_uri_unescape_rule_t>(rule)).ToString();
}

CefRefPtr<CefResourceHandler> CEFDavaResourceHandlerFactory::Create(CefRefPtr<CefBrowser> browser,
                                                                    CefRefPtr<CefFrame> frame,
                                                                    const CefString& scheme_name,
                                                                    CefRefPtr<CefRequest> request)
{
    String url = request->GetURL().ToString();
    FilePath decodedUrl = CEFDavaResourceHandler::DavaUrlToFilePath(url);

    return new CEFDavaResourceHandler(decodedUrl);
}

} // namespace DAVA
