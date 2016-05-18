/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#if defined(ENABLE_CEF_WEBVIEW)

#include "UI/Private/CEFDavaResourceHandler.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{
CEFDavaResourceHandler::CEFDavaResourceHandler(const String& url)
    : path(url.substr(6)) // path after dava:/
{
    DVASSERT_MSG(FileSystem::Instance()->IsFile(path),
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
    FileSystem::Instance()->GetFileSize(path, fileSize);
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
        file.Set(File::Create(path, File::OPEN | File::READ));
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
    return new CEFDavaResourceHandler(url);
}

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW