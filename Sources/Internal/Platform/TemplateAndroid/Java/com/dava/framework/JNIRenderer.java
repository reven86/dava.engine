package com.dava.framework;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.PowerManager;
import android.util.Log;

public class JNIRenderer implements GLSurfaceView.Renderer {

	private native void nativeResize(int w, int h);
	private native void nativeRender();
	private native void nativeRenderRecreated();
	private native void nativeOnResumeView();
	private native void nativeOnPauseView(boolean isLock);
	
	private boolean isFirstFrameAfterDraw = false;
	private long framesCounter = 0;

	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		Log.w(JNIConst.LOG_TAG, "Activity Render onSurfaceCreated started");
		
		JNIDeviceInfo.SetGPUFamily(gl);

		nativeRenderRecreated();

		LogExtensions();

		Log.w(JNIConst.LOG_TAG, "Activity Render onSurfaceCreated finished");
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
	    Log.w(JNIConst.LOG_TAG, "Activity Render onSurfaceChanged: w = " + w + " h = " + h + " start");
	    
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
            if (JNIActivity.GetActivity().isEglContextDestroyed())
            {
                nativeResize(w, h);
                JNIActivity.GetActivity().onEglContextCreated();
            }
            isFirstFrameAfterDraw = true; // Do we need this?
        }

        long endTime = System.nanoTime();
        long duration = (endTime - startTime) / 1000000L;  //divide by 1000000 to get milliseconds.
        
		Log.w(JNIConst.LOG_TAG, "Activity Render onSurfaceChanged finish " + duration + "ms");
	}
	
	public void skipRenderingWhileSplashQuicklyFirstRender()
	{
	    framesCounter = 0;
	}

	@Override
	public void onDrawFrame(GL10 gl) {
	    // skip first frame after resume, we want show on start splash view
	    // as quickly as possible, so skip first frame
	    if (framesCounter > 1)
	    {
    		nativeRender();
    		
    		if(isFirstFrameAfterDraw)
    		{
    			isFirstFrameAfterDraw = false;
    			JNIActivity.GetActivity().OnFirstFrameAfterDraw();
    			JNITextField.ShowVisibleTextFields();
    		}
	    }
	    ++framesCounter;
	}
	
	public void OnPause()
	{
		PowerManager pm = (PowerManager) JNIApplication.GetApplication().getSystemService(Context.POWER_SERVICE);
		nativeOnPauseView(!pm.isScreenOn());
		
		isFirstFrameAfterDraw = true;
		// reset counter for frames to quickly show splash on resume
		framesCounter = 0;
	}
	
	public void OnResume()
	{
	    // reset counter for frames to quickly show splash on resume
	    framesCounter = 0;
	    
		long startTime = System.nanoTime();
		nativeOnResumeView();
		long endTime = System.nanoTime();

		long duration = (endTime - startTime) / 1000000L;  //divide by 1000000 to get milliseconds.
		Log.i(JNIConst.LOG_TAG, "Activity Render nativeOnResumeView time " + duration + "ms");
	}
	
}
