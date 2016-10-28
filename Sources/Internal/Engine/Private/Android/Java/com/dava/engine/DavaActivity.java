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
import java.util.Iterator;
import java.util.HashSet;
import java.util.LinkedList;

/**
    \ingroup engine
    DavaActivity is a primary class in dava.engine implementation for Android platform which is a subclass of `android.app.Activity`
    and should be marked as entry point of application in `AndroidManifest.xml`.
    
    `DavaActivity` is responsible for:
        - creating primary `DavaSurfaceView` class instance (subclass of `android.view.SurfaceView`),
        - starting and managing thread where native C++ code lives,
        - doing other housekeeping.

    By design client applications are not permited to extend `DavaActivity`. Instead they should implement `ActivityListener` interface
    and register it to receive callbacks about activity lifecycle events (onCreate, onPause, etc).
    Application usually has its own set of java classes and native shared libraries which should be instantiated/loaded on program start.
    Application can list them in `AndroidManifest.xml` under special `meta-data` tag. In the following sample dava.engine will try to load
    `libcrystax.so`, `libc++_shared.so` and `libTestBed.so` and try to instantiate java class `com.dava.testbed.TestBed`:

    ~~~~~~{.xml}
    <application>
        <meta-data android:name="boot_modules" android:value="crystax;c++_shared;TestBed"/>
        <meta-data android:name="boot_classes" android:value="com.dava.testbed.TestBed"/>
    <application>
    ~~~~~~

    <b>Note.</b> Now dava.engine is built by Crystax NDK so list of shared libraries must contain `crystax` and `c++_shared`
    before any other libraries.
*/
public final class DavaActivity extends Activity
                                implements View.OnSystemUiVisibilityChangeListener
{
    /**
        Interface definition for a callbacks to be invoked when DavaActivity lifecycle event occurs (onCreate, onPause, etc).
        Use registerActivityListener and unregisterActivityListener methods to install/uninstall listeners.
    */
    public interface ActivityListener
    {
        public void onCreate(Bundle savedInstanceState);
        public void onStart();
        public void onResume();
        public void onPause();
        public void onRestart();
        public void onStop();
        public void onDestroy();
    }

    /**
        Class that implements ActivityListener interface with empty method implementations to permit override
        only necessary methods.
    */
    public abstract class ActivityListenerImpl implements ActivityListener
    {
        public void onCreate(Bundle savedInstanceState) {}
        public void onStart() {}
        public void onResume() {}
        public void onPause() {}
        public void onRestart() {}
        public void onStop() {}
        public void onDestroy() {}
    }

    public static final String LOG_TAG = "DAVA"; //!< Tag used by dava.engine java classes for internal log outputs

    private static DavaActivity activitySingleton;
    private static Thread nativeThread; // Thread where native C++ code is running

    protected boolean isPaused = true;
    protected boolean hasFocus = false;
    
    protected String externalFilesDir;
    protected String internalFilesDir;
    protected String sourceDir;
    protected String packageName;
    protected String cmdline;

    protected DavaCommandHandler commandHandler = new DavaCommandHandler();
    protected DavaKeyboardState keyboardState = new DavaKeyboardState();
    protected DavaGamepadManager gamepadManager = new DavaGamepadManager();
    // List of class instances loaded from boot_classes files on startup
    protected LinkedList<Object> bootstrapObjects = new LinkedList<Object>();

    private DavaSurfaceView primarySurfaceView;
    private DavaSplashView splashView;
    private ViewGroup layout;
    private HashSet<ActivityListener> activityListeners = new HashSet<ActivityListener>();
    private Bundle savedInstanceStateBundle;

    private static final int ON_ACTIVITY_CREATE = 0;
    private static final int ON_ACTIVITY_START = 1;
    private static final int ON_ACTIVITY_RESUME = 2;
    private static final int ON_ACTIVITY_PAUSE = 3;
    private static final int ON_ACTIVITY_RESTART = 4;
    private static final int ON_ACTIVITY_STOP = 5;
    private static final int ON_ACTIVITY_DESTROY = 6;

    public static native void nativeInitializeEngine(String externalFilesDir,
                                                     String internalFilesDir,
                                                     String appPath,
                                                     String packageName,
                                                     String cmdline);
    public static native void nativeShutdownEngine();
    public static native long nativeOnCreate(DavaActivity activity);
    public static native void nativeOnResume();
    public static native void nativeOnPause();
    public static native void nativeOnDestroy();
    public static native void nativeGameThread();

    /**
        Get `DavaActivity` instance.
        dava.engine creates and use only one `Activity`-derived class.
    */
    public static DavaActivity instance()
    {
        return activitySingleton;
    }

    /** Check whether native thread is running. */
    public static boolean isNativeThreadRunning()
    {
        return nativeThread != null;
    }

    /**
        Register a callback to be invoked when DavaActivity lifecycle event occurs.
    */
    public void registerActivityListener(ActivityListener l)
    {
        if (l != null && !activityListeners.contains(l))
        {
            activityListeners.add(l);
        }
    }

    /**
        Unregister activity listener.
    */
    public void unregisterActivityListener(ActivityListener l)
    {
        activityListeners.remove(l);
    }

    static DavaCommandHandler commandHandler()
    {
        return activitySingleton.commandHandler;
    }

    static DavaGamepadManager gamepadManager()
    {
        return activitySingleton.gamepadManager;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        Log.d(LOG_TAG, "DavaActivity.onCreate");
        super.onCreate(savedInstanceState);
        
        activitySingleton = this;
        savedInstanceStateBundle = savedInstanceState;
        
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
        
        nativeInitializeEngine(externalFilesDir, internalFilesDir, sourceDir, packageName, cmdline);
        
        long primaryWindowBackendPointer = nativeOnCreate(this);
        primarySurfaceView = new DavaSurfaceView(getApplication(), primaryWindowBackendPointer);
        layout.addView(primarySurfaceView);

        notifyListeners(ON_ACTIVITY_CREATE, savedInstanceStateBundle);
        savedInstanceStateBundle = null;
    }
    
    protected void onFinishCollectDeviceInfo()
    {
        runOnUiThread(new Runnable(){
            @Override
            public void run() {
                startNativeInitialization();
                // now wait till SurfaceView will be created to continue
            }
        });
    }
    
    void onFinishCreatingMainWindowSurface()
    {
        runOnUiThread(new Runnable(){
            @Override
            public void run() {
                startNativeThreadIfNotRunning();
                handleResume();
                layout.removeView(splashView);
                splashView = null;
            }
        });
    }

    @Override
    protected void onStart()
    {
        Log.d(LOG_TAG, "DavaActivity.onStart");
        super.onStart();

        notifyListeners(ON_ACTIVITY_START, null);
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

        notifyListeners(ON_ACTIVITY_RESTART, null);
    }
    
    @Override
    protected void onStop()
    {
        Log.d(LOG_TAG, "DavaActivity.onStop");
        super.onStop();

        notifyListeners(ON_ACTIVITY_STOP, null);
    }

    @Override
    protected void onDestroy()
    {
        Log.d(LOG_TAG, "DavaActivity.onDestroy");
        super.onDestroy();

        notifyListeners(ON_ACTIVITY_DESTROY, null);

        nativeOnDestroy();
        if (isNativeThreadRunning())
        {
            try {
                nativeThread.join();
            } catch (Exception e) {
                Log.e(LOG_TAG, "DavaActivity.onDestroy: davaMainThread.join() failed " + e);
            }
            nativeThread = null;
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

    void hideNavigationBar()
    {
        // Since API 19 we can hide Navigation bar (Immersive Full-Screen Mode)
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

    public void postQuit()
    {
        commandHandler.sendQuit();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    private void handleResume()
    {
        if (primarySurfaceView != null && isNativeThreadRunning())
        {
            if (isPaused && hasFocus)
            {
                isPaused = false;
                nativeOnResume();

                notifyListeners(ON_ACTIVITY_RESUME, null);

                primarySurfaceView.handleResume();
                gamepadManager.onResume();
            }
        }
    }

    private void handlePause()
    {
        if (!isPaused)
        {
            isPaused = true;
            primarySurfaceView.handlePause();
            gamepadManager.onPause();

            notifyListeners(ON_ACTIVITY_PAUSE, null);

            nativeOnPause();
        }
    }

    private void startNativeThreadIfNotRunning()
    {
        if (!isNativeThreadRunning())
        {
            nativeThread = new Thread(new Runnable() {
                @Override public void run()
                {
                    nativeGameThread();
                }
            }, "DAVA main thread");
            nativeThread.start();
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

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    private void notifyListeners(int what, Object arg)
    {
        for (Iterator<ActivityListener> i = activityListeners.iterator();i.hasNext();)
        {
            ActivityListener l = i.next();
            switch (what)
            {
            case ON_ACTIVITY_CREATE:
                l.onCreate((Bundle)arg);
                break;
            case ON_ACTIVITY_START:
                l.onStart();
                break;
            case ON_ACTIVITY_RESUME:
                l.onResume();
                break;
            case ON_ACTIVITY_PAUSE:
                l.onPause();
                break;
            case ON_ACTIVITY_RESTART:
                l.onRestart();
                break;
            case ON_ACTIVITY_STOP:
                l.onStop();
                break;
            case ON_ACTIVITY_DESTROY:
                l.onDestroy();
                break;
            }
        }
    }
}
