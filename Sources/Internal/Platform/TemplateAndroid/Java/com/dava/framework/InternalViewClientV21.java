package com.dava.framework;

import java.io.ByteArrayInputStream;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import android.content.Intent;
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
    public WebResourceResponse shouldInterceptRequest(WebView view,
            WebResourceRequest request) {
        boolean isUserClick = request.hasGesture();
        FutureTask<Integer> task = PostOnUrlChangeTask(request.getUrl()
                .toString(), isUserClick);

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
            return null;
        } else if (res == 1) {
            Intent exWeb = new Intent(Intent.ACTION_VIEW, request.getUrl());
            JNIActivity.GetActivity().startActivity(exWeb);
            return new WebResourceResponse("text/plain", "UTF-8", createEmptyStream());
        } else {
            Log.e(JNIWebView.TAG, "unknown result code res = " + res);
        }
        return null;
    }

    private ByteArrayInputStream createEmptyStream() {
        return new ByteArrayInputStream(new byte[0]);
    }
    
    @Override
    public boolean shouldOverrideUrlLoading(WebView view, final String url) {
        return false; // false means the current WebView handles the url
    }
}
