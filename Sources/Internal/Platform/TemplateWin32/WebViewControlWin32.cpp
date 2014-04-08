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



#include "WebViewControlWin32.h"
#include "CorePlatformWin32.h"
using namespace DAVA;

#include <atlbase.h>
#include <atlcom.h>
#include <ExDisp.h>
#include <ExDispid.h>
#include "Utils/Utils.h"

extern _ATL_FUNC_INFO BeforeNavigate2Info;
_ATL_FUNC_INFO BeforeNavigate2Info = {CC_STDCALL, VT_EMPTY, 7, {VT_DISPATCH,VT_BYREF|VT_VARIANT,VT_BYREF|VT_VARIANT,VT_BYREF|VT_VARIANT,VT_BYREF|VT_VARIANT,VT_BYREF|VT_VARIANT,VT_BYREF|VT_BOOL}};

extern _ATL_FUNC_INFO DocumentCompleteInfo;
_ATL_FUNC_INFO DocumentCompleteInfo =  {CC_STDCALL,VT_EMPTY,2,{VT_DISPATCH,VT_BYREF | VT_VARIANT}};
namespace DAVA 
{

struct EventSink : public IDispEventImpl<1, EventSink, &DIID_DWebBrowserEvents2>
{
private:
	DAVA::IUIWebViewDelegate* delegate;
	DAVA::UIWebView* webView;

public:
	EventSink()
	{
		delegate = NULL;
		webView = NULL;
	};

	void SetDelegate(DAVA::IUIWebViewDelegate *delegate, DAVA::UIWebView* webView)
	{
		if (delegate && webView)
		{
			this->delegate = delegate;
			this->webView = webView;
		}
	}

	void  __stdcall DocumentComplete(IDispatch* pDisp, VARIANT* URL)
	{
		if (delegate && webView)
		{
			delegate->PageLoaded(webView);
		}
	}

	void __stdcall BeforeNavigate2(IDispatch* pDisp, VARIANT* URL, VARIANT* Flags,
								   VARIANT* TargetFrameName, VARIANT* PostData,
								   VARIANT* Headers, VARIANT_BOOL* Cancel)
	{
		bool process = true;

		if (delegate && webView)
		{
			BSTR bstr = V_BSTR(URL);
			int32 len = SysStringLen(bstr) + 1;
			char* str = new char[len];
			WideCharToMultiByte(CP_ACP, 0, bstr, -1, str, len, NULL, NULL);
			String s = str;
			delete[] str;
			bool isRedirectedByMouseClick  = Flags->intVal == navHyperlink ;
			IUIWebViewDelegate::eAction action = delegate->URLChanged(webView, s, isRedirectedByMouseClick);

			switch (action)
			{
				case IUIWebViewDelegate::PROCESS_IN_WEBVIEW:
					Logger::FrameworkDebug("PROCESS_IN_WEBVIEW");
					break;

				case IUIWebViewDelegate::PROCESS_IN_SYSTEM_BROWSER:
					Logger::FrameworkDebug("PROCESS_IN_SYSTEM_BROWSER");
					process = false;
					ShellExecute(NULL, L"open", bstr, NULL, NULL, SW_SHOWNORMAL);
					break;

				case IUIWebViewDelegate::NO_PROCESS:
					Logger::FrameworkDebug("NO_PROCESS");

				default:
					process = false;
					break;
			}
		}

		*Cancel = process ? VARIANT_FALSE : VARIANT_TRUE;
	}

