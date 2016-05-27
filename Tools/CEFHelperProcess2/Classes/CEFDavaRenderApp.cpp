#include "UI/Private/CEFInterprocessMessages.h"
#include "CEFDavaRenderApp.h"

CefRefPtr<CefRenderProcessHandler> CEFDavaRenderApp::GetRenderProcessHandler()
{
    return this;
}

bool CEFDavaRenderApp::OnBeforeNavigation(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> frame,
                                          CefRefPtr<CefRequest> request,
                                          NavigationType navigation_type,
                                          bool is_redirect)
{
    // Handle browser navigation
    // Only allowed URL can be loaded
    if (request->GetURL() == allowedUrl || request->GetURL() == "about:blank")
    {
        // URL can be allowed only once
        allowedUrl.clear();
        return false;
    }

    // Create a request message to browser process about disallowed URL loading
    DAVA::URLLoadingRequest requestMessage;
    requestMessage.url = request->GetURL();
    requestMessage.frameID = frame->GetIdentifier();
    requestMessage.navigation_type = static_cast<int>(navigation_type);
    requestMessage.is_redirect = is_redirect;

    CefRefPtr<CefProcessMessage> msg = DAVA::CreateUrlLoadingRequest(requestMessage);
    browser->SendProcessMessage(PID_BROWSER, msg);

    // Deny loading
    return true;
}

bool CEFDavaRenderApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                                CefProcessId source_process,
                                                CefRefPtr<CefProcessMessage> message)
{
    // CEFDavaRenderApp can process only URL loading permission messages from browser process
    if (source_process == PID_BROWSER && message->GetName() == DAVA::urlLoadingPermitMessageName)
    {
        DAVA::URLLoadingPermit permit;
        if (!DAVA::ParseUrlLoadingPermitMessage(message, permit))
        {
            return false;
        }

        allowedUrl = permit.url;
        CefRefPtr<CefFrame> frame = browser->GetFrame(permit.frameID);
        frame->LoadURL(permit.url);
        return true;
    }
    return false;
}

void CEFDavaRenderApp::OnBeforeCommandLineProcessing(const CefString& process_type,
                                                     CefRefPtr<CefCommandLine> command_line)
{
    // Disable GPU because
    command_line->AppendSwitch("disable-gpu");
    command_line->AppendSwitch("disable-gpu-compositing");
}
