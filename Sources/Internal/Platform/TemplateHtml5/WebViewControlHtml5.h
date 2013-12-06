#ifndef __DAVAENGINE_WEBVIEWCONTROL_HTML5_H__
#define __DAVAENGINE_WEBVIEWCONTROL_HTML5_H__

#include "../../UI/IWebViewControl.h"

namespace DAVA {
    
// Web View Control - Flash version.
class WebViewControl : public IWebViewControl
{
public:
	virtual ~WebViewControl() {};
	
	// Initialize the control.
	virtual void Initialize(const Rect& rect) {};
	
	// Open the URL requested.
	virtual void OpenURL(const String& urlToOpen){};
	
	// Size/pos/visibility changes.
	virtual void SetRect(const Rect& rect) {};
	virtual void SetVisible(bool isVisible, bool hierarchic) {};
	
	virtual void SetDelegate(DAVA::IUIWebViewDelegate *delegate, DAVA::UIWebView* webView) {};
};
};
#endif