	BEGIN_SINK_MAP(EventSink)
		SINK_ENTRY_INFO(1, DIID_DWebBrowserEvents2, DISPID_BEFORENAVIGATE2, BeforeNavigate2, &BeforeNavigate2Info)
		SINK_ENTRY_INFO(1, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE, DocumentComplete, &DocumentCompleteInfo)
	END_SINK_MAP()
};


WebBrowserContainer::WebBrowserContainer() :
	hwnd(0),
	webBrowser(NULL)
{
}

WebBrowserContainer::~WebBrowserContainer()
{
	EventSink* s = (EventSink*)sink;
	s->DispEventUnadvise(webBrowser, &DIID_DWebBrowserEvents2);
	delete s;

	if (webBrowser)
	{
		webBrowser->Release();
		webBrowser = NULL;
	}
}

void WebBrowserContainer::SetDelegate(DAVA::IUIWebViewDelegate *delegate, DAVA::UIWebView* webView)
{
	EventSink* s = (EventSink*)sink;
	s->SetDelegate(delegate, webView);
}

bool WebBrowserContainer::Initialize(HWND parentWindow)
{
	this->hwnd = parentWindow;

	IOleObject* oleObject = NULL;
	HRESULT hRes = CoCreateInstance(CLSID_WebBrowser, NULL, CLSCTX_INPROC, IID_IOleObject, (void**)&oleObject);
	if (FAILED(hRes))
	{
		Logger::Error("WebBrowserContainer::Inititalize(), CoCreateInstance(CLSID_WebBrowser) failed!, error code %i", hRes);
		return false;
	}

	hRes = oleObject->SetClientSite(this);
	if (FAILED(hRes))
	{
		Logger::Error("WebBrowserContainer::Inititalize(), IOleObject::SetClientSite() failed!, error code %i", hRes);
		oleObject->Release();
		return false;
	}

	// Activating the container.
	RECT rect = {0};
	GetClientRect(hwnd, &rect);
	hRes = oleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, this, 0, this->hwnd, &rect);
	if (FAILED(hRes))
	{
		Logger::Error("WebBrowserContainer::InititalizeBrowserContainer(), IOleObject::DoVerb() failed!, error code %i", hRes);
		oleObject->Release();
		return false;
	}

	// Prepare the browser itself.
	hRes = oleObject->QueryInterface(IID_IWebBrowser2, (void**)&this->webBrowser);
	if (FAILED(hRes))
	{
		Logger::Error("WebViewControl::InititalizeBrowserContainer(), IOleObject::QueryInterface(IID_IWebBrowser2) failed!, error code %i", hRes);
		oleObject->Release();
		return false;
	}

	sink = new EventSink();
	EventSink* s = (EventSink*)sink;
	hRes = s->DispEventAdvise(webBrowser, &DIID_DWebBrowserEvents2);
	if (FAILED(hRes))
	{
		Logger::Error("WebViewControl::InititalizeBrowserContainer(), EventSink::DispEventAdvise(&DIID_DWebBrowserEvents2) failed!, error code %i", hRes);
		return false;
	}

	// Initialization is OK.
	oleObject->Release();
	return true;
}

HRESULT __stdcall WebBrowserContainer::QueryInterface(REFIID riid, void** ppvObject)
{
	if( !ppvObject )
	{
		return E_POINTER;
	}

	if( riid==IID_IUnknown || riid==IID_IOleWindow || riid==IID_IOleInPlaceSite )
	{
		return *ppvObject = (void*)static_cast<IOleInPlaceSite*>(this), S_OK;
	}

	if( riid==IID_IOleClientSite )
	{
		return *ppvObject = (void*)static_cast<IOleClientSite*>(this), S_OK;
	}

	*ppvObject = NULL;
	return E_NOINTERFACE;
}

HRESULT __stdcall WebBrowserContainer::GetWindow(HWND *phwnd)
{
	if (!phwnd)
	{
		return E_INVALIDARG;
	}

	*phwnd = this->hwnd;
	return S_OK;
}

HRESULT __stdcall WebBrowserContainer::GetWindowContext( IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc,
		LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	if( !(ppFrame && ppDoc && lprcPosRect && lprcClipRect && lpFrameInfo) )
	{
		return E_INVALIDARG;
	}

	*ppFrame = NULL;
	*ppDoc = NULL;

	GetClientRect( this->hwnd, lprcPosRect );
	GetClientRect( this->hwnd, lprcClipRect );

	lpFrameInfo->fMDIApp = false;
	lpFrameInfo->hwndFrame = this->hwnd;
	lpFrameInfo->haccel = 0;
	lpFrameInfo->cAccelEntries = 0;

	return S_OK;
}

