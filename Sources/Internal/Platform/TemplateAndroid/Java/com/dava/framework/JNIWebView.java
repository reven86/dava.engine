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
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.CookieManager;
import android.webkit.JsResult;
import android.webkit.WebChromeClient;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.FrameLayout;

public class JNIWebView {
    final static String TAG = "JNIWebView";
    final static Paint paint = new Paint();

    public static class WebViewWrapper extends android.webkit.WebView {
        public WebViewWrapper(Context context) {
            super(context);
        }

        private InternalViewClient client = null;

        void setWebViewClient(InternalViewClient client) {
            assert this.client == null;
            assert client != null;

            this.client = client;
            super.setWebViewClient(client);
        }

        InternalViewClient getInternalViewClient() {
            assert client != null;
            return client;
        }
    }

    static Map<Integer, WebViewWrapper> views = new HashMap<Integer, WebViewWrapper>();

    public static class InternalViewClient extends WebViewClient {
        int id;

        boolean isRenderToTexture = false;
        boolean isVisible = true;

        // precache as much as possible
        Bitmap bitmapCache = null;
        Canvas canvas = null;
        int pixels[] = null;
        int width = 0;
        int height = 0;

        public boolean isVisible() {
            return isVisible;
        }

        public void setVisible(WebView view, boolean isVisible) {

            this.isVisible = isVisible;
            int nextVisibleState = View.VISIBLE;

            if (isRenderToTexture) {
                nextVisibleState = isVisible ? View.INVISIBLE : View.GONE;
                // transit from VISIBLE to INVISIBLE only through GONE!
                if (view.getVisibility() == View.VISIBLE
                        && nextVisibleState == View.INVISIBLE) {
                    view.setVisibility(View.GONE);
                }
            } else {
                nextVisibleState = isVisible ? View.VISIBLE : View.GONE;
                // transit from INVISIBLE to VISIBLE only through GONE!
                if (view.getVisibility() == View.INVISIBLE
                        && nextVisibleState == View.VISIBLE) {
                    view.setVisibility(View.GONE);
                }
            }
            view.setVisibility(nextVisibleState);
        }

        public boolean isRenderToTexture() {
            return isRenderToTexture;
        }

        public void setRenderToTexture(WebView view, boolean isRenderToTexture) {
            this.isRenderToTexture = isRenderToTexture;
            // update visibility after update isRenderToTexture
            setVisible(view, isVisible);

            if (isRenderToTexture) {
                renderToBitmapAndCopyPixels(view);
                JNIActivity activity = JNIActivity.GetActivity();
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
                OnPageLoaded(id, pixels, width, height);
            }
        }

        @Override
        public void onPageFinished(final WebView view, String url) {
            super.onPageFinished(view, url);

            final JNIActivity activity = JNIActivity.GetActivity();
            if (null == activity || activity.GetIsPausing()) {
                return;
            }

            if (isRenderToTexture) {
                int delay = 0; // first do not wait
                renderWebViewUntilDone(view, activity, delay);
            } else {
                activity.PostEventToGL(new OnPageLoadedNativeRunnable(null, 0,
                        0));
            }
        }

        private void renderWebViewUntilDone(final WebView view,
                final JNIActivity activity, int delay) {
            final Runnable action = new Runnable() {

                @Override
                public void run() {
                    WebViewWrapper wrap = (WebViewWrapper) view;
                    wrap.getInternalViewClient().renderToBitmapAndCopyPixels(
                            view);
                    boolean foundNotTransparentPixel = false;
                    if (null != pixels) {
                        for (int pixel : pixels) {
                            if (pixel != 0) {
                                foundNotTransparentPixel = true;
                                break;
                            }
                        }
                    }
                    if (foundNotTransparentPixel) {
                        activity.PostEventToGL(new OnPageLoadedNativeRunnable(
                                pixels, width, height));
                    } else {
                        renderWebViewUntilDone(view, activity, 300);
                    }
                }

            };
            view.postDelayed(action, delay);
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

        private Bitmap renderWebViewIntoBitmap(WebView view) {

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
            if (cacheImage != null) {
                bitmapCache = Bitmap.createBitmap(view.getDrawingCache());
            }

            view.setDrawingCacheEnabled(false);
            return bitmapCache;
            // return b;
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
                WebViewWrapper webView = new WebViewWrapper(activity);
                FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
                        new FrameLayout.MarginLayoutParams((int) (dx + 0.5f),
                                (int) (dy + 0.5f)));

                params.leftMargin = (int) x;
                params.topMargin = (int) y;
                params.width = (int) (dx + 0.5f);
                params.height = (int) (dy + 0.5f);
                webView.setWebViewClient(new InternalViewClient(id));
                webView.getSettings().setJavaScriptEnabled(true);
                webView.getSettings().setLoadWithOverviewMode(true);
                webView.getSettings().setUseWideViewPort(false);

                if (android.os.Build.VERSION.SDK_INT >= 11) {
                    webView.setLayerType(WebView.LAYER_TYPE_SOFTWARE, null);
                }
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
                WebView webView = views.get(id);
                webView.loadUrl(url);
            }
        });
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
                WebView webView = views.get(id);
                webView.loadData(htmlString, "text/html", null);
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

                WebView webView = views.get(id);
                webView.loadDataWithBaseURL(baseUrl, data, "text/html",
                        "utf-8", null);
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
                WebView webView = views.get(id);

                String escapedJS = scriptString.replace("\"", "\\\"");

                String javaScript = "javascript:function call_back_func()"
                        + "{" 
                        + "    return \"\" + eval(\"" + escapedJS + "\");" 
                        + "}" 
                        + "javascript:alert(call_back_func())";

                webView.loadUrl(javaScript);
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

                WebView view = views.get(id);
                FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) view
                        .getLayoutParams();
                params.leftMargin = (int) x;
                params.topMargin = (int) y;
                params.width = (int) (dx + 0.5f);
                params.height = (int) (dy + 0.5f);
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

    static protected void RelinkNativeControls() {
        for (WebView view : views.values()) {
            ViewGroup viewGroup = (ViewGroup) view.getParent();
            viewGroup.removeView(view);
            JNIActivity.GetActivity().addContentView(view,
                    view.getLayoutParams());
        }
    }

    private static native int OnUrlChange(int id, String url);

    private static native int OnPageLoaded(int id, int[] pixels, int width,
            int height);

    private static native void OnExecuteJScript(int id, String result);
}
