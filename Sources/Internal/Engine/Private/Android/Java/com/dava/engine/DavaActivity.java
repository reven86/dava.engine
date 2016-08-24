package com.dava.engine;

import android.app.Activity;
import android.app.Application;
import android.os.Bundle;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.content.res.Configuration;
import android.util.Log;

public final class DavaActivity extends Activity
{
    public static final String LOG_TAG = "DAVA";

    private static DavaActivity activitySingleton;
    private static Thread davaMainThread;

    protected boolean isPaused = true;
    protected boolean hasFocus = false;
    protected boolean exitCalledFromJava = false;

    protected DavaCommandHandler commandHandler;
    
    private DavaSurfaceView primarySurfaceView;
    private ViewGroup layout;

    static {
        // TODO: I need another way to load necessary libraries
        System.loadLibrary("gnustl_shared");
        System.loadLibrary("TestBed");
    }
    
    public static native void nativeInitializeEngine(String externalFilesDir,
                                                     String internalFilesDir,
                                                     String appPath,
                                                     String packageName,
                                                     String cmdline);
    public static native void nativeShutdownEngine();
    public static native long nativeOnCreate();
    public static native void nativeOnResume();
    public static native void nativeOnPause();
    public static native void nativeOnDestroy();
    public static native void nativeGameThread();

    public static DavaActivity instance()
    {
        return activitySingleton;
    }

    public static DavaCommandHandler commandHandler()
    {
        return activitySingleton.commandHandler;
    }

    protected void onCreate(Bundle savedInstanceState)
    {
        Log.d(LOG_TAG, "DavaActivity.onCreate");
        super.onCreate(savedInstanceState);
        
        activitySingleton = this;
        commandHandler = new DavaCommandHandler();
        
        Application app = getApplication();
        String externalFilesDir = app.getExternalFilesDir(null).getAbsolutePath() + "/";
        String internalFilesDir = app.getFilesDir().getAbsolutePath() + "/";
        String sourceDir = app.getApplicationInfo().publicSourceDir;
        String packageName = app.getApplicationInfo().packageName;
        String cmdline = GetCommandLine();
        nativeInitializeEngine(externalFilesDir, internalFilesDir, sourceDir, packageName, cmdline);
        
        long primaryWindowBackendPointer = nativeOnCreate();
        primarySurfaceView = new DavaSurfaceView(getApplication(), primaryWindowBackendPointer);
        
        layout = new FrameLayout(this);
        layout.addView(primarySurfaceView);
        setContentView(layout);
    }

    protected void onStart()
    {
        Log.d(LOG_TAG, "DavaActivity.onStart");
        super.onStart();
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

    protected void onRestart()
    {
        Log.d(LOG_TAG, "DavaActivity.onRestart");
        super.onRestart();
    }
    
    protected void onStop()
    {
        Log.d(LOG_TAG, "DavaActivity.onStop");
        super.onStop();
    }

    protected void onDestroy()
    {
        Log.d(LOG_TAG, "DavaActivity.onDestroy");
        super.onDestroy();

        exitCalledFromJava = true;
        nativeOnDestroy();
        if (davaMainThread != null)
        {
            try {
                davaMainThread.join();
            } catch (Exception e) {
                Log.e(LOG_TAG, "DavaActivity.onDestroy: davaMainThread.join() failed " + e);
            }
            davaMainThread = null;
        }
        nativeShutdownEngine();
        activitySingleton = null;
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

    protected void handleResume()
    {
        startDavaMainThreadIfNotRunning();
        if (isPaused && hasFocus)
        {
            isPaused = false;
            nativeOnResume();
            primarySurfaceView.handleResume();
        }
    }

    protected void handlePause()
    {
        if (!isPaused)
        {
            isPaused = true;
            primarySurfaceView.handlePause();
            nativeOnPause();
        }
    }

    protected void startDavaMainThreadIfNotRunning()
    {
        if (davaMainThread == null)
        {
            davaMainThread = new Thread(new Runnable() {
                @Override public void run()
                {
                    nativeGameThread();
                }
            }, "DAVA main thread");
            davaMainThread.start();
        }
    }
    
    private String GetCommandLine()
    {
        String result = "app_process ";
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