bool WebBrowserContainer::OpenUrl(const WCHAR* urlToOpen)
{
	if (!webBrowser)
	{
		return false;
	}

	BSTR url = SysAllocString(urlToOpen);
	VARIANT empty = {0};
	VariantInit(&empty);

	webBrowser->Navigate(url, &empty, &empty, &empty, &empty);
	SysFreeString(url);

	return true;
}

bool WebBrowserContainer::LoadHtmlString(LPCTSTR pszHTMLContent)
{
	if (!webBrowser || !pszHTMLContent)
	{
		return false;
	}
	// Initialize html document
	webBrowser->Navigate( L"about:blank", NULL, NULL, NULL, NULL); 

	IDispatch * m_pDoc;
	IStream * pStream = NULL;
	IPersistStreamInit * pPSI = NULL;
	HGLOBAL hHTMLContent;
	HRESULT hr;
	bool bResult = false;

	// allocate global memory to copy the HTML content to
	hHTMLContent = ::GlobalAlloc( GPTR, ( ::_tcslen( pszHTMLContent ) + 1 ) * sizeof(TCHAR) );
	if (!hHTMLContent)
		return false;

	::_tcscpy( (TCHAR *) hHTMLContent, pszHTMLContent );

	// create a stream object based on the HTML content
	hr = ::CreateStreamOnHGlobal( hHTMLContent, TRUE, &pStream );
	if (SUCCEEDED(hr))
	{

		IDispatch * pDisp = NULL;

		// get the document's IDispatch*
		hr = this->webBrowser->get_Document( &pDisp );
		if (SUCCEEDED(hr))
		{
			m_pDoc = pDisp;
		}
		else
		{
			return false;
		}

		// request the IPersistStreamInit interface
		hr = m_pDoc->QueryInterface( IID_IPersistStreamInit, (void **) &pPSI );

		if (SUCCEEDED(hr))
		{
			// initialize the persist stream object
			hr = pPSI->InitNew();

			if (SUCCEEDED(hr))
			{
				// load the data into it
				hr = pPSI->Load( pStream );

				if (SUCCEEDED(hr))
				{
					bResult = true;
				}
			}

			pPSI->Release();
		}

		// implicitly calls ::GlobalFree to free the global memory
		pStream->Release();
	}

	return bResult;
}

String WebBrowserContainer::GetCookie(const String& targetUrl, const String& name)
{
	if (!webBrowser)
	{
		return String();
	}

	LPTSTR lpszData = NULL;   // buffer to hold the cookie data
	DWORD dwSize = 4096; // Initial size of buffer		
	String retCookie;

	if (GetInternetCookies(targetUrl, name, lpszData, dwSize))
	{
		retCookie = WStringToString(lpszData);

		Vector<String> cookie;
		Split(retCookie, "=", cookie);
		// Get only cookie value
		if (cookie.size() == 2)
		{
			retCookie = cookie[1];
		}
	}
	else
	{
		delete [] lpszData;
	}

	return retCookie;
}

Map<String, String> WebBrowserContainer::GetCookies(const String& targetUrl)
{
	if (!webBrowser)
	{
		return Map<String, String>();
	}

	LPTSTR lpszData = NULL;   // buffer to hold the cookie data
	DWORD dwSize = 4096; // Initial size of buffer
	Map<String, String> cookiesMap;

	if (GetInternetCookies(targetUrl, "", lpszData, dwSize))
	{
		String cookiesString = WStringToString(lpszData);
		// Split cookies string into vector - each value corresponds to one cookie name-value pair
		Vector<String> cookiesVector;
		Split(cookiesString, ";", cookiesVector);

		for (uint32 i = 0; i < (int32)cookiesVector.size(); ++i)
		{
			Vector<String> cookie;
			Split(cookiesVector[i], "=", cookie);
			// Add cookie to resulting map
			if (cookie.size() == 2)
			{
				String cookieName = cookie[0];
				// Remove all spaces in cookie name
				cookieName.erase(std::remove(cookieName.begin(), cookieName.end(), ' '), cookieName.end());
				cookiesMap[cookieName] = cookie[1];
			}
		}
	}
	else
	{
		delete [] lpszData;
	}

	return cookiesMap;
}

