package com.dava.framework;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;

import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.opengl.GLSurfaceView;
import android.opengl.GLUtils;
import android.util.Log;

public class JNIContextFactory implements GLSurfaceView.EGLContextFactory 
{
    private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
    
    private static int glVersion30 = 3;
    private static int glVersion20 = 2;
    
    @Override
	public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig) 
    {
        Log.i(JNIConst.LOG_TAG, "Activity Render createContext start");
        JNIConfigChooser.printConfig(egl, display, eglConfig);
        
        JNIConst.checkEglError("Before eglCreateContext", egl);
        
        final ActivityManager activityManager = (ActivityManager) JNIActivity.GetActivity().getSystemService(Context.ACTIVITY_SERVICE);
    	final ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();

        EGLContext context = null;
		if(configurationInfo.reqGlEsVersion >= 0x30000)
    	{
            context = createOpenGLESContext(glVersion30, egl, display, eglConfig);
    	}
    		
        if(context == null)
        {
        	Log.w(JNIConst.LOG_TAG, "[JNIContextFactory::createContext] OpenGLES 3.0 is not supported");
        	context = createOpenGLESContext(glVersion20, egl, display, eglConfig);
        }
        
        JNIConst.checkEglError("After eglCreateContext", egl);
        
        Log.i(JNIConst.LOG_TAG, "Activity Render createContext finish");
        return context;
    }

	@Override
	public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) 
    {
	    Log.i(JNIConst.LOG_TAG, "Activity Render destroyContext start");
        egl.eglDestroyContext(display, context);
        Log.i(JNIConst.LOG_TAG, "Activity Render destroyContext finish");
    }
	
	private EGLContext createOpenGLESContext(int openglESVersion, EGL10 egl, EGLDisplay display, EGLConfig eglConfig)
	{
		EGLContext context = null;
		
		try
		{
	        int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, openglESVersion, EGL10.EGL_NONE };
	        context = egl.eglCreateContext(display, eglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);
	        
	        int error = egl.eglGetError(); 
			if (error != EGL10.EGL_SUCCESS) 
			{
				Log.e(JNIConst.LOG_TAG, String.format("[createOpenGLESContext] EGL error: 0x%x (%s)", error, GLUtils.getEGLErrorString(error)));
				context = null;
			}
		}
		catch(Exception e)
		{
			Log.w(JNIConst.LOG_TAG, String.format("[createOpenGLESContext] exception: %s", e.toString()));

			context = null;
		}
		
		return context;
	}
}
