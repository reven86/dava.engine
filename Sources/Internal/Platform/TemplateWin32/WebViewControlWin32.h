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



#ifndef __WEBVIEWCONTROL_WIN32_H__
#define __WEBVIEWCONTROL_WIN32_H__

#include <MsHTML.h>
#include "Wininet.h" //for functions to delete cache entry and to end session

#include "UI/IWebViewControl.h"
#include "FileSystem/FilePath.h"

// Helper class to contain Web Browser.
interface IWebBrowser2;
namespace DAVA {

class WebBrowserContainer : IOleClientSite, IOleInPlaceSite
{
public:
	// Construction/destruction.
	WebBrowserContainer();
	virtual ~WebBrowserContainer();

	// Initialize the browser on the parent window.
	bool Initialize(HWND parentWindow);

	// Update the rect according to the parent window.
	void UpdateRect();

	// Open the URL.
	bool OpenUrl(const WCHAR* urlToOpen);

	bool LoadHtmlString(LPCTSTR pszHTMLContent);

	bool DeleteCookies(const String& targetUrl);

	String GetCookie(const String& url, const String& name);
	Map<String, String> GetCookies(const String& url);

	void ExecuteJScript(const String& targetScript);

    bool OpenFromBuffer(const String& buffer, const FilePath& basePath);
    bool DoOpenBuffer();

	// COM stuff;
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject);

	ULONG __stdcall AddRef() { return 1; /*one instance*/ }
	ULONG __stdcall Release() { return 1; /*one instance*/ }

	// IOleWindow interface.
	HRESULT __stdcall GetWindow(HWND *phwnd);
	HRESULT __stdcall ContextSensitiveHelp(BOOL) { return S_OK; }

	// IOleInPlaceSite interface.
	HRESULT __stdcall CanInPlaceActivate() { return S_OK; }
	HRESULT __stdcall OnInPlaceActivate() { return S_OK; }
	HRESULT __stdcall OnUIActivate() { return S_OK; }
	HRESULT __stdcall GetWindowContext(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc,
		LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo);

	HRESULT __stdcall Scroll(SIZE) { return S_OK; }
	HRESULT __stdcall OnUIDeactivate(BOOL) { return S_OK; }
	HRESULT __stdcall OnInPlaceDeactivate() { return S_OK; }
	HRESULT __stdcall DiscardUndoState() { return S_OK; }
	HRESULT __stdcall DeactivateAndUndo() { return S_OK; }
	HRESULT __stdcall OnPosRectChange(LPCRECT) { return S_OK; }

	// IOleClientSite
	HRESULT __stdcall SaveObject() { return S_OK; }
	HRESULT __stdcall GetMoniker(DWORD, DWORD, IMoniker **ppmk) { return *ppmk = NULL, E_NOTIMPL; }
	HRESULT __stdcall GetContainer(IOleContainer** ppContainer) { return *ppContainer=NULL, E_NOINTERFACE; }
	HRESULT __stdcall ShowObject() { return S_OK; }
	HRESULT __stdcall OnShowWindow(BOOL) { return S_OK; }
	HRESULT __stdcall RequestNewObjectLayout() { return E_NOTIMPL; }

	void SetDelegate(IUIWebViewDelegate *delegate, UIWebView* webView);
protected:
	// Parent window.
	HWND hwnd;

	// The browser itselt.
	IWebBrowser2* webBrowser;

	void* sink;

	HANDLE GetFirstCacheEntry(LPINTERNET_CACHE_ENTRY_INFO &cacheEntry, DWORD &size);
	bool GetNextCacheEntry(HANDLE cacheEnumHandle, LPINTERNET_CACHE_ENTRY_INFO &cacheEntry, DWORD &size);
	bool GetInternetCookies(const String& targetUrl, const String& name, LPTSTR &lpszData, DWORD &dwSize);

	bool openFromBufferQueued;
	String bufferToOpen; // temporary buffer
	FilePath bufferToOpenPath;
};

// Web View Control for Win32.
class WebViewControl : public IWebViewControl
{
public:
	WebViewControl();
	virtual ~WebViewControl();
	
	// Initialize the control.
	virtual void Initialize(const Rect& rect);
	
	// Open the URL requested.
	virtual void OpenURL(const String& urlToOpen);
	// Load html page from stringss
	virtual void LoadHtmlString(const WideString& htmlString);
	// Delete all cookies associated with target URL
	virtual void DeleteCookies(const String& targetUrl);
	// Get cookie for specific domain and name
	virtual String GetCookie(const String& url, const String& name);
	// Get the list of cookies for specific domain
	virtual Map<String, String> GetCookies(const String& url);
	// Execute javascript string in webview
	virtual String ExecuteJScript(const String& targetScript);
	
    void OpenFromBuffer(const String& string, const FilePath& basePath);

    // Size/pos/visibility changes.
	virtual void SetRect(const Rect& rect);
	virtual void SetVisible(bool isVisible, bool hierarchic);

	virtual void SetDelegate(IUIWebViewDelegate *delegate, UIWebView* webView);

protected:
	// Initialize the COM and create the browser container.
	bool InititalizeBrowserContainer();

	// Holder window for WebBrowser.
	HWND browserWindow;

	// Web Browser Container.
	WebBrowserContainer* browserContainer;
};

};
#endif //__WEBVIEWCONTROL_WIN32_H__
