package com.dava.framework;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import com.dava.framework.JNIWebView.WebViewWrapper;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.net.Uri;
import android.util.Log;
import android.view.View;
import android.webkit.WebView;
import android.webkit.WebViewClient;

// see c++ IUIWebViewDelegate
class eAction
{
    static int PROCESS_IN_WEBVIEW = 0;
    static int PROCESS_IN_SYSTEM_BROWSER = 1;
    static int NO_PROCESS = 2;
};

public class InternalViewClientV14 extends WebViewClient {
        int id;

        volatile boolean isRenderToTexture = false;
        volatile boolean pendingVisible = true;

        // precache as much as possible
        Bitmap bitmapCache = null;
        Canvas canvas = null;
        int pixels[] = null;
        int width = 0;
        int height = 0;

        public boolean isVisible()
        {
            return pendingVisible;
        }

        public void setVisible(WebViewWrapper view, boolean isVisible)
        {
            this.pendingVisible = isVisible;
            
            // Workaround: call updateVisible instantly because it will not be called on SystemDraw
            if(!isVisible) {
                updateVisible(view);
            }
        }
        
        public void updateVisible(WebViewWrapper view) {
        	boolean visible = view.getVisibility() == View.VISIBLE;
            if(visible != pendingVisible) {
                if (pendingVisible)
                {
                    view.setVisibility(View.VISIBLE);
                }
                else
                {
                    view.setVisibility(View.GONE);
                }
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

        void renderToTexture(WebViewWrapper view) {
            renderToBitmapAndCopyPixels(view);
            JNIActivity activity = JNIActivity.GetActivity();
            if (!activity.GetIsPausing())
            {
                activity.PostEventToGL(new OnPageLoadedNativeRunnable(pixels,
                    width, height));
            }
        }

        public InternalViewClientV14(int _id) {
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
                    JNIWebView.OnPageLoaded(id, pixels, width, height);
                }
            }
        }

        @Override
        public void onPageFinished(final WebView view, final String url) {
            super.onPageFinished(view, url);
            
            WebViewWrapper wrap = (WebViewWrapper)view;
            
            JNIActivity activity = JNIActivity.GetActivity();
            if (null == activity || activity.GetIsPausing()) {
                return;
            }

            if (isRenderToTexture) {
                // first try render into texture as soon as possible
                wrap.getInternalViewClient().renderToTexture(wrap);
                // second render with delay
                // Workaround to fix black/white view
                JNIWebView.startRecursiveRefreshSequence(wrap);
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
                Log.e(JNIWebView.TAG, "can't render WebView into bitmap");
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
            Log.e(JNIWebView.TAG, "Error in webview errorCode:" + errorCode
                    + " description:" + description + " failingUrl:"
                    + failingUrl);
        }

        @Override
        public boolean shouldOverrideUrlLoading(WebView view, final String url) {
            if (null == JNIActivity.GetActivity()
                    || JNIActivity.GetActivity().GetIsPausing())
                return false; // false means the current WebView handles the url

            boolean isRedirectedByMouseClick = true; // old default value from inside c++
            FutureTask<Integer> task = PostOnUrlChangeTask(url, isRedirectedByMouseClick);

            int res = eAction.PROCESS_IN_WEBVIEW;
            try {
                res = task.get();
            } catch (InterruptedException e) {
                e.printStackTrace();
            } catch (ExecutionException e) {
                e.printStackTrace();
            }

            if (eAction.PROCESS_IN_WEBVIEW == res) {
                return false;
            } else if (eAction.PROCESS_IN_SYSTEM_BROWSER == res) {
                Intent exWeb = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
                JNIActivity.GetActivity().startActivity(exWeb);
                return true; // return true means the host application handles the url
            }
            else
            {
                Log.e(JNIWebView.TAG, "unknown result code res = " + res);
            }
            return true;
        }

        FutureTask<Integer> PostOnUrlChangeTask(final String url, final boolean hasGesture) {
            Callable<Integer> urlChanged = new Callable<Integer>() {

                @Override
                public Integer call() throws Exception {
                    return JNIWebView.OnUrlChange(id, url, hasGesture);
                }
            };

            FutureTask<Integer> task = new FutureTask<Integer>(urlChanged);

            JNIActivity.GetActivity().PostEventToGL(task);

            return task;
        }
    }
