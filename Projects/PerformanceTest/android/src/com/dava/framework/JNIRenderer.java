package com.dava.framework;


import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.util.Log;

public class JNIRenderer implements GLSurfaceView.Renderer 
{
    private native void nativeResize(int w, int h);
    private native void nativeRender();
    private native void nativeRenderRecreated();

    private int framebuffer = 0;
    private int colorRenderbuffer = 0;

    private int newWidth = 0;
    private int newHeight = 0;
    
//    private float red = 0.0f;
//    private float green = 0.0f;
//    private float blue = 0.0f;
//    private float alpha = 1.0f;
//    
//    private float redDelta = 0.2f;
//    private float greenDelta = 0.4f;
//    private float blueDelta = 0.6f;
//    
    
    public void onSurfaceCreated(GL10 gl, EGLConfig config) 
    {
    	Log.w(JNIConst.LOG_TAG, "_________onSurfaceCreated_____!!!!_____");
    	nativeRenderRecreated();
    	
    	LogExtensions();
    	
//    	red = CreateNewColor(red, redDelta);
//    	green = CreateNewColor(green, greenDelta);
//    	blue = CreateNewColor(blue, blueDelta);
    	
    	Log.w(JNIConst.LOG_TAG, "_________onSurfaceCreated_____DONE_____");
    }
    
//    private float CreateNewColor(float color, float colorDelta)
//    {
//    	float newColor = (color + colorDelta);
//    	while(1 < newColor) newColor -= 1;
//
//    	return newColor;
//    }
    
    private void LogExtensions()
    {
    	String oglVersion = GLES20.glGetString(GLES20.GL_VERSION);
    	String deviceName = GLES20.glGetString(GLES20.GL_RENDERER);
    	String extensions = GLES20.glGetString(GLES20.GL_EXTENSIONS);
    	
    	Log.i(JNIConst.LOG_TAG, "[GLES_20] oglVersion is " + oglVersion);
    	Log.i(JNIConst.LOG_TAG, "[GLES_20] deviceName is " + deviceName);
    	Log.i(JNIConst.LOG_TAG, "[GLES_20] extensions is " + extensions);
    }
    
    public void onSurfaceChanged(GL10 gl, int w, int h) 
    {
    	Log.w(JNIConst.LOG_TAG, "_________onSurfaceChanged");
    	newWidth = w;
    	newHeight = h;

    	nativeResize(newWidth, newHeight);
    	
//    	GLES20.glViewport(0, 0, w, h);
    	
    	Log.w(JNIConst.LOG_TAG, "_________onSurfaceChanged__DONE___");
    }
    
    
    public boolean isFirstDraw = true;
    public void onDrawFrame(GL10 gl) 
    {
//    	GLES20.glClearColor(red, green, blue, alpha);
//    	GLES20.glClear(GLES20.GL_DEPTH_BUFFER_BIT | GLES20.GL_COLOR_BUFFER_BIT);
    	nativeRender();
    }
}
