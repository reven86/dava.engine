package com.dava.framework;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import android.annotation.SuppressLint;
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
import android.webkit.CookieSyncManager;
import android.webkit.JsResult;
import android.webkit.WebChromeClient;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.FrameLayout;

@SuppressLint("UseSparseArrays")
public class JNIWebView {
	final static String TAG = "JNIWebView";
	final static Paint paint = new Paint();
	static Map<Integer, WebView> views = new HashMap<Integer, WebView>();
	
	private static class InternalViewClient extends WebViewClient {
		int id;
		
		// TODO remember back reference to WebView to check isVisible
		
		boolean isRenderToTexture = false;
		
		public boolean isRenderToTexture() {
			return isRenderToTexture;
		}

		public void setRenderToTexture(boolean isRenderToTexture) {
			this.isRenderToTexture = isRenderToTexture;
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
				Log.d(TAG, "id = " + id + " pixels = " + pixels + " width = "
						+ width + " height = " + height);
				OnPageLoaded(id, pixels, width, height);
				Log.d(TAG, "finish onPageLoded");
			}
		}
		
		@Override
		public void onPageFinished(WebView view, String url) {
			super.onPageFinished(view, url);
			JNIActivity activity = JNIActivity.GetActivity();
			if (null == activity || activity.GetIsPausing())
				return;
			int pixels[] = null;
			int width = 0;
			int height = 0;
			
			if (isRenderToTexture)
			{
				// render webview into bitmap and pass it to native code
				Bitmap bitmap = renderWebViewIntoBitmap(view);
				width = bitmap.getWidth();
				height = bitmap.getHeight();
				pixels = new int[width * height]; 
				// copy ARGB pixels values into our buffer
				bitmap.getPixels(pixels, 0, width, 0, 0, width, height);
				Log.d(TAG, "prepare bitmap for webview texture");
			}
			
			JNIActivity.GetActivity().PostEventToGL(
					new OnPageLoadedNativeRunnable(pixels, width, height));
		}

		private Bitmap renderWebViewIntoBitmap(WebView view) {
			assert view != null;
			assert view.getMeasuredWidth() != 0;
			assert view.getMeasuredHeight() != 0;
			
			int view_width = view.getMeasuredWidth();
			int view_height = view.getMeasuredHeight();
			
			Bitmap bm = Bitmap.createBitmap(view_width,
                    view_height, Bitmap.Config.ARGB_8888);
			
            assert bm != null;
            Canvas bigcanvas = new Canvas(bm);
            int height = bm.getHeight();
            bigcanvas.drawBitmap(bm, 0, height, paint);
            view.draw(bigcanvas);
//            try {
//                String path = Environment.getExternalStorageDirectory()
//                        .toString();
//                OutputStream fOut = null;
//                File file = new File(path, "/test_output.png");
//                fOut = new FileOutputStream(file);
//
//                bm.compress(Bitmap.CompressFormat.PNG, 50, fOut);
//                fOut.flush();
//                fOut.close();
//                bm.recycle();
//            } catch (Exception e) {
//                e.printStackTrace();
//            }
            return bm;
		};
		
		
		@Override
		public void onLoadResource(WebView view, String url) {
			String[] urlParts = url.split("/");
			if (urlParts.length > 0)
			{
				String urlPart = urlParts[urlParts.length - 1];
				if (urlPart.charAt(0) == '?' && !urlPart.contains("."))
					PostOnUrlChangeTask(url);
			}
			
			super.onLoadResource(view, url);
		}
		
		@Override
		public boolean shouldOverrideUrlLoading(WebView view, final String url) {
			if (null == JNIActivity.GetActivity() || JNIActivity.GetActivity().GetIsPausing())
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
		
		FutureTask<Integer> PostOnUrlChangeTask(final String url) {
			Callable<Integer> urlChanged = new Callable<Integer>() {
				
				@Override
				public Integer call() throws Exception {
					return OnUrlChange(id, url);
				}
			};
			
			if (url.contains("code"))
				Log.d("shouldOverrideUrlLoading", url);
			
			FutureTask<Integer> task = new FutureTask<Integer>(urlChanged);
			
			JNIActivity.GetActivity().PostEventToGL(task);
			
			return task;
		}
	}
	
	public static void Initialize(final int id, final float x, final float y, final float dx, final float dy)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		if (null == activity || activity.GetIsPausing())
			return;

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
				webView.getSettings().setUseWideViewPort(false);
				if (android.os.Build.VERSION.SDK_INT >= 11)
				{
				webView.setLayerType(WebView.LAYER_TYPE_SOFTWARE, null);
				}
				webView.setWebChromeClient(new InternalWebClient(id));
				webView.setOnTouchListener(new View.OnTouchListener()
				{
				    @Override
				    public boolean onTouch(View v, MotionEvent event)
				    {
				        switch (event.getAction())
				        {
				            case MotionEvent.ACTION_DOWN:
				            case MotionEvent.ACTION_UP:
				                if (!v.hasFocus())
				                {
				                    v.requestFocus();
				                }
				                break;
				        }
				        return false;
				    }
				});
				
				activity.addContentView(webView, params);
				views.put(id, webView);
				
