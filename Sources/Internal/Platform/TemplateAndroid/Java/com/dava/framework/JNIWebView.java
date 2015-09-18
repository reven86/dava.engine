package com.dava.framework;

import java.util.HashMap;
import java.util.Map;

import android.content.Context;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Build;
import android.os.Handler;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.CookieManager;
import android.webkit.JsResult;
import android.webkit.WebChromeClient;
import android.webkit.WebView;
import android.widget.FrameLayout;

public class JNIWebView {
    private static final int MOVE_VIEW_OFFSCREEN_STEP = 10000;
    static final String TAG = "JNIWebView";
    private static final Paint paint = new Paint();

    public static class WebViewWrapper extends android.webkit.WebView {
        private InternalViewClientV14 client = null;
        private final static int MAX_DELAY = 1600;
        private final static int START_DELAY = 50;
        private int delay = 50; //50, 100, 200, 400, 800, 1600
        private String[] lastLoadData = null;
        
        public WebViewWrapper(Context context, InternalViewClientV14 client) {
            super(context);
            this.client = client;
            super.setWebViewClient(client);
        }

        InternalViewClientV14 getInternalViewClient() {
            return client;
        }
        
        public void updateViewRectPosition()
        {
            FrameLayout.LayoutParams params = (FrameLayout.LayoutParams)getLayoutParams();
            if(client.isRenderToTexture())
            {
                // hide view off screen if we render into texture
                if (params.leftMargin < JNIWebView.MOVE_VIEW_OFFSCREEN_STEP)
                {
                    params.leftMargin += JNIWebView.MOVE_VIEW_OFFSCREEN_STEP;
                }
            }
            else
            {
                if (params.leftMargin >= JNIWebView.MOVE_VIEW_OFFSCREEN_STEP)
                {
                    params.leftMargin -= JNIWebView.MOVE_VIEW_OFFSCREEN_STEP;
                }
            }
            setLayoutParams(params);
        }
        
        public void restoreVisibility()
        {
            client.setVisible(this, client.isVisible());
            // on unlock we have to call super.loadUrl() to show webview content 
            // we have to do that because user may lock phone before webView loading finishes
            // and sometimes already loaded content disappear after going to background
            if(lastLoadData != null) {
	            switch (lastLoadData.length) {
				case 1: // If length equals 1 then load content by URL (see WebViewWrapper.loadUrl(...))
					super.loadUrl(lastLoadData[0]);
					break;
				case 3: // If length equals 3 then load content from html string with mime type (see WebViewWrapper.loadData(...))
					super.loadData(lastLoadData[0], lastLoadData[1], lastLoadData[2]);
					break;
				case 5: // If length equals 5 then load content from string data with some base url (see WebViewWrapper.loadDataWithBaseURL(...))  
					super.loadDataWithBaseURL(lastLoadData[0], lastLoadData[1], lastLoadData[2], lastLoadData[3], lastLoadData[4]);
					break;
				default:
					Log.e(JNIConst.LOG_TAG, "Incorrect data to reload WebView content");
					break;
				}
            }
        }
        
        @Override
        public void loadUrl(String url)
        {
        	lastLoadData = new String[]{url}; // Using for reload lost content (see WebViewWrapper.restoreVisibility)
            super.loadUrl(url);
        }
        
        @Override
        public void loadData(String htmlString, String mimeType, String encoding)
        {
        	lastLoadData = new String[]{htmlString, mimeType, encoding}; // Using for reload lost content (see WebViewWrapper.restoreVisibility)
            super.loadData(htmlString, mimeType, encoding);
        }
        
        @Override
        public void loadDataWithBaseURL(String baseUrl, String data, String mimeType,
                String encoding, String failUrl)
        {
        	lastLoadData = new String[]{baseUrl, data, mimeType, encoding, failUrl}; // Using for reload lost content (see WebViewWrapper.restoreVisibility)
            super.loadDataWithBaseURL(baseUrl, data, mimeType, encoding, failUrl);
        }
        
        // Workaround to pass hardware-BACK key out from webview
        // to the top parent receiver
        @Override
        public boolean onKeyPreIme(int keyCode, KeyEvent event) {
            if (keyCode == KeyEvent.KEYCODE_BACK) {
                clearFocus();
                return false;
            }
            return super.onKeyPreIme(keyCode, event);
        }
        
