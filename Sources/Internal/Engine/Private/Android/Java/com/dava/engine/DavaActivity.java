package com.dava.engine;

import android.app.Activity;
import android.app.Application;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
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
import java.util.LinkedList;
import java.util.List;

import com.dava.framework.JNIDeviceInfo;

public final class DavaActivity extends Activity
                                implements View.OnSystemUiVisibilityChangeListener
{
    public static final String LOG_TAG = "DAVA";

    private static DavaActivity activitySingleton;
    private static Thread davaMainThread;

    protected boolean isPaused = true;
    protected boolean hasFocus = false;
    protected boolean isDestoying = false;
    
    protected String externalFilesDir;
    protected String internalFilesDir;
    protected String sourceDir;
    protected String packageName;
    protected String cmdline;
    
    private int displayWidth = 0;
    private int displayHeight = 0;

    protected DavaCommandHandler commandHandler = new DavaCommandHandler();
    protected DavaKeyboardState keyboardState = new DavaKeyboardState();
    // List of class instances loaded from boot_classes files on startup
    protected List<Object> bootstrapObjects = new LinkedList<Object>();

    private DavaSurfaceView primarySurfaceView;
    private DavaSplashView splashView;
    private ViewGroup layout;

    public static native void nativeInitializeEngine(String externalFilesDir,
                                                     String internalFilesDir,
                                                     String appPath,
                                                     String packageName,
                                                     String cmdline,
                                                     int defaultDisplayWidth,
                                                     int defaultDisplayHeight);
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
        
        Application app = getApplication();
        externalFilesDir = app.getExternalFilesDir(null).getAbsolutePath() + "/";
        internalFilesDir = app.getFilesDir().getAbsolutePath() + "/";
        sourceDir = app.getApplicationInfo().publicSourceDir;
        packageName = app.getApplicationInfo().packageName;
        cmdline = getCommandLine();
        
        Window window = getWindow();
        window.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_NOTHING | WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
        window.addFlags(WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN | WindowManager.LayoutParams.FLAG_FULLSCREEN);
        window.requestFeature(Window.FEATURE_ACTION_MODE_OVERLAY);
        window.requestFeature(Window.FEATURE_ACTION_BAR_OVERLAY);
        window.requestFeature(Window.FEATURE_NO_TITLE);
        window.getDecorView().setOnSystemUiVisibilityChangeListener(this);
        hideNavigationBar();
        
        splashView = new DavaSplashView(this);
        
        layout = new FrameLayout(this);
        layout.addView(splashView);
        setContentView(layout);
    }
    
    private void startNativeInitialization() {
        // Load library modules and create class instances specified under meta-data tag
        // in AndroidManifest.xml with names boot_modules and boot_classes accordingly
        bootstrap();
        
        displayWidth = JNIDeviceInfo.GetDefaultDisplayWidth();
        displayHeight = JNIDeviceInfo.GetDefaultDisplayHeight();
        
        nativeInitializeEngine(externalFilesDir, internalFilesDir, sourceDir, packageName, cmdline, displayWidth, displayHeight);
        
        long primaryWindowBackendPointer = nativeOnCreate();
        primarySurfaceView = new DavaSurfaceView(getApplication(), primaryWindowBackendPointer);
        layout.addView(primarySurfaceView);
    }
    
    public void onFinishCollectDeviceInfo()
    {
        runOnUiThread(new Runnable(){
            @Override
            public void run() {
                startNativeInitialization();
                // now wait till SurfaceView will be created to continue
            }
        });
    }
    
    public void onFinishCreatingMainWindowSurface()
    {
        runOnUiThread(new Runnable(){
            @Override
            public void run() {
                startDavaMainThreadIfNotRunning();
                handleResume();
                layout.removeView(splashView);
            }
        });
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
        if (primarySurfaceView != null && davaMainThread != null)
        {
            if (isPaused && hasFocus) {
                isPaused = false;
                nativeOnResume();
                primarySurfaceView.handleResume();
            }
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
                Object value = extras.get(key);
                result += key + " " + value.toString() + " ";
            }
        }
        return result;
    }

    private void bootstrap()
    {
        // Read and load bootstrap library modules
        int nloaded = 0;
        String[] modules = getBootMetadata("boot_modules");
        if (modules != null)
        {
            for (String m : modules)
            {
                if (!m.isEmpty())
                {
                    Log.i(LOG_TAG, String.format("DavaActivity: loading bootstrap module '%s'", m));
                    try {
                        System.loadLibrary(m);
                        nloaded += 1;
                    } catch (Throwable e) {
                        Log.e(DavaActivity.LOG_TAG, String.format("DavaActivity: module '%s' not loaded: %s", m, e.toString()));
                    }
                }
            }
        }
        if (nloaded == 0)
        {
            // Issue warning if no boot modules were loaded as usually all logic is contained
            // in shared libraries written in C/C++
            Log.w(LOG_TAG, "DavaActivity: no bootstrap modules loaded!!! Maybe you forgot to add meta-data tag with module list to AndroidManifest.xml");
        }

        // Read and create instances of bootstrap classes
        // Do not issue warning if no classes instantiated as application may not require own java part
        String[] classes = getBootMetadata("boot_classes");
        if (classes != null)
        {
            for (String c : classes)
            {
                if (!c.isEmpty())
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
    }
    
    private String[] getBootMetadata(String key)
    {
        try {
            ApplicationInfo appInfo = getPackageManager().getApplicationInfo(packageName, PackageManager.GET_META_DATA);
            String s = appInfo.metaData.getString(key);
            if (s != null)
            {
                return s.split(";");
            }
        } catch (Exception e) {
            Log.e(LOG_TAG, String.format("DavaActivity: get metadata for '%s' failed: %s", key, e.toString()));
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
