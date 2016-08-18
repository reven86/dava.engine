package com.dava.engine;

import android.webkit.WebView;
import android.webkit.WebViewClient;

class DavaWebViewClient extends WebViewClient
{
    @Override
    public void onPageFinished(WebView view, String url)
    {
        //DavaWebView davaWebView = (DavaWebView)view;
        //davaWebView.onPageFinished(url);
    }

    //@Override
    //public boolean shouldOverrideUrlLoading(WebView view, String url)
    //{
        //DavaWebView davaWebView = (DavaWebView)view;
        //return davaWebView.shouldOverrideUrlLoading(url, true);
    //}

    @Override
    public void onReceivedError(WebView view, int errorCode, String description, String failingUrl)
    {

    }
}
