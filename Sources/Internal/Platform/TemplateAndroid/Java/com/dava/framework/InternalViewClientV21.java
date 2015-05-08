package com.dava.framework;

import java.io.ByteArrayInputStream;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import com.dava.framework.JNIWebView.WebViewWrapper;

import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import android.webkit.WebResourceRequest;
import android.webkit.WebResourceResponse;
import android.webkit.WebView;

public class InternalViewClientV21 extends InternalViewClientV14 {
    public InternalViewClientV21(int _id) {
        super(_id);
    }
    
    // NOTE: This method is called on a thread other than the UI thread so
    // clients should exercise caution when accessing private data or the view
    // system.
    @Override
    public WebResourceResponse shouldInterceptRequest(final WebView view,
            WebResourceRequest request) {
        // check only main frame, not every image url
        if(request.isForMainFrame())
        {
            boolean isUserClick = request.hasGesture();
            Uri url = request.getUrl();
            FutureTask<Integer> task = PostOnUrlChangeTask(url.toString(), isUserClick);

            int res = eAction.PROCESS_IN_WEBVIEW;
            try {
                // wait till done
                res = task.get();
            } catch (InterruptedException e) {
                e.printStackTrace();
            } catch (ExecutionException e) {
                e.printStackTrace();
            }

            if (eAction.PROCESS_IN_WEBVIEW == res) {
                return null;
            } else if (eAction.PROCESS_IN_SYSTEM_BROWSER == res) {
                Intent exWeb = new Intent(Intent.ACTION_VIEW, url);
                JNIActivity.GetActivity().startActivity(exWeb);
                // return empty response
                // is there any better way to discard request here?
                WebResourceResponse response = new WebResourceResponse("text/plain", "UTF-8", createEmptyStream());
                
                // HACK we have to reload current web view
                // because we can't discard current request and leave 
                // web view as is
                WebViewWrapper wrap = (WebViewWrapper)view;
                final String currentViewUrl = wrap.getLastLoadedUrl();
                Runnable reloadView = new Runnable() {
                    @Override
                    public void run() {
                        view.loadUrl(currentViewUrl);
                    }
                };
                
                JNIActivity.GetActivity().runOnUiThread(reloadView);
                return response;
            } else {
                Log.e(JNIWebView.TAG, "unknown result code res = " + res);
            }
        }
        return null;
    }

    private ByteArrayInputStream createEmptyStream() {
        return new ByteArrayInputStream(new byte[0]);
    }
    
    @Override
    public boolean shouldOverrideUrlLoading(WebView view, final String url) {
        return false; // false means the current WebView handles the url
                      // true means the host application handles the url
    }
}
