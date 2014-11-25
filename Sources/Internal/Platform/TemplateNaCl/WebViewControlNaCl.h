#ifndef __DAVAENGINE_WEBVIEWCONTROL_NACL_H__
#define __DAVAENGINE_WEBVIEWCONTROL_NACL_H__

#include "../../UI/IWebViewControl.h"


namespace DAVA {
class FilePath;    
// Web View Control - NACL version.
class WebViewControl : public IWebViewControl
{
public:
	WebViewControl(){};
	~WebViewControl(){};
	
	// Initialize the control.
	void Initialize(const Rect& rect){};
	
	// Open the URL requested.
	void OpenURL(const String& urlToOpen){};

    void OpenFromBuffer(const String& string, const FilePath& basePath){};
    
	// Size/pos/visibility changes.
	void SetRect(const Rect& rect){};
	void SetVisible(bool isVisible, bool hierarchic){};

	void SetDelegate(DAVA::IUIWebViewDelegate *delegate, DAVA::UIWebView* webView){};
	void SetBackgroundTransparency(bool enabled){};

	// Bounces control.
	bool GetBounces() const{ return false;};
	void SetBounces(bool value){};
    void SetGestures(bool value){};

/*protected:
	// Get the scale divider for Retina devices.
	float GetScaleDivider();

	//A pointer to iOS WebView.
	void* webViewPtr;
	
	// A pointer to the WebView delegate.
	void* webViewDelegatePtr;

	void* webViewURLDelegatePtr;

    void *rightSwipeGesturePtr;
    void *leftSwipeGesturePtr;

    
	Map<void*, bool> subviewVisibilityMap;

	void HideSubviewImages(void* view);
	void RestoreSubviewImages();
    bool gesturesEnabled;*/
};

};
#endif