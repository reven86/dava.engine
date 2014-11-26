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
	
	private boolean isRenderRecreated = false;

	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		Log.w(JNIConst.LOG_TAG, "_________onSurfaceCreated_____!!!!_____");
		
		JNIDeviceInfo.SetGPUFamily(gl);

		nativeRenderRecreated();

		LogExtensions();

		Log.w(JNIConst.LOG_TAG, "_________onSurfaceCreated_____DONE_____");
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
	    // while we always in landscape mode, but some devices 
        // call this method on lock screen with portrait w and h
        // then call this method second time with correct portrait w and h
        // I assume width always more then height
	    // we need skip portrait w and h because text in engine can be
	    // pre render into texture with incorrect aspect and on second
	    // call with correct w and h such text textures stays incorrect
        // http://stackoverflow.com/questions/8556332/is-it-safe-to-assume-that-in-landscape-mode-height-is-always-less-than-width
	    // https://jira.wargaming.net/browse/DF-5068
        if (w > h)
        {
            nativeResize(w, h);
            nativeOnResumeView();
            isFirstFrameAfterDraw = true; // Do we need this?
        }

		Log.w(JNIConst.LOG_TAG, "Activity Render onSurfaceChanged finish");
	}

	@Override
	public void onDrawFrame(GL10 gl) {
		nativeRender();
		
		if(isFirstFrameAfterDraw)
		{
			isFirstFrameAfterDraw = false;
			JNIActivity.GetActivity().OnFirstFrameAfterDraw();
			JNITextField.ShowVisibleTextFields();
		}
	}
	
	public void OnPause()
	{
		PowerManager pm = (PowerManager) JNIApplication.GetApplication().getSystemService(Context.POWER_SERVICE);
		nativeOnPauseView(!pm.isScreenOn());
	}
	
	public void OnResume()
	{
		nativeOnResumeView();
	}
	
}
