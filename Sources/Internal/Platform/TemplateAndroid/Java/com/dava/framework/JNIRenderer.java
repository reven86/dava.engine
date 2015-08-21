package com.dava.framework;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.PowerManager;
import android.util.Log;

public class JNIRenderer implements GLSurfaceView.Renderer {

    private native void nativeResize(int w, int h);

    private native void nativeRender();

    private native void nativeRenderRecreated();

    private native void nativeOnResumeView();

    private native void nativeOnPauseView(boolean isLock);

    private int frameCounter = 0;
    
    private int cachedWidth = 0;
    private int cachedHeight = 0;
    
    public static volatile boolean nativeSingletonsDestroyed = false;

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        Log.d(JNIConst.LOG_TAG, "Activity Render onSurfaceCreated started");

        JNIDeviceInfo.SetGPUFamily(gl);
        
        // remember current thread (OpenGL) for future native controls
        long currentThreadId = Thread.currentThread().getId();
        JNIActivity.GetActivity().setGLThreadId(currentThreadId);

        nativeRenderRecreated();

        LogExtensions();
        
        cachedWidth = 0;
        cachedHeight = 0;

        Log.d(JNIConst.LOG_TAG, "Activity Render onSurfaceCreated finished");
    }

    private void LogExtensions() {
        String oglVersion = GLES20.glGetString(GLES20.GL_VERSION);
        String deviceName = GLES20.glGetString(GLES20.GL_RENDERER);
        String extensions = GLES20.glGetString(GLES20.GL_EXTENSIONS);

        Log.i(JNIConst.LOG_TAG, "[GLES_20] oglVersion is " + oglVersion);
        Log.i(JNIConst.LOG_TAG, "[GLES_20] deviceName is " + deviceName);
        Log.i(JNIConst.LOG_TAG, "[GLES_20] extensions is " + extensions);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int w, int h) {
        Log.d(JNIConst.LOG_TAG, "Activity Render onSurfaceChanged: w = " + w
                + " h = " + h + " start");

        long startTime = System.nanoTime();

        // while we always in landscape mode, but some devices
        // call this method on lock screen with portrait w and h
        // then call this method second time with correct portrait w and h
        // I assume width always more then height
        // we need skip portrait w and h because text in engine can be
        // pre render into texture with incorrect aspect and on second
        // call with correct w and h such text textures stays incorrect
        // http://stackoverflow.com/questions/8556332/is-it-safe-to-assume-that-in-landscape-mode-height-is-always-less-than-width
        // DF-5068
        // strange but after add to manifest.xml screenSize to config
        // on nexus 5 w == h == 1080
        // if you have any trouble here you should first check
        // res/layout/activity_main.xml and root layout is FrameLayout!
        if (w > h)
        {
            if (   w != cachedWidth
                || h != cachedHeight
                || JNIApplication.isEglContextWasDestroyed())
            {
                // nativeResize - recreate all shaders textures etc
                // long wait call
                Log.d(JNIConst.LOG_TAG, "Renderer call nativeResize(w, h)");
                nativeResize(w, h);
                JNIApplication.setEglContextWasDestroyed(false);
                
                cachedWidth = w;
                cachedHeight = h;
                
                // Workaround! we have to initialize keyboard after glView(OpenGL)
                // initialization for some devices like
                // HTC One (adreno 320, os 4.3)
                final JNIActivity activity = JNIActivity.GetActivity();
                activity.runOnUiThread(new Runnable(){
                    @Override
                    public void run() {
                        activity.InitKeyboardLayout();
                    }
                });
            }
            
        }

        long endTime = System.nanoTime();
        long duration = (endTime - startTime) / 1000000L; // divide by 1000000
                                                          // to get
                                                          // milliseconds.

        Log.d(JNIConst.LOG_TAG, "Activity Render onSurfaceChanged finish "
                + duration + "ms");
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        if (!JNIAssert.waitUserInputOnAssertDialog ||
            !nativeSingletonsDestroyed)
        {
            nativeRender();
            
            ++frameCounter;
            // Workaround wait 5 frames for render static text field to textures
            // and transition from one screen to another during lock/unlock
            // skip bad print screen texture
            if (5 == frameCounter)
            {
                JNIActivity.GetActivity().HideSplashScreenView();
            }
        }
    }

    public void OnPause() {
        Log.d(JNIConst.LOG_TAG, "Activity Render OnPause start");
        PowerManager pm = (PowerManager) JNIApplication.GetApplication()
                .getSystemService(Context.POWER_SERVICE);
        nativeOnPauseView(isScreenLocked(pm));

        frameCounter = 0;
        Log.d(JNIConst.LOG_TAG, "Activity Render OnPause finish");
    }

    @SuppressWarnings("deprecation")
    private boolean isScreenLocked(final PowerManager pm) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT_WATCH)
        {
            return !pm.isInteractive();
        } else
        {
            return !pm.isScreenOn();
        }
    }

    public void OnResume() {
        Log.d(JNIConst.LOG_TAG, "Activity Render OnResume start");
        nativeOnResumeView();
        Log.d(JNIConst.LOG_TAG, "Activity Render OnResume finish");
    }

}
