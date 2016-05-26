#pragma once
#include <cef/include/cef_process_message.h>

namespace DAVA
{
//--------------------------------------------------------------------------------------------------
// Definition of url loading request message
//--------------------------------------------------------------------------------------------------
// Render process sends it to browser process in case of loading url
// Browser process should make a decision about permission of url loading
// and send the message to render process
// Message protocol:
// String: url
// Int: low int of frame ID
// Int: high int of frame ID
// Int: navigation type
// Bool: is redirect
const char urlLoadingRequestMessageName[] = "DAVA_Engine_URL_Loading_Request";
struct URLLoadingRequest
{
    std::string url;
    int64 frameID;
    int navigation_type;
    bool is_redirect;
};

inline CefRefPtr<CefProcessMessage> CreateUrlLoadingRequest(const URLLoadingRequest& request);
inline bool ParseUrlLoadingRequest(CefRefPtr<CefProcessMessage> msg, URLLoadingRequest& request);

//--------------------------------------------------------------------------------------------------
// Definition of url loading permit message
//--------------------------------------------------------------------------------------------------
// Browser process send it to render process in case of url loading permission
// Message protocol:
// String: url
// Int: low int of frame ID
// Int: high int of frame ID
const char urlLoadingPermitMessageName[] = "DAVA_Engine_URL_Loading_Permit";
struct URLLoadingPermit
{
    std::string url;
    int64 frameID;
};

inline CefRefPtr<CefProcessMessage> CreateUrlLoadingPermitMessage(const URLLoadingPermit& permit);
inline bool ParseUrlLoadingPermitMessage(CefRefPtr<CefProcessMessage> msg, URLLoadingPermit& permit);

//--------------------------------------------------------------------------------------------------
// Functions implementation
//--------------------------------------------------------------------------------------------------
// Helper macros for splitting and combining the int64 frame ID value
#define CEF_MAKE_INT64(int_low, int_high) \
    ((int64)(((int)(int_low)) | ((int64)((int)(int_high))) << 32))
#define CEF_LOW_INT(int64_val) ((int)(int64_val))
#define CEF_HIGH_INT(int64_val) ((int)(((int64)(int64_val) >> 32) & 0xFFFFFFFFL))

inline CefRefPtr<CefProcessMessage> CreateUrlLoadingRequest(const URLLoadingRequest& request)
{
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(urlLoadingRequestMessageName);
    CefRefPtr<CefListValue> args = msg->GetArgumentList();

    args->SetString(0, request.url);
    args->SetInt(1, CEF_LOW_INT(request.frameID));
    args->SetInt(2, CEF_HIGH_INT(request.frameID));
    args->SetInt(3, request.navigation_type);
    args->SetBool(4, request.is_redirect);

    return msg;
}

inline bool ParseUrlLoadingRequest(CefRefPtr<CefProcessMessage> msg, URLLoadingRequest& request)
{
    if (msg->GetName() != urlLoadingRequestMessageName)
    {
        return false;
    }

    CefRefPtr<CefListValue> args = msg->GetArgumentList();

    request.url = args->GetString(0);
    if (request.url.empty())
    {
        return false;
    }

    request.frameID = CEF_MAKE_INT64(args->GetInt(1), args->GetInt(2));
    request.navigation_type = args->GetInt(3);
    request.is_redirect = args->GetBool(4);

    return true;
}

inline CefRefPtr<CefProcessMessage> CreateUrlLoadingPermitMessage(const URLLoadingPermit& permit)
{
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(urlLoadingPermitMessageName);
    CefRefPtr<CefListValue> args = msg->GetArgumentList();

    args->SetString(0, permit.url);
    args->SetInt(1, CEF_LOW_INT(permit.frameID));
    args->SetInt(2, CEF_HIGH_INT(permit.frameID));

    return msg;
}

inline bool ParseUrlLoadingPermitMessage(CefRefPtr<CefProcessMessage> msg, URLLoadingPermit& permit)
{
    if (msg->GetName() != urlLoadingPermitMessageName)
    {
        return false;
    }

    CefRefPtr<CefListValue> args = msg->GetArgumentList();

    permit.url = args->GetString(0);
    if (permit.url.empty())
    {
        return false;
    }

    permit.frameID = CEF_MAKE_INT64(args->GetInt(1), args->GetInt(2));
    return true;
}

#undef CEF_MAKE_INT64
#undef CEF_LOW_INT
#undef CEF_HIGH_INT

} // namespace DAVA
