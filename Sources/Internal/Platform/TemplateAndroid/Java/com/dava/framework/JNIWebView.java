package com.dava.framework;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.net.Uri;
import android.os.Handler;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.KeyEvent;
import android.webkit.CookieManager;
import android.webkit.JsResult;
import android.webkit.WebChromeClient;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.FrameLayout;

public class JNIWebView {
    private static final int MOVE_VIEW_OFFSCREEN_STEP = 10000;
    private static final String TAG = "JNIWebView";
    private static final Paint paint = new Paint();

    public static class WebViewWrapper extends android.webkit.WebView {
        private InternalViewClient client = null;
        private final static int MAX_DELAY = 1600;
        private final static int START_DELAY = 50;
        private int delay = 50; //50, 100, 200, 400, 800, 1600
        private volatile boolean isLoadingData = false;
        
        public WebViewWrapper(Context context, InternalViewClient client) {
            super(context);
            this.client = client;
            super.setWebViewClient(client);
        }

        InternalViewClient getInternalViewClient() {
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
            // on lock/unlock if webview still was loading we have to call
            // reload() even if in client.isVisible() == false for now
            if (isLoadingData)
            {
                reload();
            }
        }
        @Override
        public void loadUrl(String url)
        {
            isLoadingData = true;
            super.loadUrl(url);
        }
        @Override
        public void loadData(String htmlString, String mimeType, String encoding)
        {
            isLoadingData = true;
            super.loadData(htmlString, mimeType, encoding);
        }
        @Override
        public void loadDataWithBaseURL(String baseUrl, String data, String mimeType,
                String encoding, String failUrl)
        {
            isLoadingData = true;
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
    }

    static Map<Integer, WebViewWrapper> views = new HashMap<Integer, WebViewWrapper>();

    public static class InternalViewClient extends WebViewClient {
        int id;

        volatile boolean isRenderToTexture = false;
        volatile boolean isVisible = true;

        // precache as much as possible
        Bitmap bitmapCache = null;
        Canvas canvas = null;
        int pixels[] = null;
        int width = 0;
        int height = 0;

        public boolean isVisible()
        {
            return isVisible;
        }

        public void setVisible(WebViewWrapper view, boolean isVisible)
        {
            this.isVisible = isVisible;
            if (isVisible)
            {
                view.setVisibility(View.VISIBLE);
            }
            else
            {
                view.setVisibility(View.GONE);
            }
        }

        public boolean isRenderToTexture() {
            return isRenderToTexture;
        }

        public void setRenderToTexture(WebViewWrapper view, boolean isRenderToTexture) {
            this.isRenderToTexture = isRenderToTexture;
            // update visibility after update isRenderToTexture
            view.updateViewRectPosition();

            if (isRenderToTexture) {
                renderToTexture(view);
            }
            else
            {
                view.invalidate();
            }
        }

        private void renderToTexture(WebViewWrapper view) {
            renderToBitmapAndCopyPixels(view);
            JNIActivity activity = JNIActivity.GetActivity();
            if (!activity.GetIsPausing())
            {
                activity.PostEventToGL(new OnPageLoadedNativeRunnable(pixels,
                    width, height));
            }
        }

        public InternalViewClient(int _id) {
            id = _id;
        }

        class OnPageLoadedNativeRunnable implements Runnable {
            int[] pixels;
            int width;
            int height;

            OnPageLoadedNativeRunnable(int[] pixels, int width, int height) {
                this.pixels = pixels;
                this.width = width;
                this.height = height;
            }

            @Override
            public void run() {
                // if user lock screen just return - prevent crush in gl thread
                if(!JNIGLSurfaceView.isPaused())
                {
                    OnPageLoaded(id, pixels, width, height);
                }
            }
        }

        @Override
        public void onPageFinished(final WebView view, final String url) {
            super.onPageFinished(view, url);
            
            WebViewWrapper wrap = (WebViewWrapper)view;
            // mark web view loaded content so on lock/unlock do not call
            // reload
            wrap.isLoadingData = false;
            
            JNIActivity activity = JNIActivity.GetActivity();
            if (null == activity || activity.GetIsPausing()) {
                return;
            }

            if (isRenderToTexture) {
                // first try render into texture as soon as possible
                wrap.getInternalViewClient().renderToTexture(wrap);
                // second render with delay
                // Workaround to fix black/white view
                startRecursiveRefreshSequence(wrap);
            } 
            else 
            {
                activity.PostEventToGL(new OnPageLoadedNativeRunnable(null, 0, 0));
            }
        }

