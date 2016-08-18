package com.dava.engine;

import android.view.KeyEvent;
import android.widget.FrameLayout;
import android.content.Context;
import android.graphics.Color;
import android.webkit.WebView;
import android.webkit.CookieManager;

public final class DavaWebView
{
    private long webviewBackendPointer = 0;
    private DavaSurfaceView surfaceView = null;
    private CustomWebView nativeWebView = null;

    // Properties that have been set in DAVA::Engine thread and waiting to apply to WebView
    private WebViewProperties properties = new WebViewProperties();
    // Properties that reflect WebView current properties
    private WebViewProperties curProperties = new WebViewProperties();

    //private boolean renderToTexture = false;
    //private boolean visible = false;                   // Native control initially is invisible
    //private float x;
    //private float y;
    //private float width;
    //private float height;

    private final class eNavigateTo
    {
        static final int NAVIGATE_NONE = 0;
        static final int NAVIGATE_OPEN_URL = 1;
        static final int NAVIGATE_LOAD_HTML = 2;
        static final int NAVIGATE_OPEN_BUFFER = 3;
    };

    private class WebViewProperties
    {
        WebViewProperties() {}
        WebViewProperties(WebViewProperties other)
        {
            x = other.x;
            y = other.y;
            width = other.width;
            height = other.height;
            visible = other.visible;
            renderToTexture = other.renderToTexture;
            backgroundTransparency = other.backgroundTransparency;
            urlOrHtml = other.urlOrHtml;
            basePath = other.basePath;
            jsScript = other.jsScript;

            createNew = other.createNew;
            anyPropertyChanged = other.anyPropertyChanged;
            rectChanged = other.rectChanged;
            visibleChanged = other.visibleChanged;
            renderToTextureChanged = other.renderToTextureChanged;
            backgroundTransparencyChanged = other.backgroundTransparencyChanged;
            execJavaScript = other.execJavaScript;
            navigateTo = other.navigateTo;
        }

        float x;
        float y;
        float width;
        float height;
        boolean visible;
        boolean renderToTexture;
        boolean backgroundTransparency;
        String urlOrHtml;
        String basePath;
        String jsScript;

        boolean createNew;
        boolean anyPropertyChanged;
        boolean rectChanged;
        boolean visibleChanged;
        boolean renderToTextureChanged;
        boolean backgroundTransparencyChanged;
        boolean execJavaScript;
        int navigateTo = eNavigateTo.NAVIGATE_NONE;

        void clearChangedFlags()
        {
            createNew = false;
            anyPropertyChanged = false;
            rectChanged = false;
            visibleChanged = false;
            renderToTextureChanged = false;
            backgroundTransparencyChanged = false;
            execJavaScript = false;
            navigateTo = eNavigateTo.NAVIGATE_NONE;
        }
    }

    private static class CustomWebView extends WebView
    {
        CustomWebView(Context context)
        {
            super(context);
        }

        // Override onKeyPreIme to pass hardware back key to upper-level receiver
        @Override
        public boolean onKeyPreIme(int keyCode, KeyEvent event)
        {
            if (keyCode == KeyEvent.KEYCODE_BACK)
            {
                if (event.getAction() == KeyEvent.ACTION_DOWN)
                {
                    // skip ACTION_DOWN event
                }
                else
                {
                    clearFocus();
                }
                return true;
            }
            return super.onKeyPreIme(keyCode, event);
        }
    }

    public DavaWebView(DavaSurfaceView view, long backendPointer)
    {
        webviewBackendPointer = backendPointer;
        surfaceView = view;

        //DavaWebViewClient webViewClient = new DavaWebViewClient();
        //nativeWebView = new CustomWebView(context);
        //nativeWebView.setWebViewClient(webViewClient);
        //nativeWebView.setWebChromeClient(new InternalWebClient(id));
    }

    void openURL(String url)
    {
        properties.urlOrHtml = url;
        properties.navigateTo = eNavigateTo.NAVIGATE_OPEN_URL;
        properties.anyPropertyChanged = true;
    }