bool WebBrowserContainer::GetInternetCookies(const String& targetUrl, const String& name, LPTSTR &lpszData, DWORD &dwSize)
{
	// Setup initial cache entry size
	lpszData = new TCHAR[dwSize];

	BOOL bResult = InternetGetCookieEx(StringToWString(targetUrl).c_str(), 
											name.empty() ? NULL : StringToWString(name).c_str(),
											lpszData,
											&dwSize,
											INTERNET_COOKIE_HTTPONLY,
											NULL);
	// Encrease buffer if its size is not enough
	if (!bResult && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
	{
		delete [] lpszData;
		lpszData = new TCHAR[dwSize];

		bResult = InternetGetCookieEx(StringToWString(targetUrl).c_str(), 
											name.empty() ? NULL : StringToWString(name).c_str(),
											lpszData,
											&dwSize,
											INTERNET_COOKIE_HTTPONLY,
											NULL);
	}

	return bResult ? true : false;
}

bool WebBrowserContainer::DeleteCookies(const String& targetUrl)
{
	if (!webBrowser)
	{
		return false;
	}

	WideString url = StringToWString(targetUrl);
	LPINTERNET_CACHE_ENTRY_INFO cacheEntry = NULL;  
 	// Initial buffer size
    DWORD  entrySize = 4096;     
	HANDLE cacheEnumHandle = NULL; 

	// Get first entry and enum handle
	cacheEnumHandle = GetFirstCacheEntry(cacheEntry, entrySize);
	if (!cacheEnumHandle)
	{	
		delete [] cacheEntry; 
		return false;
	}

	bool bResult = false;
    bool bDone = false;

	do
	{
		// Delete only cookies for specific site
		if ((cacheEntry->CacheEntryType & COOKIE_CACHE_ENTRY))
		{            
			// If cache entry url do have target URL - do not remove that cookie
			if (StrStr(cacheEntry->lpszSourceUrlName, url.c_str()))
			{
				DeleteUrlCacheEntry(cacheEntry->lpszSourceUrlName);
			}
		}
		// Try to get next cache entry - in case when we can't do it - exit the cycle
		if (!GetNextCacheEntry(cacheEnumHandle, cacheEntry, entrySize))
		{
			// ERROR_NO_MORE_FILES means search is finished successfully.
			bResult = (GetLastError() == ERROR_NO_MORE_ITEMS);
			bDone = true;            
		}
	} while (!bDone);

	// clean up		
	delete [] cacheEntry; 
	FindCloseUrlCache(cacheEnumHandle);  

    return bResult;
}

HANDLE WebBrowserContainer::GetFirstCacheEntry(LPINTERNET_CACHE_ENTRY_INFO &cacheEntry, DWORD &size)
{
	// Setup initial cache entry size
	cacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) new char[size];
    cacheEntry->dwStructSize = size;

	// Create handle for cache entries with tag "Cookie:"
	HANDLE cacheEnumHandle = FindFirstUrlCacheEntry(L"cookie:", cacheEntry, &size);
	// If handle was not created with error - ERROR_INSUFFICIENT_BUFFER - enlarge cacheEntry size and try again
	if ((cacheEnumHandle == NULL) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
	{
		delete [] cacheEntry;            
        cacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) new char[size];
		cacheEntry->dwStructSize = size;

		cacheEnumHandle = FindFirstUrlCacheEntry(L"cookie:", cacheEntry, &size);
	}

	return cacheEnumHandle;
}

