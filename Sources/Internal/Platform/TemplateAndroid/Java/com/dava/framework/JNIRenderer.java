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

	private int width = 0;
	private int height = 0;
	
	private boolean isRenderRecreated = false;
	private static JNIBackgroundUpdateThread backgroundUpdateThread = null;
	
	public JNIRenderer()
	{
		if (null == backgroundUpdateThread)
			backgroundUpdateThread = new JNIBackgroundUpdateThread();
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
		if (isRenderRecreated || width != w || height != h)
		{
			width = w;
			height = h;

			nativeResize(width, height);
			isRenderRecreated = false;
		}
		OnResume();
		skipFirstFrame = true;

		Log.w(JNIConst.LOG_TAG, "_________onSurfaceChanged__DONE___");
	}

	public boolean isFirstDraw = true;

	@Override
	public void onDrawFrame(GL10 gl) {
		if (skipFirstFrame) {
			skipFirstFrame = false;
			return;
		}
			
		nativeRender();
	}
	
	public void OnPause()
	{
		PowerManager pm = (PowerManager) JNIApplication.GetApplication().getSystemService(Context.POWER_SERVICE);
		nativeOnPauseView(!pm.isScreenOn());
		if (null != backgroundUpdateThread)
			backgroundUpdateThread.StartBackgroundDownloadThread();
	}
	
	public void OnResume()
	{
		nativeOnResumeView();
		if (null != backgroundUpdateThread)
			backgroundUpdateThread.WaitExitDownloadTread();
	}
	
}
