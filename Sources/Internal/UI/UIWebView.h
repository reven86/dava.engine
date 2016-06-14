#ifndef __DAVAENGINE_UIWEBVIEW_H__
#define __DAVAENGINE_UIWEBVIEW_H__

#include "UIControl.h"
#include "IWebViewControl.h"

namespace DAVA
{
// The purpose of UIWebView class is displaying embedded Web Page Controls.
class UIWebView : public UIControl
{
public:
    // Data detector types. May be a combination of several flags.
    enum eDataDetectorType
    {
        DATA_DETECTOR_NONE = 0x00,
        DATA_DETECTOR_PHONE_NUMBERS = 0x01,
        DATA_DETECTOR_LINKS = 0x02,
        DATA_DETECTOR_ADDRESSES = 0x04,
        DATA_DETECTOR_CALENDAR_EVENTS = 0x08,
        DATA_DETECTOR_ALL = 0xFF
    };

    UIWebView(const Rect& rect = Rect());

protected:
    virtual ~UIWebView();

public:
    // Open the URL.
    void OpenFile(const FilePath& path);
    void OpenURL(const String& urlToOpen);
    // Load html page
    void LoadHtmlString(const WideString& htmlString);
    // Delete all cookies for target URL
    void DeleteCookies(const String& targetUrl);
    // Get cookie for specific domain and name
    String GetCookie(const String& targetUrl, const String& name) const;
    // Get the list of cookies for specific domain
    Map<String, String> GetCookies(const String& targetUrl) const;
    // Perform Java script
    // if you need return data from javascript just
    // return JSON string you can parse it in c++
    // with yaml parser, call back with JSON will come to
    // IUIWebViewDelegate::OnExecuteJScript
    void ExecuteJScript(const String& scriptString);

    void OpenFromBuffer(const String& string, const FilePath& basePath);

    // Overloaded virtual methods.
    void SetPosition(const Vector2& position) override;
    void SetSize(const Vector2& newSize) override;

    // Page scale property change
    void SetScalesPageToFit(bool isScalesToFit);

    UIWebView* Clone() override;
    void CopyDataFrom(UIControl* srcControl) override;

    void SystemDraw(const UIGeometricData& geometricData) override;
    void Draw(const UIGeometricData& geometricData) override;
    void Input(UIEvent* currentInput) override;

    void Update(float32 timeElapsed) override;

protected:
    void OnVisible() override;
    void OnInvisible() override;
    void OnActive() override;

public:
    void SetRenderToTexture(bool value);
    bool IsRenderToTexture() const;

    void SetDelegate(IUIWebViewDelegate* delegate);
    void SetBackgroundTransparency(bool enabled);

    // Enable/disable bounces.
    void SetBounces(bool value);
    bool GetBounces() const;
    void SetGestures(bool value);

    // Set the data detector types.
    void SetDataDetectorTypes(int32 value);
    int32 GetDataDetectorTypes() const;

protected:
    void SetNativeControlVisible(bool isVisible);
    bool GetNativeControlVisible() const;
    // Set the visibility of native control.
    void UpdateNativeControlVisible(bool value);

    // Update the rect of the web view control.
    void UpdateControlRect();

    // Platform-specific implementation of the Web View Control.
    std::unique_ptr<IWebViewControl> webViewControl;

private:
    bool isNativeControlVisible;
    int32 dataDetectorTypes;

public:
    INTROSPECTION_EXTEND(UIWebView, UIControl,
                         PROPERTY("dataDetectorTypes", InspDesc("Data detector types", GlobalEnumMap<eDataDetectorType>::Instance(), InspDesc::T_FLAGS), GetDataDetectorTypes, SetDataDetectorTypes, I_SAVE | I_VIEW | I_EDIT));
};
};

#endif /* defined(__DAVAENGINE_UIWEBVIEW_H__) */
