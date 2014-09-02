package com.dava.framework;

import javax.microedition.khronos.egl.EGL10;

import android.opengl.GLES20;
import android.util.Log;

public class JNIConst
{
	public static String LOG_TAG = "DAVA";
	
	public static void checkEglError(String prompt, EGL10 egl) 
	{
		int error = EGL10.EGL_SUCCESS;
		while ((error = egl.eglGetError()) != EGL10.EGL_SUCCESS) 
		{
			Log.e(LOG_TAG, String.format("%s: EGL error: 0x%x", prompt, error));
		}
	}
	
	public static void checkGlError(String op) 
    {
        int error;
        while ((error = GLES20.glGetError()) != GLES20.GL_NO_ERROR) 
        {
            Log.e(LOG_TAG, op + ": glError " + error);
            throw new RuntimeException(op + ": glError " + error);
        }
    }

}