bool WebBrowserContainer::GetNextCacheEntry(HANDLE cacheEnumHandle, LPINTERNET_CACHE_ENTRY_INFO &cacheEntry, DWORD &size)
{
	BOOL bResult = FindNextUrlCacheEntry(cacheEnumHandle, cacheEntry, &size);
	// If buffer size was not enough - give more memory for cacheEntry
	if ((!bResult) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
	{
		delete [] cacheEntry;            
        cacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) new char[size];
		cacheEntry->dwStructSize = size;

		bResult = FindNextUrlCacheEntry(cacheEnumHandle, cacheEntry, &size);
	}

	return bResult ? true : false;
}

void WebBrowserContainer::ExecuteJScript(const String& targetScript)
{
	IDispatch *m_pDisp = NULL; 
    webBrowser->get_Document(&m_pDisp);
	DVASSERT(m_pDisp);

	IHTMLDocument2* pHtmDoc2 = NULL;
	m_pDisp->QueryInterface(IID_IHTMLDocument2, (LPVOID*)&pHtmDoc2);
	DVASSERT(pHtmDoc2);

	//pHtmDoc2->Invoke
	
	IHTMLWindow2* pHtmWin2 = NULL;
	pHtmDoc2->get_parentWindow(&pHtmWin2);
	DVASSERT(pHtmWin2); 

	IDispatch *disp;
	pHtmWin2->QueryInterface(&disp);
	DVASSERT(disp);
	/*IDispatchEx *dispEx;
	pHtmWin2->QueryInterface(&dispEx);
	DVASSERT(dispEx);*/

	IDispatch* pScript = 0;
	pHtmDoc2->get_Script(&pScript);

    DISPID dispidEval = -1;
	VARIANT var;
	OLECHAR FAR* sMethod = L"eval";
    HRESULT hr2 = webBrowser->GetIDsOfNames(IID_NULL, &sMethod, 1, LOCALE_SYSTEM_DEFAULT,&dispidEval);//GetDispID(L"eval", fdexNameCaseSensitive, &dispidEval);
	
    DISPPARAMS dispparams = {0};
    dispparams.cArgs      = 1;
    dispparams.rgvarg     = new VARIANT[dispparams.cArgs];
    dispparams.cNamedArgs = 0;

	WideString wStr = L"alert('Hello!');"; 
	BSTR tmpBSTR = SysAllocString(wStr.c_str());
    dispparams.rgvarg[0].bstrVal = tmpBSTR;
    dispparams.rgvarg[0].vt = VT_BSTR;

	HRESULT hr = disp->Invoke(dispidEval, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD,  &dispparams, &var, NULL, NULL);

//	WideString sJSCode=L"document.title";
//	BSTR bstrJSCode = SysAllocString(sJSCode.c_str());//.AllocSysString();
//	WideString JSl = L"JavaScript";
//	BSTR bstrJSl = SysAllocString(JSl.c_str());
//	VARIANT var;

	//HRESULT hr = pHtmWin2->execScript(bstrJSCode, bstrJSl, &var);

	//::SysFreeString(bstrJSCode);
	//::SysFreeString(bstrJSl);

	if (m_pDisp) m_pDisp->Release(); 
	if (pHtmWin2) pHtmWin2->Release();
	if (pHtmDoc2) pHtmDoc2->Release();
}

void WebBrowserContainer::UpdateRect()
{
	IOleInPlaceObject* oleInPlaceObject = NULL;
	HRESULT hRes = webBrowser->QueryInterface(IID_IOleInPlaceObject, (void**)&oleInPlaceObject);
	if (FAILED(hRes))
	{
		Logger::Error("WebBrowserContainer::SetSize(), IOleObject::QueryInterface(IID_IOleInPlaceObject) failed!, error code %i", hRes);
		return;
	}

	// Update the browser window according to the holder window.
	RECT rect = {0};
	GetClientRect(this->hwnd, &rect);

	hRes = oleInPlaceObject->SetObjectRects(&rect, &rect);
	if (FAILED(hRes))
	{
		Logger::Error("WebBrowserContainer::SetSize(), IOleObject::SetObjectRects() failed!, error code %i", hRes);
		return;
	}

	oleInPlaceObject->Release();
}

