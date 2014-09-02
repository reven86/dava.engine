package com.dava.framework;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;

import android.opengl.GLSurfaceView;
//import android.util.Log;

public class JNIContextFactory implements GLSurfaceView.EGLContextFactory 
{
    private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
    
	public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig) 
    {
//        Log.w(JNIConst.LOG_TAG, "[JNIContextFactory] ___ eglConfig _____");
        JNIConfigChooser.printConfig(egl, display, eglConfig);
        
        JNIConst.checkEglError("Before eglCreateContext", egl);
        int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };
        EGLContext context = egl.eglCreateContext(display, eglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);
        
        JNIConst.checkEglError("After eglCreateContext", egl);

//        Log.w(JNIConst.LOG_TAG, "[JNIContextFactory] ___ eglConfig DONE _____");
        return context;
    }

	public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) 
    {
        egl.eglDestroyContext(display, context);
    }
}