        /*
         * HACK to get last known loaded url from any thread
         * cause standard getUrl method can't be called from chromium thread
         */
        public String getLastLoadedUrl()
        {
        	if(lastLoadData != null && lastLoadData.length > 0)
        	{
        		return lastLoadData[0];
        	}
        	return null;
        }
    }

    static Map<Integer, WebViewWrapper> views = new HashMap<Integer, WebViewWrapper>();

    

    public static void Initialize(final int id, final float x, final float y,
            final float dx, final float dy) {
        final JNIActivity activity = JNIActivity.GetActivity();
        if (null == activity || activity.GetIsPausing())
            return;

        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (views.containsKey(id)) {
                    Log.e(TAG, String.format(
                            "WebView with id %d already initialized", id));
                    return;
                }
                InternalViewClientV14 webViewClient = null;
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                    webViewClient = new InternalViewClientV21(id);
                } else {
                    webViewClient = new InternalViewClientV14(id);
                }
                
                final WebViewWrapper webView = new WebViewWrapper(activity, 
                        webViewClient);
                
                FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
                        new FrameLayout.MarginLayoutParams((int) (dx + 0.5f),
                                (int) (dy + 0.5f)));
                
                params.leftMargin = (int) (x + 0.5);
                params.topMargin = (int) (y + 0.5);
                params.width = (int) dx;
                params.height = (int) dy;
                
                webView.getSettings().setJavaScriptEnabled(true);
                webView.getSettings().setLoadWithOverviewMode(true);
                webView.getSettings().setUseWideViewPort(false);

                webView.setLayerType(WebView.LAYER_TYPE_SOFTWARE, null);
                
                webView.setDrawingCacheEnabled(true);
                
                webView.setWebChromeClient(new InternalWebClient(id));
                webView.setOnTouchListener(new View.OnTouchListener() {
                    @Override
                    public boolean onTouch(View v, MotionEvent event) {
                        switch (event.getAction()) {
                        case MotionEvent.ACTION_DOWN:
                        case MotionEvent.ACTION_UP:
                            if (!v.hasFocus()) {
                                v.requestFocus();
                            }
                            break;
                        }
                        return false;
                    }
                });

                activity.addContentView(webView, params);
                views.put(id, webView);
            }
        });
    }

    public static void Deinitialize(final int id) {
        final JNIActivity activity = JNIActivity.GetActivity();
        if (null == activity)
            return;

        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (!views.containsKey(id)) {
                    Log.e(TAG, String.format("Unknown view id %d", id));
                    return;
                }
                WebView view = views.get(id);
                views.remove(id);

                ((ViewGroup) view.getParent()).removeView(view);
            }
        });
    }

    public static void OpenURL(final int id, final String url) {
    	Log.e(TAG, "Try to open URL: " + url);
        final JNIActivity activity = JNIActivity.GetActivity();
        if (null == activity || activity.GetIsPausing())
            return;

        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (!views.containsKey(id)) {
                    Log.d(TAG, String.format("Unknown view id %d", id));
                    return;
                }
                Log.e(TAG, "Load URL: " + url);
                WebViewWrapper webView = views.get(id);
                webView.loadUrl(url);
            }
        });
    }
    
    
    // "recursive" call to render WebView into texture
    // with delays 50, 100, 200, 400 ... 1600 milliseconds
    // because first render on WebView may be incorrect (black/white)
    // after onFinishLoad(url)
    private static void refreshStaticTexture(final WebViewWrapper webView) {
        if (webView.getInternalViewClient().isRenderToTexture())
        {
            if (webView.delay < WebViewWrapper.MAX_DELAY)
            {
                final Handler handler = new Handler();
                Runnable runnable = new Runnable(){
                    @Override
                    public void run() {
                        webView.getInternalViewClient().renderToTexture(webView);
                        refreshStaticTexture(webView);
                    }
                };
                handler.postDelayed(runnable, webView.delay);
                webView.delay *= 2;
            } else
            {
                webView.delay = WebViewWrapper.START_DELAY;
            }
        }
    }

    public static void LoadHtmlString(final int id, final String htmlString) {
        final JNIActivity activity = JNIActivity.GetActivity();
        if (null == activity || activity.GetIsPausing())
            return;

        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (!views.containsKey(id)) {
                    Log.e(TAG, String.format("Unknown view id %d", id));
                    return;
                }
                final WebViewWrapper webView = views.get(id);
                webView.loadData(htmlString, "text/html", null);
                startRecursiveRefreshSequence(webView);
            }
        });
    }

    public static void OpenFromBuffer(final int id, final String data,
            final String baseUrl) {
        final JNIActivity activity = JNIActivity.GetActivity();
        if (null == activity || activity.GetIsPausing())
            return;

        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (!views.containsKey(id)) {
                    Log.e(TAG, String.format("Unknown view id %d", id));
                    return;
                }

                WebViewWrapper webView = views.get(id);
                webView.loadDataWithBaseURL(baseUrl, data, "text/html",
                        "utf-8", null);
                startRecursiveRefreshSequence(webView);
            }
        });
    }

    public static void ExecuteJScript(final int id, final String scriptString) {
        final JNIActivity activity = JNIActivity.GetActivity();
        if (null == activity || activity.GetIsPausing())
            return;
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (!views.containsKey(id)) {
                    Log.e(TAG, String.format("Unknown view id %d", id));
                    return;
                }
                final WebViewWrapper webView = views.get(id);

                String escapedJS = scriptString.replace("\"", "\\\"");

                String javaScript = "javascript:function call_back_func()"
                        + "{" 
                        + "    return \"\" + eval(\"" + escapedJS + "\");" 
                        + "}" 
                        + "javascript:alert(call_back_func())";

                webView.loadUrl(javaScript);

                startRecursiveRefreshSequence(webView);
            }
        });
    }

    public static void DeleteCookies(final String targetURL) {
        // The CookieSyncManager is used to synchronize the browser
        // cookie store between RAM and permanent storage.
        CookieManager cookieManager = CookieManager.getInstance();
        if (cookieManager.hasCookies()) {
            // Get cookies for specific URL and change their expiration date
            // This should force android system to remove these cookies
            String cookiestring = cookieManager.getCookie(targetURL);
            String[] cookies = cookiestring.split(";");

            for (int i = 0; i < cookies.length; i++) {
                String[] cookieparts = cookies[i].split("=");
                String cookieKey = cookieparts[0].trim();
                cookieManager.setCookie(targetURL, cookieKey
                        + "=; Expires=Mon, 31 Dec 2012 23:59:59 GMT");
            }

            cookieManager.flush();
        }
    }

    public static String GetCookie(final String targetURL,
            final String cookieName) {
        // The CookieSyncManager is used to synchronize the browser
        // cookie store between RAM and permanent storage.
        CookieManager cookieManager = CookieManager.getInstance();

        if (cookieManager.hasCookies()) {
            // Get cookies for specific URL
            String cookieString = cookieManager.getCookie(targetURL);
            String[] cookies = cookieString.split(";");

            for (int i = 0; i < cookies.length; i++) {
                String[] cookieparts = cookies[i].split("=");
                if (cookieparts[0].trim().compareTo(cookieName) == 0) {
                    return cookieparts[1];
                }
            }
        }

        return "";
    }

    public static Object[] GetCookies(final String targetURL) {
        // The CookieSyncManager is used to synchronize the browser cookie store
        // between RAM and permanent storage.
        CookieManager cookieManager = CookieManager.getInstance();
        // Get cookies for specific URL and put them into array
        String cookieString = cookieManager.getCookie(targetURL);
        String[] cookies = cookieString.split(";");
        return cookies;
    }

    public static void SetRect(final int id, final float x, final float y,
            final float dx, final float dy) {
        final JNIActivity activity = JNIActivity.GetActivity();
        if (null == activity || activity.GetIsPausing())
            return;

        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (!views.containsKey(id)) {
                    Log.e(TAG, String.format("Unknown view id %d", id));
                    return;
                }

                WebViewWrapper view = views.get(id);
                FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) view
                        .getLayoutParams();
                
                int xRoundPos = (int)(x + 0.5f);
                
                if (isRenderToTexture(id))
                {
                    params.leftMargin = xRoundPos + JNIWebView.MOVE_VIEW_OFFSCREEN_STEP;
                } else
                {
                    params.leftMargin = xRoundPos;
                }

                params.topMargin = (int) (y + 0.5f);
                params.width = (int) (dx /*+ 0.5f*/);
                params.height = (int) (dy/* + 0.5f*/);
                view.setLayoutParams(params);
            }
        });
    }

    public static void SetVisible(final int id, final boolean isVisible) {
        final JNIActivity activity = JNIActivity.GetActivity();
        if (null == activity || activity.GetIsPausing())
            return;

        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (!views.containsKey(id)) {
                    Log.e(TAG, String.format("Unknown view id %d", id));
                    return;
                }
                WebViewWrapper view = views.get(id);
                InternalViewClientV14 client = view.getInternalViewClient();
                client.setVisible(view, isVisible);
            }
        });
    }
    
    public static void WillDraw(final int id) {
        final JNIActivity activity = JNIActivity.GetActivity();
        if (null == activity || activity.GetIsPausing())
            return;

        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (!views.containsKey(id)) {
                    Log.e(TAG, String.format("Unknown view id %d", id));
                    return;
                }
                WebViewWrapper view = views.get(id);
                InternalViewClientV14 client = view.getInternalViewClient();
                client.updateVisible(view);
            }
        });
    }

    public static void setRenderToTexture(final int id,
            final boolean renderToTexture) {
        final JNIActivity activity = JNIActivity.GetActivity();
        if (null == activity || activity.GetIsPausing())
            return;

        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (!views.containsKey(id)) {
                    Log.e(TAG, String.format("Unknown view id %d", id));
                    return;
                }
                WebViewWrapper view = views.get(id);
                InternalViewClientV14 client = view.getInternalViewClient();
                client.setRenderToTexture(view, renderToTexture);
            }
        });
    }

    public static boolean isRenderToTexture(final int id) {
        if (!views.containsKey(id)) {
            Log.e(TAG, String.format("Unknown view id %d", id));
            return false;
        } else {
            WebViewWrapper view = views.get(id);
            InternalViewClientV14 client = view.getInternalViewClient();
            return client.isRenderToTexture();
        }
    }

    public static void SetBackgroundTransparency(final int id,
            final boolean enabled) {
        final JNIActivity activity = JNIActivity.GetActivity();
        if (null == activity || activity.GetIsPausing())
            return;

        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (!views.containsKey(id)) {
                    Log.e(TAG, String.format("Unknown view id %d", id));
                    return;
                }
                WebView view = views.get(id);
                view.setBackgroundColor((enabled ? Color.TRANSPARENT
                        : Color.WHITE));
            }
        });
    }

    static class InternalWebClient extends WebChromeClient {
        int id;

        InternalWebClient(int id) {
            this.id = id;
        }

        @Override
        public boolean onJsAlert(WebView view, String url, String message,
                JsResult result) {

            class jsCallback implements Runnable {
                int id;
                String result;

                jsCallback(int id, String result) {
                    this.id = id;
                    this.result = result;
                }

                @Override
                public void run() {
                    OnExecuteJScript(id, result);
                }
            }

            JNIActivity.GetActivity()
                    .PostEventToGL(new jsCallback(id, message));
            result.confirm();
            return true;
        }
    }

    public static void HideAllWebViews() {
        for (WebViewWrapper view: views.values()) {
            view.setVisibility(View.GONE);
        }
    }

    public static void ShowVisibleWebViews() {
        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                for (WebViewWrapper view : views.values()) {
                    view.restoreVisibility();
                    refreshStaticTexture(view);
                }
            }
        });
    }
    
    /**
     * Workaround for samsung tab 4 10.1 adreno 305 (Game Client)
     * on lock/unlock disappeared text fields
     */
    static protected void RelinkNativeControls() {
        for (WebViewWrapper view : views.values()) {
            ViewGroup viewGroup = (ViewGroup) view.getParent();
            viewGroup.removeView(view);
            JNIActivity.GetActivity().addContentView(view,
                    view.getLayoutParams());
        }
    }

    static native int OnUrlChange(int id, String url, boolean isRedirectedByMouseClick);

    static native int OnPageLoaded(int id, int[] pixels, int width,
            int height);

    static native void OnExecuteJScript(int id, String result);

    static void startRecursiveRefreshSequence(WebViewWrapper wrap) {
        wrap.delay = WebViewWrapper.START_DELAY; 
        refreshStaticTexture(wrap);
    }
}
