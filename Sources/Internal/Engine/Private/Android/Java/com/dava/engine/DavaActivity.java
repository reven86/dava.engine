package com.dava.engine;

import android.app.Activity;
import android.app.Application;
import android.content.res.Configuration;
import android.os.Build;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.util.Log;

import java.lang.reflect.Constructor;
import java.io.InputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

public final class DavaActivity extends Activity
                                implements View.OnSystemUiVisibilityChangeListener
{
    public static final String LOG_TAG = "DAVA";

    private static DavaActivity activitySingleton;
    private static Thread davaMainThread;

    protected boolean isPaused = true;
    protected boolean hasFocus = false;
    protected boolean isDestoying = false;

    protected DavaCommandHandler commandHandler = new DavaCommandHandler();
    protected DavaKeyboardState keyboardState = new DavaKeyboardState();
    // List of class instances loaded from boot_classes files on startup
    protected List<Object> bootstrapObjects = new LinkedList<Object>();

    private DavaSurfaceView primarySurfaceView;
    private ViewGroup layout;

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

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        Log.d(LOG_TAG, "DavaActivity.onCreate");
        super.onCreate(savedInstanceState);
        
        activitySingleton = this;
        // Load library modules and create class instances specified in
        // res/raw/boot_modules and res/raw/boot_classes accordingly
        bootstrap();

        Application app = getApplication();
        String externalFilesDir = app.getExternalFilesDir(null).getAbsolutePath() + "/";
        String internalFilesDir = app.getFilesDir().getAbsolutePath() + "/";
        String sourceDir = app.getApplicationInfo().publicSourceDir;
        String packageName = app.getApplicationInfo().packageName;
        String cmdline = getCommandLine();
        nativeInitializeEngine(externalFilesDir, internalFilesDir, sourceDir, packageName, cmdline);

        Window window = getWindow();
        window.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_NOTHING | WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
        window.addFlags(WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN | WindowManager.LayoutParams.FLAG_FULLSCREEN);
        window.requestFeature(Window.FEATURE_ACTION_MODE_OVERLAY);
        window.requestFeature(Window.FEATURE_ACTION_BAR_OVERLAY);
        window.requestFeature(Window.FEATURE_NO_TITLE);
        window.getDecorView().setOnSystemUiVisibilityChangeListener(this);
        hideNavigationBar();
        
        long primaryWindowBackendPointer = nativeOnCreate();
        primarySurfaceView = new DavaSurfaceView(getApplication(), primaryWindowBackendPointer);
        
        layout = new FrameLayout(this);
        layout.addView(primarySurfaceView);
        setContentView(layout);
    }

    @Override
    protected void onStart()
    {
        Log.d(LOG_TAG, "DavaActivity.onStart");
        super.onStart();
    }

    @Override
    protected void onResume()
    {
        Log.d(LOG_TAG, "DavaActivity.onResume");
        super.onResume();

        handleResume();
    }

    @Override
    protected void onPause()
    {
        Log.d(LOG_TAG, "DavaActivity.onPause");
        super.onPause();

        keyboardState.stop();
        handlePause();
    }

    @Override
    protected void onRestart()
    {
        Log.d(LOG_TAG, "DavaActivity.onRestart");
        super.onRestart();
    }
    
    @Override
    protected void onStop()
    {
        Log.d(LOG_TAG, "DavaActivity.onStop");
        super.onStop();
    }

    @Override
    protected void onDestroy()
    {
        Log.d(LOG_TAG, "DavaActivity.onDestroy");
        super.onDestroy();

        isDestoying = true;
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
        bootstrapObjects.clear();
        activitySingleton = null;

        // Quit application when activity has been destroyed as:
        //  - it is hard to make java to unload shared library, it is necessary because:
        //    dava.engine is started when Activity.onCreate called and is ended on Activity.onDestroy
        //    but engine's shared library is not unloaded and all static variables preserve their
        //    values which leads to unpredictable behavior on next activity.onCreate call
        //  - same is applied to java classes
        Log.i(LOG_TAG, "Quitting application...");
        System.exit(0);
    }
    
    @Override
    public void onWindowFocusChanged(boolean hasWindowFocus)
    {
        Log.d(LOG_TAG, String.format("DavaActivity.onWindowFocusChanged: focus=%b", hasWindowFocus));

        hasFocus = hasWindowFocus;
        if (hasFocus)
        {
            keyboardState.start();
            hideNavigationBar();
            handleResume();
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig)
    {
        Log.d(LOG_TAG, "DavaActivity.onConfigurationChanged");
        super.onConfigurationChanged(newConfig);
    }

    @Override
    public void onBackPressed()
    {
        // Do not call base class method to prevent finishing activity
        // and application on back pressed
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event)
    {
        int keyCode = event.getKeyCode();
        // Ignore certain special keys so they're handled by Android
        // Stealed from SDL
        if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN ||
            keyCode == KeyEvent.KEYCODE_VOLUME_UP ||
            keyCode == KeyEvent.KEYCODE_CAMERA ||
            keyCode == KeyEvent.KEYCODE_ZOOM_IN ||
            keyCode == KeyEvent.KEYCODE_ZOOM_OUT)
        {
            return false;
        }
        return super.dispatchKeyEvent(event);
    }

    /**
     * Since API 19 we can hide Navigation bar (Immersive Full-Screen Mode)
     */
    public void hideNavigationBar()
    {
        View view = getWindow().getDecorView();
        int uiOptions = view.getSystemUiVisibility();
        
        // Navigation bar hiding: backward compatible with ICS.
        // Don't use View.SYSTEM_UI_FLAG_HIDE_NAVIGATION on API less that 19 because any
        // click on view shows navigation bar, and we must hide it manually only. It is
        // bad workflow.

        // Status bar hiding: backward compatible with Jellybean
        if (Build.VERSION.SDK_INT >= 16)
        {
            uiOptions |= 0x00000004; //View.SYSTEM_UI_FLAG_FULLSCREEN;
        }

        // Immersive mode: backward compatible with KitKat.
        // Note that this flag doesn't do anything by itself, it only augments the behavior
        // of HIDE_NAVIGATION and FLAG_FULLSCREEN.  For the purposes of this sample
        // all three flags are being toggled together.
        // Note that there are two immersive mode UI flags, one of which is referred to as "sticky".
        // Sticky immersive mode differs in that it makes the navigation and status bars
        // semi-transparent, and the UI flag does not get cleared when the user interacts with
        // the screen.
        if (Build.VERSION.SDK_INT >= 19)
        {
            uiOptions |= View.SYSTEM_UI_FLAG_HIDE_NAVIGATION 
                    | 0x00000200 //View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | 0x00000100 //View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                    | 0x00000400 //View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    | 0x00001000; //View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
        }
        view.setSystemUiVisibility(uiOptions);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    private void handleResume()
    {
        startDavaMainThreadIfNotRunning();
        if (isPaused && hasFocus)
        {
            isPaused = false;
            nativeOnResume();
            primarySurfaceView.handleResume();
        }
    }

    private void handlePause()
    {
        if (!isPaused)
        {
            isPaused = true;
            primarySurfaceView.handlePause();
            nativeOnPause();
        }
    }

    private void startDavaMainThreadIfNotRunning()
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
    
    private String getCommandLine()
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

    private void bootstrap()
    {
        // Read and load bootstrap library modules
        String[] modules = readTextFileFromRawDir("boot_modules");
        if (modules != null)
        {
            for (String m : modules)
            {
                Log.i(LOG_TAG, String.format("DavaActivity: loading bootstrap module '%s'", m));
                try {
                    System.loadLibrary(m);
                } catch (Throwable e) {
                    Log.e(DavaActivity.LOG_TAG, String.format("DavaActivity: module '%s' not loaded: %s", m, e.toString()));
                }
            }
        }

        // Read and create instances of bootstrap classes
        String[] classes = readTextFileFromRawDir("boot_classes");
        if (classes != null)
        {
            for (String c : classes)
            {
                Log.i(LOG_TAG, String.format("DavaActivity: instantiate bootstrap class '%s'", c));
                try {
                    Class<?> clazz = Class.forName(c);
                    Constructor<?> ctor = clazz.getConstructor();
                    Object obj = ctor.newInstance();
                    bootstrapObjects.add(obj);
                } catch (Throwable e) {
                    Log.e(DavaActivity.LOG_TAG, String.format("DavaActivity: class '%s' not instantiated: %s", c, e.toString()));
                }
            }
        }
    }

    private String[] readTextFileFromRawDir(String nameWithoutExtension)
    {
        try {
            int resourceId = getResources().getIdentifier(nameWithoutExtension, "raw", getPackageName());
            InputStream is = getResources().openRawResource(resourceId);
            BufferedReader br = new BufferedReader(new InputStreamReader(is));

            String curLine = null;
            List<String> lines = new ArrayList<String>();
            while ((curLine = br.readLine()) != null)
            {
                lines.add(curLine);
            }
            br.close();
            return lines.toArray(new String[lines.size()]);
        } catch (Exception e) {
            Log.e(LOG_TAG, String.format("DavaActivity.readTextFileFromRawDir: file '%s': %s", nameWithoutExtension, e.toString()));
        }
        return null;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    // View.OnSystemUiVisibilityChangeListener interface
    @Override
    public void onSystemUiVisibilityChange(int visibility)
    {
        hideNavigationBar();
    }
}
