package com.dava.framework;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;
import android.annotation.SuppressLint;
import android.content.Intent;
import android.graphics.Color;
import android.net.Uri;
import android.util.Log;
import android.view.ViewGroup;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.FrameLayout;

@SuppressLint("UseSparseArrays")
public class JNIWebView {
	final static String TAG = "JNIWebView";
	static Map<Integer, WebView> views = new HashMap<Integer, WebView>();
	
	private static class InternalViewClient extends WebViewClient {
		int id;
		
		public InternalViewClient(int _id) {
			id = _id;
		}
		
		@Override
		public void onPageFinished(WebView view, String url) {
			super.onPageFinished(view, url);
			JNIActivity.GetActivity().PostEventToGL(new Runnable() {
				@Override
				public void run() {
					OnPageLoaded(id);
				}
			});
		};
		
		@Override
		public boolean shouldOverrideUrlLoading(WebView view, final String url) {
			Callable<Integer> urlChanged = new Callable<Integer>() {
				
				@Override
				public Integer call() throws Exception {
					return OnUrlChange(id, url);
				}
			};
			
			FutureTask<Integer> task = new FutureTask<Integer>(urlChanged);
			
			JNIActivity.GetActivity().PostEventToGL(task);
			
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
			
			/*enum eAction
			{
				PROCESS_IN_WEBVIEW = 0,
				PROCESS_IN_SYSTEM_BROWSER,
				NO_PROCESS,
				ACTIONS_COUNT
			};*/

			if (res == 0) {
				return false;
			}
			else if (res == 1) {
				Intent exWeb = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
				JNIActivity.GetActivity().startActivity(exWeb);
				return true;
			}
			return true;
		}
	}
	
	public static void Initialize(final int id, final float x, final float y, final float dx, final float dy)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (views.containsKey(id))
				{
					Log.d(TAG, String.format("WebView with id %d already initialized", id));
					return;
				}
				WebView webView = new WebView(activity);
				FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
						new FrameLayout.MarginLayoutParams((int)(dx + 0.5f), (int)(dy + 0.5f)));
				params.leftMargin = (int)x;
				params.topMargin = (int)y;
				params.width = (int)(dx + 0.5f);
				params.height = (int)(dy + 0.5f);
				webView.setWebViewClient(new InternalViewClient(id));
				webView.getSettings().setJavaScriptEnabled(true);
				webView.getSettings().setLoadWithOverviewMode(true);
				webView.getSettings().setUseWideViewPort(true);
				webView.setLayerType(WebView.LAYER_TYPE_SOFTWARE, null);
				
				activity.addContentView(webView, params);
				views.put(id, webView);
			}
		});
	}
	
	public static void Deinitialize(final int id)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (!views.containsKey(id))
				{
					Log.d(TAG, String.format("Unknown view id %d", id));
					return;
				}
				WebView view = views.get(id);
				views.remove(id);
				
				((ViewGroup)view.getParent()).removeView(view);
			}
		});
	}
	
	public static void OpenURL(final int id, final String url)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (!views.containsKey(id))
				{
					Log.d(TAG, String.format("Unknown view id %d", id));
					return;
				}
				WebView webView = views.get(id);
				webView.loadUrl(url);
			}
		});
	}
	
	public static void OpenFromBuffer(final int id, final String data, final String baseUrl)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable() {
			
			@Override
			public void run() {
				if (!views.containsKey(id))
				{
					Log.d(TAG, String.format("Unknown view id %d", id));
					return;
				}
				WebView webView = views.get(id);
				webView.loadDataWithBaseURL(baseUrl, data, "text/html", "utf-8", null);
			}
		});
	}
	
	public static void SetRect(final int id, final float x, final float y, final float dx, final float dy)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (!views.containsKey(id))
				{
					Log.d(TAG, String.format("Unknown view id %d", id));
					return;
				}
				
				WebView view = views.get(id);
				FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) view.getLayoutParams();
				params.leftMargin = (int)x;
				params.topMargin = (int)y;
				params.width = (int)(dx + 0.5f);
				params.height = (int)(dy + 0.5f); 
				view.setLayoutParams(params);
			}
		});
	}
	
	public static void SetVisible(final int id, final boolean isVisible)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (!views.containsKey(id))
				{
					Log.d(TAG, String.format("Unknown view id %d", id));
					return;
				}
				WebView view = views.get(id);
				view.setVisibility(isVisible ? WebView.VISIBLE : WebView.GONE);
			}
		});
	}

	public static void SetBackgroundTransparency(final int id, final boolean enabled)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable()
		{
			@Override
			public void run()
			{
				if (!views.containsKey(id))
				{
					Log.d(TAG, String.format("Unknown view id %d", id));
					return;
				}
				WebView view = views.get(id);
				view.setBackgroundColor((enabled ? Color.TRANSPARENT : Color.WHITE));
			}
		});
	}
	
	static protected void RelinkNativeControls() {
		for (WebView view: views.values()) {
			ViewGroup viewGroup = (ViewGroup) view.getParent();
			viewGroup.removeView(view);
			JNIActivity.GetActivity().addContentView(view, view.getLayoutParams());
		}
	}
	
	private static native int OnUrlChange(int id, String url);
	private static native int OnPageLoaded(int id);
}