				CookieSyncManager.createInstance(activity.getApplicationContext()); 
			}
		});
	}
	
	public static void Deinitialize(final int id)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		if (null == activity || activity.GetIsPausing())
			return;

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
		if (null == activity || activity.GetIsPausing())
			return;

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
	
	public static void LoadHtmlString(final int id, final String htmlString)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		if (null == activity || activity.GetIsPausing())
			return;

		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (!views.containsKey(id))
				{
					Log.d(TAG, String.format("Unknown view id %d", id));
					return;
				}
				WebView webView = views.get(id);
				webView.loadData(htmlString, "text/html", null);
			}
		});
	}
		
	public static void OpenFromBuffer(final int id, final String data, final String baseUrl)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		if (null == activity || activity.GetIsPausing())
			return;

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
	
	public static void ExecuteJScript(final int id, final int requestId, final String scriptString)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		if (null == activity || activity.GetIsPausing())
			return;
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (!views.containsKey(id))
				{
					Log.d(TAG, String.format("Unknown view id %d", id));
					return;
				}
				WebView webView = views.get(id);
				//String a = "javascript:function call_back_func(){return \"" + requestId +", \" + " + scriptString + ";}javascript:alert(call_back_func())";
				String a = "javascript:function call_back_func(){return \"" + requestId +", \" + eval(\"" + scriptString + "\");}javascript:alert(call_back_func())";
				webView.loadUrl(a);
			}
		});
	}
	
	public static void DeleteCookies(final String targetURL)
	{
		// The CookieSyncManager is used to synchronize the browser cookie store between RAM and permanent storage. 		
		CookieManager cookieManager = CookieManager.getInstance();
		if (cookieManager.hasCookies())
		{
		   	// Get cookies for specific URL and change their expiration date
		   	// This should force android system to remove these cookies
		   	String cookiestring = cookieManager.getCookie(targetURL);
		   	String[] cookies =  cookiestring.split(";");
		    
		   	for (int i=0; i<cookies.length; i++) 
		   	{
		   		String[] cookieparts = cookies[i].split("=");
		   		cookieManager.setCookie(targetURL, cookieparts[0].trim()+"=; Expires=Mon, 31 Dec 2012 23:59:59 GMT");
		   	}
		   	// Synchronize cookies storage
		   	CookieSyncManager.getInstance().sync();
		    
		   	// Check if cookies were removed - if not - delete all cookies
		   	cookiestring = cookieManager.getCookie(targetURL);
		   	cookies =  cookiestring.split(";");
		   	if (cookies.length > 0)
		   	{
		   		cookieManager.removeExpiredCookie();
		   		// Synchronize cookies storage
		   		CookieSyncManager.getInstance().sync();
		   	}	
		}
	}	
	
	public static String GetCookie(final String targetURL, final String cookieName)
	{		
		// The CookieSyncManager is used to synchronize the browser cookie store between RAM and permanent storage. 
	    CookieManager cookieManager = CookieManager.getInstance();

	    if (cookieManager.hasCookies())
	    {
	    	// Get cookies for specific URL
	    	String cookieString = cookieManager.getCookie(targetURL);	
	    	String[] cookies =  cookieString.split(";");

	    	for (int i=0; i<cookies.length; i++) 
	    	{
	    		String[] cookieparts = cookies[i].split("=");
	    		if (cookieparts[0].trim().compareTo(cookieName) == 0)
	    		{
	    			return cookieparts[1];		
	    		}			    		
	    	}
	    }
	    
	    return "";
	}
	
	public static Object[] GetCookies(final String targetURL)
	{
		// The CookieSyncManager is used to synchronize the browser cookie store between RAM and permanent storage. 
	    CookieManager cookieManager = CookieManager.getInstance();
	    // Get cookies for specific URL and put them into array
	    String cookieString = cookieManager.getCookie(targetURL);
	    String[] cookies =  cookieString.split(";");	    
	    return cookies;
	}

	public static void SetRect(final int id, final float x, final float y, final float dx, final float dy)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		if (null == activity || activity.GetIsPausing())
			return;

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
		if (null == activity || activity.GetIsPausing())
			return;

		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (!views.containsKey(id))
				{
					Log.d(TAG, String.format("Unknown view id %d", id));
					return;
				}
				WebView view = views.get(id);
				
				InternalWebClient internalClient = (InternalWebClient)view.getW
				
				int visible = isVisible ? WebView.VISIBLE : WebView.INVISIBLE;
				view.setVisibility(visible);
			}
		});
	}

	public static void SetBackgroundTransparency(final int id, final boolean enabled)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		if (null == activity || activity.GetIsPausing())
			return;

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
	
	static class InternalWebClient extends WebChromeClient {
		int id;
		
		InternalWebClient(int id) {
			this.id = id;
		}
		
		@Override
		public boolean onJsAlert(WebView view, String url, String message,
				JsResult result) {

			class jsCallback implements Callable<Void> {
				int id;
				int requestId;
				String result;
				
				jsCallback(int id, int requestId, String result){
					this.id = id;
					this.requestId = requestId;
					this.result = result;
				}

				@Override
				public Void call() throws Exception {
					OnExecuteJScript(this.id, this.requestId, this.result);
					return null;
				}
				
			}
			
			int split = message.indexOf(",");
			if (split > 0)
			{
				FutureTask<Void> task = new FutureTask<Void>(new jsCallback(
						id,
						Integer.parseInt((String) message.subSequence(0, split)),
						(String) message.subSequence(split + 2, message.length())));
			
				JNIActivity.GetActivity().PostEventToGL(task);
			}
			
			result.confirm();
			return true;
		}
	}
	
	static protected void RelinkNativeControls() {
		for (WebView view: views.values()) {
			ViewGroup viewGroup = (ViewGroup) view.getParent();
			viewGroup.removeView(view);
			JNIActivity.GetActivity().addContentView(view, view.getLayoutParams());
		}
	}
	
	private static native int OnUrlChange(int id, String url);
	private static native int OnPageLoaded(int id, int[] pixels, int width, int height);
	private static native void OnExecuteJScript(int id, int requestId, String result);
}