WebViewControl::WebViewControl()
{
	browserWindow = 0;
	browserContainer = NULL;
}

WebViewControl::~WebViewControl()
{
	if (browserWindow != 0)
	{
		::DestroyWindow(browserWindow);
	}

	SafeDelete(browserContainer);
}

void WebViewControl::SetDelegate(DAVA::IUIWebViewDelegate *delegate, DAVA::UIWebView* webView)
{
	browserContainer->SetDelegate(delegate, webView);
}

void WebViewControl::Initialize(const Rect& rect)
{
	CoreWin32Platform *core = dynamic_cast<CoreWin32Platform *>(CoreWin32Platform::Instance());
	if (core == NULL)
	{
		return;
	}

	// Create the browser holder window.
	browserWindow = ::CreateWindowEx(0, L"Static", L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		0, 0, 0, 0, core->hWindow, NULL, core->hInstance, NULL);
	SetRect(rect);

	// Initialize the browser itself.
	InititalizeBrowserContainer();
}

bool WebViewControl::InititalizeBrowserContainer()
{
	HRESULT hRes = ::CoInitialize(NULL);
	if (FAILED(hRes))
	{
		Logger::Error("WebViewControl::InititalizeBrowserContainer(), CoInitialize() failed!");
		return false;
	}

	browserContainer= new WebBrowserContainer();
	return browserContainer->Initialize(this->browserWindow);
}

void WebViewControl::OpenURL(const String& urlToOpen)
{
	if (browserContainer)
	{
		browserContainer->OpenUrl(StringToWString(urlToOpen.c_str()).c_str());
	}
}

void WebViewControl::LoadHtmlString(const WideString& htmlString)
{
	if (browserContainer)
	{
		browserContainer->LoadHtmlString(htmlString.c_str());
	}
}

void WebViewControl::DeleteCookies(const String& targetUrl)
{
	if (browserContainer)
	{
		browserContainer->DeleteCookies(targetUrl);
	}
}

String WebViewControl::GetCookie(const String& targetUrl, const String& name)
{
	if (browserContainer)
	{
		return browserContainer->GetCookie(targetUrl, name);
	}

	return String();
}

Map<String, String> WebViewControl::GetCookies(const String& targetUrl)
{
	if (browserContainer)
	{
		return browserContainer->GetCookies(targetUrl);
	}

	return Map<String, String>();
}

String WebViewControl::ExecuteJScript(const String& targetScript)
{
	if (browserContainer)
	{
		browserContainer->ExecuteJScript(targetScript);
	}

	return String();
}

void WebViewControl::OpenFromBuffer(const String& string, const FilePath& basePath)
{
    // TODO
}

void WebViewControl::SetVisible(bool isVisible, bool /*hierarchic*/)
{
	if (browserWindow != 0)
	{
		::ShowWindow(browserWindow, isVisible);
	}
}

void WebViewControl::SetRect(const Rect& rect)
{
	if (browserWindow == 0)
	{
		return;
	}

	RECT browserRect = {0};
	::GetWindowRect(browserWindow, &browserRect);

	browserRect.left = (LONG)(rect.x * DAVA::Core::GetVirtualToPhysicalFactor());
	browserRect.top  = (LONG)(rect.y * DAVA::Core::GetVirtualToPhysicalFactor());
	browserRect.left  += (LONG)Core::Instance()->GetPhysicalDrawOffset().x;
	browserRect.top += (LONG)Core::Instance()->GetPhysicalDrawOffset().y;

	browserRect.right = (LONG)(browserRect.left + rect.dx * Core::GetVirtualToPhysicalFactor());
	browserRect.bottom = (LONG)(browserRect.top + rect.dy * Core::GetVirtualToPhysicalFactor());

	::SetWindowPos(browserWindow, NULL, browserRect.left, browserRect.top,
		browserRect.right - browserRect.left, browserRect.bottom - browserRect.top, SWP_NOZORDER );

	if (browserContainer)
	{
		browserContainer->UpdateRect();
	}
}

}
