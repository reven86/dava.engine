#pragma once
#include <cef/include/cef_app.h>

class CEFDavaRenderApp : public CefApp, public CefRenderProcessHandler
{
    // CefApp interface implementation
    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override;

    // CefRenderProcessHandler interface implementation
    bool OnBeforeNavigation(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            CefRefPtr<CefRequest> request,
                            NavigationType navigation_type,
                            bool is_redirect) override;

    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                  CefProcessId source_process,
                                  CefRefPtr<CefProcessMessage> message) override;

    void OnBeforeCommandLineProcessing(const CefString& process_type,
                                       CefRefPtr<CefCommandLine> command_line) override;

    IMPLEMENT_REFCOUNTING(CEFDavaRenderApp)
    std::string allowedUrl;
};