    void loadHtmlString(String htmlString)
    {
        properties.urlOrHtml = htmlString;
        properties.navigateTo = eNavigateTo.NAVIGATE_LOAD_HTML;
        properties.anyPropertyChanged = true;
    }

    void openFromBuffer(String htmlString, String basePath)
    {
        properties.urlOrHtml = htmlString;
        properties.basePath = basePath;
        properties.navigateTo = eNavigateTo.NAVIGATE_OPEN_BUFFER;
        properties.anyPropertyChanged = true;
    }

    void executeJScript(String script)
    {
        properties.jsScript = script;
        properties.execJavaScript = true;
        properties.anyPropertyChanged = true;
    }

    void setRect(float x, float y, float width, float height)
    {
        boolean changed = properties.x != x || properties.y != y ||
                          properties.width != width || properties.height != height; 
        if (changed)
        {
            properties.x = x;
            properties.y = y;
            properties.width = width;
            properties.height = height;
            properties.rectChanged = true;
            properties.anyPropertyChanged = true;
        }
    }

    void setVisible(boolean visible)
    {
        if (properties.visible != visible)
        {
            properties.visible = visible;
            properties.visibleChanged = true;
            properties.anyPropertyChanged = true;
            if (!visible)
            {
                // Immediately hide native control
                DavaActivity.commandHandler().post(new Runnable() {
                    @Override public void run()
                    {
                        setNativePositionAndSize(curProperties.x,
                                                 curProperties.y,
                                                 curProperties.width,
                                                 curProperties.height,
                                                 true);
                    }
                });
            }
        }
    }

    void setBackgroundTransparency(boolean enabled)
    {
        if (properties.backgroundTransparency != enabled)
        {
            properties.backgroundTransparency = enabled;
            properties.backgroundTransparencyChanged = true;
            properties.anyPropertyChanged = true;
        }
    }

    void setRenderToTexture(boolean value)
    {
        if (properties.renderToTexture != value)
        {
            properties.renderToTexture = value;
            properties.renderToTextureChanged = true;
            properties.anyPropertyChanged = true;
        }
    }

    boolean isRenderToTexture()
    {
        return properties.renderToTexture;
    }

    void update()
    {
        if (properties.createNew || properties.anyPropertyChanged)
        {
            WebViewProperties props = new WebViewProperties(properties);
            // TODO: apply properties on ui thread
            //processProperties(props);
            properties.clearChangedFlags();
        }
    }

    void processProperties(WebViewProperties props)
    {
        if (props.createNew)
        {
            createNativeControl();
        }
        if (props.anyPropertyChanged)
        {
            applyChangedProperties(props);
            //if (renderToTexture && (props.rectChanged || props.backgroundTransparencyChanged || props.renderToTextureChanged))
            //{
            //    RenderToTexture();
            //}
        }
    }

    void createNativeControl()
    {
        nativeWebView = new CustomWebView(DavaActivity.instance());

        nativeWebView.getSettings().setJavaScriptEnabled(true);
        nativeWebView.getSettings().setLoadWithOverviewMode(true);
        nativeWebView.getSettings().setUseWideViewPort(false);

        nativeWebView.setLayerType(WebView.LAYER_TYPE_SOFTWARE, null);
        nativeWebView.setDrawingCacheEnabled(true);

        //nativeWebView.setWebChromeClient(new InternalWebClient(id));
    }