        private void renderToBitmapAndCopyPixels(WebView view) {
            Bitmap bitmap = renderWebViewIntoBitmap(view);
            if (bitmap != null) {
                if (pixels == null || width != bitmap.getWidth()
                        || height != bitmap.getHeight()) {
                    width = bitmap.getWidth();
                    height = bitmap.getHeight();
                    pixels = new int[width * height];
                }
                // copy ARGB pixels values into our buffer
                bitmap.getPixels(pixels, 0, width, 0, 0, width, height);
            }
        }

        private Bitmap renderWebViewIntoBitmap(final WebView view) {

            if (bitmapCache != null) {
                bitmapCache.recycle();
            }

            // we need to do it every time because this only works with
            // scrolling
            view.setDrawingCacheEnabled(true);
            view.buildDrawingCache();
            // Returns an immutable bitmap from the source bitmap.
            // The new bitmap may be the same object as source, or a copy may
            // have been made. It is initialized with the same density as the
            // original bitmap.
            Bitmap cacheImage = view.getDrawingCache();
            if (cacheImage == null)
            {
                Log.e(TAG, "can't render WebView into bitmap");
            } 
            else
            {
                bitmapCache = Bitmap.createBitmap(cacheImage);
            }

            view.setDrawingCacheEnabled(false);
            return bitmapCache;
        };

        @Override
        public void onReceivedError(WebView view, int errorCode,
                String description, String failingUrl) {
            Log.e(TAG, "Error in webview errorCode:" + errorCode
                    + " description:" + description + " failingUrl:"
                    + failingUrl);
        }

        @Override
        public void onLoadResource(WebView view, String url) {
            String[] urlParts = url.split("/");
            if (urlParts.length > 0) {
                String urlPart = urlParts[urlParts.length - 1];
                if (urlPart.charAt(0) == '?' && !urlPart.contains("."))
                    PostOnUrlChangeTask(url);
            }

            super.onLoadResource(view, url);
        }

        @Override
        public boolean shouldOverrideUrlLoading(WebView view, final String url) {
            if (null == JNIActivity.GetActivity()
                    || JNIActivity.GetActivity().GetIsPausing())
                return false;

            FutureTask<Integer> task = PostOnUrlChangeTask(url);

            while (!task.isDone()) {
                Thread.yield();
            }

            int res = 0;
            try {
                res = task.get();
            } catch (InterruptedException e) {
                e.printStackTrace();
            } catch (ExecutionException e) {
                e.printStackTrace();
            }

            if (res == 0) {
                return false;
            } else if (res == 1) {
                Intent exWeb = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
                JNIActivity.GetActivity().startActivity(exWeb);
                return true;
            }
            else
            {
                Log.e(TAG, "unknown result code res = " + res);
            }
            return true;
        }

        FutureTask<Integer> PostOnUrlChangeTask(final String url) {
            Callable<Integer> urlChanged = new Callable<Integer>() {

                @Override
                public Integer call() throws Exception {
                    return OnUrlChange(id, url);
                }
            };

            FutureTask<Integer> task = new FutureTask<Integer>(urlChanged);

            JNIActivity.GetActivity().PostEventToGL(task);

            return task;
        }
    }

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
                final WebViewWrapper webView = new WebViewWrapper(activity, 
                        new InternalViewClient(id));
                
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
                InternalViewClient client = view.getInternalViewClient();
                client.setVisible(view, isVisible);
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
                InternalViewClient client = view.getInternalViewClient();
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
            InternalViewClient client = view.getInternalViewClient();
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

    private static native int OnUrlChange(int id, String url);

    private static native int OnPageLoaded(int id, int[] pixels, int width,
            int height);

    private static native void OnExecuteJScript(int id, String result);

    private static void startRecursiveRefreshSequence(WebViewWrapper wrap) {
        wrap.delay = WebViewWrapper.START_DELAY; 
        refreshStaticTexture(wrap);
    }
}
