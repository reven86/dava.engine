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

#include "shlobj.h" // for function to get path to cookies folder
#include "Wininet.h" //for functions to delete cache entry and to end session

extern _ATL_FUNC_INFO BeforeNavigate2Info;
_ATL_FUNC_INFO BeforeNavigate2Info = {CC_STDCALL, VT_EMPTY, 7, {VT_DISPATCH,VT_BYREF|VT_VARIANT,VT_BYREF|VT_VARIANT,VT_BYREF|VT_VARIANT,VT_BYREF|VT_VARIANT,VT_BYREF|VT_VARIANT,VT_BYREF|VT_BOOL}};

extern _ATL_FUNC_INFO DocumentCompleteInfo;
_ATL_FUNC_INFO DocumentCompleteInfo =  {CC_STDCALL,VT_EMPTY,2,{VT_DISPATCH,VT_BYREF | VT_VARIANT}};

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
	if (!this->webBrowser)
	{
		return false;
	}

	BSTR url = SysAllocString(urlToOpen);
	VARIANT empty = {0};
	VariantInit(&empty);

	this->webBrowser->Navigate(url, &empty, &empty, &empty, &empty);
	SysFreeString(url);

	return true;
}

bool WebBrowserContainer::LoadHtmlString(LPCTSTR pszHTMLContent)
{
	if (!this->webBrowser || !pszHTMLContent)
	{
		return false;
	}
	// Initialize html document
	this->webBrowser->Navigate( L"about:blank", NULL, NULL, NULL, NULL); 

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


bool WebBrowserContainer::DeleteApplicationCookies(const WCHAR* targetUrl)
{
	if (!this->webBrowser)
	{
		return false;
	}

	LPTSTR url = SysAllocString(targetUrl);

    HANDLE hCacheEnumHandle  = NULL;
    LPINTERNET_CACHE_ENTRY_INFO lpCacheEntry = NULL;
    DWORD dwSize = 4096; // initial buffer size

    // Enable initial buffer size for cache entry structure. 
    lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) new char[dwSize];
    lpCacheEntry->dwStructSize = dwSize;
    
    hCacheEnumHandle = FindFirstUrlCacheEntry(L"cookie:", lpCacheEntry, &dwSize);
    
    // First, obtain handle to internet cache with FindFirstUrlCacheEntry 
    // for later use with FindNextUrlCacheEntry. 
    if (hCacheEnumHandle != NULL) 
    {
        // Delete only cookies for specific site
        if ((lpCacheEntry->CacheEntryType & COOKIE_CACHE_ENTRY))
        {
			LPTSTR searchStr = StrStr(lpCacheEntry->lpszSourceUrlName, url);
			// If cach entry url is not equat to target - do not remove that cookie
			if (searchStr != NULL)
			{
				DeleteUrlCacheEntry(lpCacheEntry->lpszSourceUrlName);
			}
        }
    }
    else
    {
        switch (GetLastError())
        {
            case ERROR_INSUFFICIENT_BUFFER:
			{
				lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) new char[dwSize];
				lpCacheEntry->dwStructSize = dwSize;

				// Repeat first step search with adjusted buffer, exit if not
				// found again.
				hCacheEnumHandle = FindFirstUrlCacheEntry(NULL, lpCacheEntry, 
												  &dwSize);
				if (hCacheEnumHandle != NULL) 
				{
					// Delete only cookies for specific site
					if ((lpCacheEntry->CacheEntryType & COOKIE_CACHE_ENTRY))
					{            
						LPTSTR searchStr = StrStr(lpCacheEntry->lpszSourceUrlName, url);
						// If cach entry url is not equat to target - do not remove that cookie
						if (searchStr != NULL)
						{
							DeleteUrlCacheEntry(lpCacheEntry->lpszSourceUrlName);
						}
					}
				}
				else
				{
					// FindFirstUrlCacheEntry fails again, return.
					SysFreeString(url);
					return false; 
				}
			}
			default:
				FindCloseUrlCache(hCacheEnumHandle);
				SysFreeString(url);
				return false;
		}
    }
    
    // Next, use hCacheEnumHandle obtained from the previous step to delete 
    // subsequent items of cache.
    do 
    {
         // Notice that return values of FindNextUrlCacheEntry (BOOL) and 
         // FindFirstUrlCacheEntry (HANDLE) are different.
		 if (FindNextUrlCacheEntry(hCacheEnumHandle, lpCacheEntry, &dwSize))
         {
			// Delete only cookies for specific site
            if ((lpCacheEntry->CacheEntryType & COOKIE_CACHE_ENTRY))
            {            
				LPTSTR searchStr = StrStr(lpCacheEntry->lpszSourceUrlName, url);
				// If cach entry url is not equat to target - do not remove that cookie
				if (searchStr != NULL)
				{
					DeleteUrlCacheEntry(lpCacheEntry->lpszSourceUrlName);
				}
            }
         }
         else
         {
            switch(GetLastError())
            {
                case ERROR_INSUFFICIENT_BUFFER:
				{
					lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) new char[dwSize];
					lpCacheEntry->dwStructSize = dwSize;

					// Repeat next step search with adjusted buffer, exit if 
					// error comes up again ((in practice one buffer's size 
					// adustment is always OK).
					if (FindNextUrlCacheEntry(hCacheEnumHandle, lpCacheEntry, &dwSize)) 
					{
						// Delete only cookies for specific site
						if ((lpCacheEntry->CacheEntryType & COOKIE_CACHE_ENTRY))
						{            
							LPTSTR searchStr = StrStr(lpCacheEntry->lpszSourceUrlName, url);
							// If cach entry url is not equat to target - do not remove that cookie
							if (searchStr != NULL)
							{
								DeleteUrlCacheEntry(lpCacheEntry->lpszSourceUrlName);
							}
						}
						break;
					}
					else
					{
						// FindFirstUrlCacheEntry fails again, return.
						FindCloseUrlCache(hCacheEnumHandle);
						SysFreeString(url);
						return false; 
					}
				}
                break;

                case ERROR_NO_MORE_ITEMS:
					FindCloseUrlCache(hCacheEnumHandle);
					SysFreeString(url);
					return true; 

                default:
					FindCloseUrlCache(hCacheEnumHandle);
					SysFreeString(url);
					return false;
            }
        }
    } while (true);
}