    void applyChangedProperties(WebViewProperties props)
    {
        if (props.rectChanged)
        {
            curProperties.x = props.x;
            curProperties.y = props.y;
            curProperties.width = props.width;
            curProperties.height = props.height;
        }
        if (props.renderToTextureChanged)
            curProperties.renderToTexture = props.renderToTexture;
        if (props.visibleChanged)
            curProperties.visible = props.visible;
        if (props.rectChanged || props.visibleChanged || props.renderToTextureChanged)
        {
            setNativePositionAndSize(curProperties.x,
                                     curProperties.y,
                                     curProperties.width,
                                     curProperties.height,
                                     curProperties.renderToTexture || !curProperties.visible);
        }
        if (props.backgroundTransparencyChanged)
            setNativeBackgroundTransparency(props.backgroundTransparency);
        if (props.navigateTo != eNavigateTo.NAVIGATE_NONE)
            nativeNavigateTo(props);
        if (props.execJavaScript)
            nativeExecuteJavaScript(props.jsScript);
    }

    void nativeNavigateTo(WebViewProperties props)
    {
        if (eNavigateTo.NAVIGATE_OPEN_URL == props.navigateTo)
        {
            nativeWebView.loadUrl(props.urlOrHtml);
        }
        else if (eNavigateTo.NAVIGATE_LOAD_HTML == props.navigateTo)
        {
            nativeWebView.loadData(props.urlOrHtml, "text/html", null);
        }
        else if (eNavigateTo.NAVIGATE_OPEN_BUFFER == props.navigateTo)
        {
            nativeWebView.loadDataWithBaseURL(props.basePath, props.urlOrHtml, "text/html", "utf-8", null);
        }
    }

    void nativeExecuteJavaScript(String jsScript)
    {
        String escapedJS = jsScript.replace("\"", "\\\"");
        String javaScript = "javascript:function call_back_func() {" +
                            "    return \"\" + eval(\"" + escapedJS + "\"); }" + 
                            "javascript:alert(call_back_func())";
        nativeWebView.loadUrl(javaScript);
    }

    void setNativePositionAndSize(float x, float y, float width, float height, boolean offScreen)
    {
        float xOffset = 0.0f;
        float yOffset = 0.0f;
        if (offScreen)
        {
            // Move control very far offscreen
            xOffset = x + width + 10000.0f;
            yOffset = y + height + 10000.0f;
        }

        FrameLayout.LayoutParams params = (FrameLayout.LayoutParams)nativeWebView.getLayoutParams();
        params.leftMargin = (int)xOffset;
        params.topMargin = (int)yOffset;
        params.width = Math.max(0, (int)width);
        params.height = Math.max(0, (int)height);
        nativeWebView.setLayoutParams(params);
    }

    void setNativeBackgroundTransparency(boolean enabled)
    {
        nativeWebView.setBackgroundColor(enabled ? Color.TRANSPARENT : Color.WHITE);
    }

    public static void DeleteCookies(String url)
    {
        CookieManager cookieManager = CookieManager.getInstance();
        if (cookieManager.hasCookies())
        {
            // Get cookies for specific URL and change their expiration date to some date in past.
            // This should force android system to remove these cookies
            String[] cookies = cookieManager.getCookie(url).split(";");
            for (int i = 0; i < cookies.length; i++)
            {
                String[] cookie = cookies[i].split("=");
                String name = cookie[0].trim();
                cookieManager.setCookie(url, name + "=; Expires=Mon, 31 Dec 2012 23:59:59 GMT");
            }
            cookieManager.flush();
        }
    }

    public static String GetCookie(String url, String name)
    {
        CookieManager cookieManager = CookieManager.getInstance();
        if (cookieManager.hasCookies())
        {
            String[] cookies = cookieManager.getCookie(url).split(";");
            for (int i = 0; i < cookies.length; i++)
            {
                String cookieName = cookies[0].trim();
                if (cookieName.compareTo(name) == 0)
                {
                    return cookies[1];
                }
            }
        }
        return "";
    }

    public static String[] GetCookies(final String url)
    {
        CookieManager cookieManager = CookieManager.getInstance();
        String[] cookies = cookieManager.getCookie(url).split(";");
        return cookies;
    }

    public void onPageFinished(String url)
    {

    }

    public boolean shouldOverrideUrlLoading(String url, boolean isRedirectedByMouseClick)
    {
        return false;
    }
}
