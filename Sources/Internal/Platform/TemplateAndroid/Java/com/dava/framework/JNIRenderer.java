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
	
	private boolean skipFirstFrame = false;
	private boolean isFirstFrameAfterDraw = false;

	private int width = 0;
	private int height = 0;
	
	private boolean isRenderRecreated = false;
	
	public JNIRenderer()
	{
	}

	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		Log.w(JNIConst.LOG_TAG, "_________onSurfaceCreated_____!!!!_____");
		
		JNIDeviceInfo.SetGPUFamily(gl);

		isRenderRecreated = true;
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
		Log.w(JNIConst.LOG_TAG, "_________onSurfaceChanged");
		if (isRenderRecreated || width != w || height != h) {
			if (width != w || height != h) {
				skipFirstFrame = true;
				width = w;
				height = h;
			}
			nativeResize(width, height);
			isRenderRecreated = false;
		}
		OnResume();
		isFirstFrameAfterDraw = true;

		Log.w(JNIConst.LOG_TAG, "_________onSurfaceChanged__DONE___");
	}

	@Override
	public void onDrawFrame(GL10 gl) {
		if (skipFirstFrame) {
			skipFirstFrame = false; //skip first frame for correct unlock device in landscape mode, after unlock device in first frame draw in portrait mode
			return;
		}

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