void WebBrowserContainer::UpdateRect()
{
	IOleInPlaceObject* oleInPlaceObject = NULL;
	HRESULT hRes = this->webBrowser->QueryInterface(IID_IOleInPlaceObject, (void**)&oleInPlaceObject);
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

	this->browserContainer= new WebBrowserContainer();
	return browserContainer->Initialize(this->browserWindow);
}

void WebViewControl::OpenURL(const String& urlToOpen)
{
	if (this->browserContainer)
	{
		this->browserContainer->OpenUrl(StringToWString(urlToOpen.c_str()).c_str());
	}
}

void WebViewControl::LoadHtmlString(const WideString& htmlString)
{
	if (this->browserContainer)
	{
		this->browserContainer->LoadHtmlString(htmlString.c_str());
	}
}

void WebViewControl::DeleteApplicationCookies(const String& targetUrl)
{
	if (this->browserContainer)
	{
		this->browserContainer->DeleteApplicationCookies(StringToWString(targetUrl.c_str()).c_str());
	}
}

void WebViewControl::SetVisible(bool isVisible, bool /*hierarchic*/)
{
	if (this->browserWindow != 0)
	{
		::ShowWindow(this->browserWindow, isVisible);
	}
}

void WebViewControl::SetRect(const Rect& rect)
{
	if (this->browserWindow == 0)
	{
		return;
	}

	RECT browserRect = {0};
	::GetWindowRect(this->browserWindow, &browserRect);

	browserRect.left = (LONG)(rect.x * DAVA::Core::GetVirtualToPhysicalFactor());
	browserRect.top  = (LONG)(rect.y * DAVA::Core::GetVirtualToPhysicalFactor());
	browserRect.left  += (LONG)Core::Instance()->GetPhysicalDrawOffset().x;
	browserRect.top += (LONG)Core::Instance()->GetPhysicalDrawOffset().y;

	browserRect.right = (LONG)(browserRect.left + rect.dx * Core::GetVirtualToPhysicalFactor());
	browserRect.bottom = (LONG)(browserRect.top + rect.dy * Core::GetVirtualToPhysicalFactor());

	::SetWindowPos(browserWindow, NULL, browserRect.left, browserRect.top,
		browserRect.right - browserRect.left, browserRect.bottom - browserRect.top, SWP_NOZORDER );

	if (this->browserContainer)
	{
		this->browserContainer->UpdateRect();
	}
}
