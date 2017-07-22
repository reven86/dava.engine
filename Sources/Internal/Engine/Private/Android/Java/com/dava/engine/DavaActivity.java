package com.dava.engine;

import android.app.Activity;
import android.app.AlarmManager;
import android.app.Application;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Build;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;

import java.lang.reflect.Constructor;
import java.util.ArrayList;
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
    and register it to receive callbacks about activity events (onCreate, onPause, etc).
    Application usually has its own set of java classes and native shared libraries which should be instantiated/loaded on program start.
    Application can list them in `AndroidManifest.xml` under special `meta-data` tag. In the following sample dava.engine will try to load
    `libcrystax.so`, `libc++_shared.so` and `libTestBed.so` and try to instantiate java class `com.dava.testbed.TestBed`:
    ~~~~~~{.xml}
    <application>
        ...
        <meta-data android:name="com.dava.engine.BootModules" android:value="crystax;c++_shared;TestBed"/>
        <meta-data android:name="com.dava.engine.BootClasses" android:value="com.dava.testbed.TestBed"/>
    <application>
    ~~~~~~

    To show a splash image client application should have a `meta-data` tag inside of AndroidManifest.xml which specifies what resource to use:
    ~~~~~~{.xml}
    <application>
        ...
        <meta-data android:name="com.dava.engine.SplashImageId" android:resource="@drawable/splash_picture"/>
    <application>
    ~~~~~~

    <b>Note.</b> Now dava.engine is built by Crystax NDK so list of shared libraries must contain `crystax` and `c++_shared`
    before any other libraries.
