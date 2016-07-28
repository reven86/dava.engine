package com.dava.engine;

import android.app.Activity;
import android.os.Bundle;
import android.view.ViewGroup;
import android.widget.RelativeLayout;
import android.content.res.Configuration;
import android.util.Log;

public final class DavaActivity extends Activity
{
    public static final boolean CALL_NATIVE = true;
    public static final String LOG_TAG = "DAVA";

    public static DavaActivity activitySingleton;
    
    protected static Thread davaMainThread;
    
    protected boolean isPaused;
    protected boolean hasFocus;

    private DavaSurface primarySurface;
    private ViewGroup layout;

    static {
        // TODO: I need another way to load necessary libraries
        System.loadLibrary("gnustl_shared");
        System.loadLibrary("TestBed");
    }
    
    public static native long nativeOnCreate();
    public static native void nativeOnStart();
    public static native void nativeOnResume();
    public static native void nativeOnPause();
    public static native void nativeOnStop();
    public static native void nativeOnDestroy();
    public static native void nativeGameThread();
    
    DavaActivity()
    {
        super();
    }
    
    protected void onCreate(Bundle savedInstanceState)
    {
        Log.d(LOG_TAG, "DavaActivity.onCreate");
        super.onCreate(savedInstanceState);
        
        activitySingleton = this;
        
        String cmdline = GetCommandLine();
        Log.i(LOG_TAG, "****************** commandline: " + cmdline);
        
        long primaryWindowBackendPointer = nativeOnCreate();
        //long primaryWindowBackendPointer = 0;
        primarySurface = new DavaSurface(getApplication(), primaryWindowBackendPointer, true);
        
        layout = new RelativeLayout(this);
        layout.addView(primarySurface);
        setContentView(layout);
        
        isPaused = true;
        hasFocus = false;
    }

    protected void onStart()
    {
        Log.d(LOG_TAG, "DavaActivity.onStart");
        super.onStart();
        
        if (davaMainThread != null)
        {
            if (CALL_NATIVE)
            {
                nativeOnStart();
            }
        }
    }

    protected void onResume()
    {
        Log.d(LOG_TAG, "DavaActivity.onResume");
        super.onResume();

        handleResume();
    }

    protected void onPause()
    {
        Log.d(LOG_TAG, "DavaActivity.onPause");
        super.onPause();

        handlePause();
    }

    protected void onStop()
    {
        Log.d(LOG_TAG, "DavaActivity.onStop");
        super.onStop();
        
        if (davaMainThread != null)
        {
            if (CALL_NATIVE)
            {
                nativeOnStop();
            }
        }
    }

    protected void onRestart()
    {
        Log.d(LOG_TAG, "DavaActivity.onRestart");
        super.onRestart();
    }

    protected void onDestroy()
    {
        Log.d(LOG_TAG, "DavaActivity.onDestroy");
        
        if (CALL_NATIVE)
        {
            nativeOnDestroy();
        }
        Log.d(LOG_TAG, "DavaActivity.onDestroy: nativeOnDestroy called");
        
        if (davaMainThread != null)
        {
            Log.d(LOG_TAG, "DavaActivity.onDestroy: wait thread");
            try {
                davaMainThread.join();
            } catch (Exception e) {
                Log.e(LOG_TAG, "DavaActivity.onDestroy: davaWatcherThread.join() failed " + e);
            }

            davaMainThread = null;
        }
        activitySingleton = null;
        
        super.onDestroy();
        Log.d(LOG_TAG, "DavaActivity.onDestroy: leave");
    }
    
    public void onWindowFocusChanged(boolean hasWindowFocus)
    {
        Log.d(LOG_TAG, String.format("DavaActivity.onWindowFocusChanged: focus=%b", hasWindowFocus));

        hasFocus = hasWindowFocus;
        if (hasFocus)
        {
            handleResume();
        }
    }
    
    public void onConfigurationChanged(Configuration newConfig)
    {
        Log.d(LOG_TAG, "DavaActivity.onConfigurationChanged");
        super.onConfigurationChanged(newConfig);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    public void handleResume()
    {
        if (davaMainThread == null)
        {
            if (CALL_NATIVE)
            {
                startDavaMainThread();
            }
        }
        
        if (isPaused && hasFocus)
        {
            Log.d(LOG_TAG, "********* DavaActivity.handleResume");
            isPaused = false;
            
            if (CALL_NATIVE)
            {
                nativeOnResume();
            }
            primarySurface.handleResume();
        }
    }

    public void handlePause()
    {
        if (!isPaused)
        {
            Log.d(LOG_TAG, "********* DavaActivity.handlePause");
            isPaused = true;
            
            primarySurface.handlePause();
            if (CALL_NATIVE)
            {
                nativeOnPause();
            }
        }
    }

    protected void startDavaMainThread()
    {
        if (davaMainThread == null)
        {
            davaMainThread = new Thread(new Runnable() {
                @Override
                public void run()
                {
                    nativeGameThread();
                    Log.d(LOG_TAG, "########## main-thread: leave");
                }
            }, "DAVA main thread");
            davaMainThread.start();
        }
    }
    
    private String GetCommandLine()
    {
        String result = "";
        Bundle extras = getIntent().getExtras();
        if (extras != null)
        {
            for (String key : extras.keySet())
            {
                String value = extras.getString(key);
                result += key + " " + value + " ";
            }
        }
        return result;
    }
}
