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

    private boolean isFirstFrameAfterDraw = false;
    private boolean skipResumeAfterPortraitSurfaceChange = false;
    private long framesCounter = 0;

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        Log.d(JNIConst.LOG_TAG, "Activity Render onSurfaceCreated started");

        JNIDeviceInfo.SetGPUFamily(gl);

        nativeRenderRecreated();

        LogExtensions();

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
        if (w > h) {
            // nativeResize call core->RenderRecreated(w, h); in c++
            // it take 2.5 seconds on samsung galaxy S 3 so
            // check if eglContext not recreated and skip this step
            final JNIActivity activity = JNIActivity.GetActivity();
            assert (activity != null);
            if (activity.isEglContextDestroyed()) {
                nativeResize(w, h);
                activity.onEglContextCreated();
            }
            nativeOnResumeView();
            isFirstFrameAfterDraw = true; // Do we need this?
        } else
        {
            skipResumeAfterPortraitSurfaceChange = true;
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
        // if we need quickly show splash screen, we can try do it three ways:
        // 1. skip first render frame in glView, so only splashView is visible
        // 2. skip resume glView in Activity resume and resume it later
        // in Activity.onWindowsFocusChande(true)
        // 3. set splash in styles.xml and AndroidManifest (not worked)
        // Workaround:
        // a) if you choose 1 way:
        // game will crush in PushNotificationBridgeImplAndroid.cpp line 15
        // assert
        // b) if you choose 2 way and remove skip first frame:
        // video intro on game start will be skipped (you will see several
        // video frames)
        if (!JNIAssert.waitUserInputOnAssertDialog)
        {
            if (framesCounter > 0) {
                nativeRender();
    
                if (isFirstFrameAfterDraw) {
                    isFirstFrameAfterDraw = false;
                    JNIActivity.GetActivity().OnFirstFrameAfterDraw();
                    JNITextField.ShowVisibleTextFields();
                }
            }
            ++framesCounter;
        }
    }

    public void OnPause() {
        Log.d(JNIConst.LOG_TAG, "Activity Render OnPause start");
        PowerManager pm = (PowerManager) JNIApplication.GetApplication()
                .getSystemService(Context.POWER_SERVICE);
        nativeOnPauseView(isScreenLocked(pm));

        isFirstFrameAfterDraw = true;
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
        // reset counter for frames to quickly show splash on resume
        framesCounter = 0;

        long startTime = System.nanoTime();
        if (skipResumeAfterPortraitSurfaceChange)
        {
            skipResumeAfterPortraitSurfaceChange = false;
        } else
        {
            nativeOnResumeView();
        }
        long endTime = System.nanoTime();

        long duration = (endTime - startTime) / 1000000L; // divide by 1000000
                                                          // to get
                                                          // milliseconds.
        Log.d(JNIConst.LOG_TAG, "Activity Render OnResume finish time: " 
                                                          + duration + "ms");
    }

}