*/
public final class DavaActivity extends Activity
                                implements View.OnSystemUiVisibilityChangeListener
{
    /**
        Interface definition for callbacks to be invoked when DavaActivity event occurs (onCreate, onPause, etc).
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
        public void onSaveInstanceState(Bundle outState);
        public void onActivityResult(int requestCode, int resultCode, Intent data);
        public void onNewIntent(Intent intent);
        public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults);
    }

    /**
        Class that implements ActivityListener interface with empty method implementations to permit override
        only necessary methods.
    */
    public static abstract class ActivityListenerImpl implements ActivityListener
    {
        public void onCreate(Bundle savedInstanceState) {}
        public void onStart() {}
        public void onResume() {}
        public void onPause() {}
        public void onRestart() {}
        public void onStop() {}
        public void onDestroy() {}
        public void onSaveInstanceState(Bundle outState) {}
        public void onActivityResult(int requestCode, int resultCode, Intent data) {}
        public void onNewIntent(Intent intent) {}
        public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {}
    }

    // Helper class to hold arguments for calling listener's onActivityResult
    private static class ActivityResultArgs
    {
        public int requestCode;
        public int resultCode;
        public Intent data;
    }

    // Helper class to hold arguments for calling listener's onPermissionResult
    private static class RequestPermissionResultArgs
    {
        public int requestCode;
        public String[] permissions;
        public int[] grantResults;
    }

    public static final String LOG_TAG = "DAVA"; //!< Tag used by dava.engine java classes for internal log outputs

    private static final String META_TAG_SPLASH_IMAGE = "com.dava.engine.SplashViewImageId";
    private static final String META_TAG_BOOT_MODULES = "com.dava.engine.BootModules";
    private static final String META_TAG_BOOT_CLASSES = "com.dava.engine.BootClasses";

    private static DavaActivity activitySingleton;
    private static Thread nativeThread; // Thread where native C++ code is running
    private static int nativeThreadId; // C++ native thread id
    private static int uiThreadId; // UI thread id

    protected boolean isStopped = true; // Activity is stopped after onStop and before onStart
    protected boolean isPaused = false; // Activity is paused after onPause and before onResume
    protected boolean isFocused = false;
    protected boolean isEngineRunning = false; // Engine has entered Run method in c++ thread and ready to pump MainDispatcher events
    
    protected String externalFilesDir;
    protected String internalFilesDir;
    protected String sourceDir;
    protected String packageName;
    protected String cmdline;

    protected DavaCommandHandler commandHandler = new DavaCommandHandler();
    protected DavaKeyboardState keyboardState = null;
    protected DavaGamepadManager gamepadManager = null;
    protected DavaGlobalLayoutState globalLayoutState = null;

    // List of class instances created during bootstrap (using meta-tag in AndroidManifest)
    protected LinkedList<Object> bootstrapObjects = new LinkedList<Object>();

    private DavaSurfaceView primarySurfaceView;
    private DavaSplashView splashView;
    private ViewGroup layout;
    private ArrayList<ActivityListener> activityListeners = new ArrayList<ActivityListener>();
    private ArrayList<ActivityResultArgs> savedActivityResultArgs = null; // Saved arguments coming from onActivityResult (possible multiple calls)

    private static final int ON_ACTIVITY_CREATE = 0;
    private static final int ON_ACTIVITY_START = 1;
    private static final int ON_ACTIVITY_RESUME = 2;
    private static final int ON_ACTIVITY_PAUSE = 3;
    private static final int ON_ACTIVITY_RESTART = 4;
    private static final int ON_ACTIVITY_STOP = 5;
    private static final int ON_ACTIVITY_DESTROY = 6;
    private static final int ON_ACTIVITY_SAVE_INSTANCE_STATE = 7;
    private static final int ON_ACTIVITY_RESULT = 8;
    private static final int ON_ACTIVITY_NEW_INTENT = 9;
    private static final int ON_ACTIVITY_REQUEST_PERMISSION_RESULT = 10;

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
        dava.engine creates and uses only one `Activity`-derived class.
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

    /** Check whether current thread is UI thread */
    public static boolean isUIThread()
    {
        return android.os.Process.myTid() == uiThreadId;
    }

    /** Check whether current thread is main native thread where C++ code lives */
    public static boolean isNativeMainThread()
    {
        return android.os.Process.myTid() == nativeThreadId;
    }

    /**
        Register a callback to be invoked when DavaActivity lifecycle event occurs.

        Application can register a callback from any thread, but callbacks are invoked in the context of UI thread.
    */
    synchronized public void registerActivityListener(ActivityListener l)
    {
        if (l != null && !activityListeners.contains(l))
        {
            activityListeners.add(l);
        }
    }

    /**
        Unregister a callback previously registered by `registerActivityListener` function.

        Application can unregister a callback from any thread, even during callback invocation.
    */
    synchronized public void unregisterActivityListener(ActivityListener l)
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

    private void restart()
    {
        try {
            Intent intent = new Intent(this, DavaActivity.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);

            int pendingIntentId = 223344;
            PendingIntent pendingIntent = PendingIntent.getActivity(this, pendingIntentId, intent, PendingIntent.FLAG_CANCEL_CURRENT);
            AlarmManager alarmManager = (AlarmManager)getSystemService(Context.ALARM_SERVICE);
            alarmManager.set(AlarmManager.RTC, System.currentTimeMillis() + 100, pendingIntent);
            System.exit(0);
        } catch (Exception e) {
            DavaLog.e(LOG_TAG, String.format("DavaActivity.restart failed: %s", e.toString()));
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        if (activitySingleton != null)
        {
            // dava.engine's is tightly bound to Activity lifecycle events. But in some cases on some devices Activity.onDestroy
            // does not arrive when Activity is removed by system from memory while application with loaded shared libraries is
            // still running. Later system may recreate Activity and Activity.onCreate handler tries to initialize again already
            // running dava.engine and game which may lead to crash or unpredictable behaviour.
            // Solution is to restart application.
            DavaLog.e(LOG_TAG, "DavaActivity.onCreate: restarting");
            activitySingleton = null;
            restart();
        }

        DavaLog.i(LOG_TAG, "DavaActivity.onCreate");

        activitySingleton = this;
        uiThreadId = android.os.Process.myTid();
        
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

        // Load & show splash view
        Bitmap splashViewBitmap =  loadSplashViewBitmap();
        splashView = new DavaSplashView(this, splashViewBitmap);

        // #1 create splash view
        layout = new FrameLayout(this);
        layout.addView(splashView);
        setContentView(layout);

        // #2 Load library modules and create class instances specified under meta-data tag in AndroidManifest.xml
        bootstrap();

        // #3 Initialize engine and run its onCreate method
        nativeInitializeEngine(externalFilesDir, internalFilesDir, sourceDir, packageName, cmdline);
        long nativePrimaryWindowBackend = nativeOnCreate(this);
        // Create primary DavaSurfaceView in advance but add to view hierarchy later when DavaSplashView will do its work
        primarySurfaceView = new DavaSurfaceView(getApplication(), nativePrimaryWindowBackend);

        notifyListeners(ON_ACTIVITY_CREATE, savedInstanceState);

        super.onCreate(savedInstanceState);
    }

    private Bitmap loadSplashViewBitmap()
    {
        Bitmap splashViewBitmap = null;

        try
        {
            ApplicationInfo appMetaDataInfo = getPackageManager().getApplicationInfo(packageName, PackageManager.GET_META_DATA);
            if (appMetaDataInfo != null)
            {
                int splashViewImageId = appMetaDataInfo.metaData.getInt(META_TAG_SPLASH_IMAGE, 0);
                if (splashViewImageId != 0)
                {
                    splashViewBitmap = BitmapFactory.decodeResource(getResources(), splashViewImageId);
                }
            }
        }
        catch (Exception e)
        {
            DavaLog.e(LOG_TAG, String.format("DavaActivity: loading splash image failed: %s. Splash view will be empty", e.toString()));
        }

        return splashViewBitmap;
    }
    
    private void continueOnCreate()
    {
        // #4 add primary DavaSurfaceView to view hierarchy
        layout.addView(primarySurfaceView);

        gamepadManager = new DavaGamepadManager();
        registerActivityListener(gamepadManager);

        globalLayoutState = new DavaGlobalLayoutState();
        registerActivityListener(globalLayoutState);

        keyboardState = new DavaKeyboardState();
        registerActivityListener(keyboardState);
    }

    protected void onSplashFinishedCollectingDeviceInfo()
    {
        // On some devices step #1 can cause this callback
        // to be synchronously invoked. In this case onCreate hasn't
        // been finished yet. To ensure that we call `continueOnCreate`
        // only when onCreate is finished we must put Runnable into commandHandler
        // and it will be run on next java-message-queue processing step.
        commandHandler.post(new Runnable(){
            @Override
            public void run() {
                continueOnCreate();
            }
        });
    }

    void onFinishCreatingMainWindowSurface()
    {
        startNativeThreadIfNotRunning();
    }

    @Override
    protected void onStart()
    {
        DavaLog.i(LOG_TAG, "DavaActivity.onStart");
        super.onStart();

        isStopped = false;
        
        notifyListeners(ON_ACTIVITY_START, null);
    }

    @Override
    protected void onResume()
    {
        DavaLog.i(LOG_TAG, "DavaActivity.onResume");
        super.onResume();

        handleResume();
        notifyListeners(ON_ACTIVITY_RESUME, null);
    }

    @Override
    protected void onPause()
    {
        DavaLog.i(LOG_TAG, "DavaActivity.onPause");
        super.onPause();

        handlePause();
        notifyListeners(ON_ACTIVITY_PAUSE, null);
    }

    @Override
    protected void onRestart()
    {
        DavaLog.i(LOG_TAG, "DavaActivity.onRestart");
        super.onRestart();

        notifyListeners(ON_ACTIVITY_RESTART, null);
    }
    
    @Override
    protected void onStop()
    {
        DavaLog.i(LOG_TAG, "DavaActivity.onStop");
        super.onStop();
        
        isStopped = true;

        notifyListeners(ON_ACTIVITY_STOP, null);
    }

    @Override
    protected void onDestroy()
    {
        DavaLog.i(LOG_TAG, "DavaActivity.onDestroy");
        super.onDestroy();

        notifyListeners(ON_ACTIVITY_DESTROY, null);
        activityListeners.clear();

        if (isEngineRunning)
        {
            DavaLog.i(LOG_TAG, "DavaActivity.nativeOnDestroy");
            nativeOnDestroy();
            if (isNativeThreadRunning())
            {
                try {
                    DavaLog.i(LOG_TAG, "Joining native thread");
                    nativeThread.join();
                } catch (Exception e) {
                    DavaLog.e(LOG_TAG, "DavaActivity.onDestroy: davaMainThread.join() failed " + e);
                }
                nativeThread = null;
            }
            DavaLog.i(LOG_TAG, "DavaActivity.nativeShutdownEngine");
            nativeShutdownEngine();
        }

        bootstrapObjects.clear();
        activitySingleton = null;

        // Quit application when activity has been destroyed as:
        //  - it is hard to make java to unload shared library, it is necessary because:
        //    dava.engine is started when Activity.onCreate called and is ended on Activity.onDestroy
        //    but engine's shared library is not unloaded and all static variables preserve their
        //    values which leads to unpredictable behavior on next activity.onCreate call
        //  - same is applied to java classes
        DavaLog.i(LOG_TAG, "Quitting application...");
        System.exit(0);
    }
    
    @Override
    public void onWindowFocusChanged(boolean hasWindowFocus)
    {
        DavaLog.i(LOG_TAG, String.format("DavaActivity.onWindowFocusChanged: focus=%b", hasWindowFocus));

        isFocused = hasWindowFocus;
        if (isFocused)
        {
            hideNavigationBar();
            handleResume();
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig)
    {
        DavaLog.i(LOG_TAG, "DavaActivity.onConfigurationChanged");
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

    @Override
    protected void onSaveInstanceState(Bundle outState)
    {
        super.onSaveInstanceState(outState);
        notifyListeners(ON_ACTIVITY_SAVE_INSTANCE_STATE, null);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        super.onActivityResult(requestCode, resultCode, data);

        ActivityResultArgs args = new ActivityResultArgs();
        args.requestCode = requestCode;
        args.resultCode = resultCode;
        args.data = data;
        if (isEngineRunning)
        {
            notifyListeners(ON_ACTIVITY_RESULT, args);
        }
        else
        {
            // Save arguments and notify listeners later when Engine will enter Run method.
            // Reason: onActivityResult can be called before native thread has started, e.g.:
            //  - application starts Google Services activity,
            //  - user puts application into background (presses Home button),
            //  - system silently kill application to free some memory,
            //  - onActivityResult is invoked before c++ thread has started.
            if (savedActivityResultArgs == null)
            {
                savedActivityResultArgs = new ArrayList<ActivityResultArgs>();
            }
            savedActivityResultArgs.add(args);
        }
    }

    @Override
    protected void onNewIntent(Intent intent)
    {
        super.onNewIntent(intent);
        notifyListeners(ON_ACTIVITY_NEW_INTENT, intent);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults)
    {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        RequestPermissionResultArgs args = new RequestPermissionResultArgs();
        args.requestCode = requestCode;
        args.permissions = permissions;
        args.grantResults = grantResults;
        notifyListeners(ON_ACTIVITY_REQUEST_PERMISSION_RESULT, args);
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

    // Will be invoked from C++ thread right before game loop is started
    private void hideSplashView()
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                if (splashView != null)
                {
                    layout.removeView(splashView);
                    splashView.cleanup();
                    splashView = null;
                }
            }
        });

    }

    // Invoked from c++ thread and tells that Engine has entered Run method
    private void notifyEngineRunning()
    {
        runOnUiThread(new Runnable()
        {
            @Override public void run()
            {
                isEngineRunning = true;

                // Notify about onActivityResult if it has occured (see comments in onActivityResult method)
                if (savedActivityResultArgs != null)
                {
                    for (ActivityResultArgs args : savedActivityResultArgs)
                    {
                        notifyListeners(ON_ACTIVITY_RESULT, args);
                    }
                    savedActivityResultArgs = null;
                }
            }
        });
    }

    public boolean isPaused()
    {
        return isPaused;
    }
    
    public boolean isStopped()
    {
        return isStopped;
    }

    public boolean isFocused()
    {
        return isFocused;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void handleResume()
    {
        if (primarySurfaceView.isSurfaceReady() && isPaused && isFocused)
        {
            DavaLog.i(LOG_TAG, "DavaActivity.handleResume");

            isPaused = false;
            nativeOnResume();
            primarySurfaceView.onResume();
        }
    }

    void handlePause()
    {
        if (primarySurfaceView.isSurfaceReady() && !isPaused)
        {
            DavaLog.i(LOG_TAG, "DavaActivity.handlePause");

            isPaused = true;
            primarySurfaceView.onPause();
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
                    nativeThreadId = android.os.Process.myTid();
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
        String[] modules = getBootMetadata(META_TAG_BOOT_MODULES);
        if (modules != null)
        {
            for (String m : modules)
            {
                if (!m.isEmpty())
                {
                    DavaLog.i(LOG_TAG, String.format("DavaActivity: loading bootstrap module '%s'", m));
                    try {
                        System.loadLibrary(m);
                        nloaded += 1;
                    } catch (Throwable e) {
                        DavaLog.e(DavaActivity.LOG_TAG, String.format("DavaActivity: module '%s' not loaded: %s", m, e.toString()));
                    }
                }
            }
        }
        if (nloaded == 0)
        {
            // Issue warning if no boot modules were loaded as usually all logic is contained
            // in shared libraries written in C/C++
            DavaLog.w(LOG_TAG, "DavaActivity: no bootstrap modules loaded!!! Maybe you forgot to add meta-data tag with module list to AndroidManifest.xml");
        }

        // Read and create instances of bootstrap classes
        // Do not issue warning if no classes instantiated as application may not require own java part
        String[] classes = getBootMetadata(META_TAG_BOOT_CLASSES);
        if (classes != null)
        {
            for (String c : classes)
            {
                if (!c.isEmpty())
                {
                    DavaLog.i(LOG_TAG, String.format("DavaActivity: instantiate bootstrap class '%s'", c));
                    try {
                        Class<?> clazz = Class.forName(c);
                        Constructor<?> ctor = clazz.getConstructor();
                        Object obj = ctor.newInstance();
                        bootstrapObjects.add(obj);
                    } catch (Throwable e) {
                        DavaLog.e(DavaActivity.LOG_TAG, String.format("DavaActivity: class '%s' not instantiated: %s", c, e.toString()));
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
            DavaLog.e(LOG_TAG, String.format("DavaActivity: get metadata for '%s' failed: %s", key, e.toString()));
        }
        return null;
    }

    void setScreenTimeoutEnabled(final boolean enabled)
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run() 
            {
                if (enabled)
                {
                    getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                }
                else
                {
                    getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                }
            }
        });
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
        // Make listeners copy to allow unregistering listeners inside callback
        ArrayList<ActivityListener> listenersCopy = new ArrayList<ActivityListener>();
        synchronized(this)
        {
            listenersCopy.addAll(activityListeners);
        }
        for (ActivityListener l : listenersCopy)
        {
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
            case ON_ACTIVITY_SAVE_INSTANCE_STATE:
                l.onSaveInstanceState((Bundle)arg);
                break;
            case ON_ACTIVITY_RESULT:
                ActivityResultArgs activityResultArgs = (ActivityResultArgs)arg;
                l.onActivityResult(activityResultArgs.requestCode, activityResultArgs.resultCode, activityResultArgs.data);
                break;
            case ON_ACTIVITY_NEW_INTENT:
                l.onNewIntent((Intent)arg);
                break;
            case ON_ACTIVITY_REQUEST_PERMISSION_RESULT:
                RequestPermissionResultArgs requestResultArgs = (RequestPermissionResultArgs)arg;
                l.onRequestPermissionsResult(requestResultArgs.requestCode, requestResultArgs.permissions, requestResultArgs.grantResults);
                break;
            }
        }
    }
}
