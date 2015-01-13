package com.dava.framework;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;

import android.opengl.GLSurfaceView;
import android.util.Log;

public class JNIConfigChooser implements GLSurfaceView.EGLConfigChooser 
{
    private static final int EGL_RENDERABLE_TYPE = 0x3040;
    private static final int EGL_OPENGL_ES2_BIT = 0x0004;

    /** The number of bits requested for the red component */
    protected int redSize = 8;
    /** The number of bits requested for the green component */
    protected int greenSize = 8;
    /** The number of bits requested for the blue component */
    protected int blueSize = 8;
    /** The number of bits requested for the alpha component */
    protected int alphaSize = 8;
    /** The number of bits requested for the stencil component */
    protected int stencilSize = 8;
    /** The number of bits requested for the depth component */
    protected int depthSize = 16;

    public JNIConfigChooser(int r, int g, int b, int a, int depth, int stencil) 
    {
    	redSize = r;
    	greenSize = g;
    	blueSize = b;
    	alphaSize = a;
    	depthSize = depth;
        stencilSize = stencil;
    }
    
    public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) 
    {
//    	Log.w(JNIConst.LOG_TAG, "[JNIConfigChooser.chooseConfig] start");
    	
        int[] oldConf = new int[] {EGL10.EGL_NONE};
        int[] configAttrs = new int[3 + oldConf.length-1];

        int i = 0;
        for (i = 0; i < oldConf.length-1; i++)
        {
            configAttrs[i] = oldConf[i];
        }
        configAttrs[i++] = EGL_RENDERABLE_TYPE;
        configAttrs[i++] = EGL_OPENGL_ES2_BIT;
        configAttrs[i++] = EGL10.EGL_NONE;

        int[] oldConfES2 = configAttrs;
        
        configAttrs = new int[13 + oldConfES2.length-1];
        for (i = 0; i < oldConfES2.length-1; i++)
        {
            configAttrs[i] = oldConfES2[i];
        }
        configAttrs[i++] = EGL10.EGL_RED_SIZE;
        configAttrs[i++] = redSize;
        configAttrs[i++] = EGL10.EGL_GREEN_SIZE;
        configAttrs[i++] = greenSize;
        configAttrs[i++] = EGL10.EGL_BLUE_SIZE;
        configAttrs[i++] = blueSize;
        configAttrs[i++] = EGL10.EGL_ALPHA_SIZE;
        configAttrs[i++] = alphaSize;
        configAttrs[i++] = EGL10.EGL_STENCIL_SIZE;
        configAttrs[i++] = stencilSize;
        configAttrs[i++] = EGL10.EGL_DEPTH_SIZE;
        configAttrs[i++] = depthSize;
        configAttrs[i++] = EGL10.EGL_NONE;

        egl.eglGetError();
        int[] version = new int[2];
        boolean ret = egl.eglInitialize(display, version);
//        Log.w(JNIConst.LOG_TAG, "EglInitialize returned: " + ret);
        if (!ret)
        {
            Log.w(JNIConst.LOG_TAG, "error1");
            return null;
        }
        int eglErr = egl.eglGetError();
        if (eglErr != EGL10.EGL_SUCCESS)
        {
            Log.w(JNIConst.LOG_TAG, "error2: " + eglErr);
            return null;
        }

        final EGLConfig[] config = new EGLConfig[20];
        int num_configs[] = new int[1];
        egl.eglChooseConfig(display, configAttrs, config, config.length, num_configs);
        eglErr = egl.eglGetError();
        if (eglErr != EGL10.EGL_SUCCESS)
        {
            Log.w(JNIConst.LOG_TAG, "eglChooseConfig err: " + egl.eglGetError());
        }

        EGLConfig eglConfig = null;
        int score = 1<<24; // to make sure even worst score is better than this, like 8888 when request 565...
        int val[] = new int[1];
        for (i = 0; i < num_configs[0]; i++)
        {
            boolean cont = true;
            int currScore = 0;
            int r, g, b, a, d, s;
            for (int j = 0; j < (oldConf.length-1)>>1; j++)
            {
                egl.eglGetConfigAttrib(display, config[i], configAttrs[j*2], val);
                if ((val[0] & configAttrs[j*2+1]) != configAttrs[j*2+1])
                {
                    cont = false; // Doesn't match the "must have" configs
                    break;
                }
            }
            if (!cont)
            {
                continue;
            }
            egl.eglGetConfigAttrib(display, config[i], EGL10.EGL_RED_SIZE, val); r = val[0];
            egl.eglGetConfigAttrib(display, config[i], EGL10.EGL_GREEN_SIZE, val); g = val[0];
            egl.eglGetConfigAttrib(display, config[i], EGL10.EGL_BLUE_SIZE, val); b = val[0];
            egl.eglGetConfigAttrib(display, config[i], EGL10.EGL_ALPHA_SIZE, val); a = val[0];
            egl.eglGetConfigAttrib(display, config[i], EGL10.EGL_DEPTH_SIZE, val); d = val[0];
            egl.eglGetConfigAttrib(display, config[i], EGL10.EGL_STENCIL_SIZE, val); s = val[0];

//            Log.w(JNIConst.LOG_TAG, ">>> EGL Config ["+i+"] R"+r+"G"+g+"B"+b+"A"+a+" D"+d+"S"+s);

            currScore = (Math.abs(r - redSize) + Math.abs(g - greenSize) + Math.abs(b - blueSize) + Math.abs(a - alphaSize)) << 16;
            currScore += Math.abs(d - depthSize) << 8;
            currScore += Math.abs(s - stencilSize);
            
            if (currScore < score)
            {
                Log.w(JNIConst.LOG_TAG, "--------------------------");
                Log.w(JNIConst.LOG_TAG, "New config chosen: " + i);
                for (int j = 0; j < (configAttrs.length-1)>>1; j++)
                {
                    egl.eglGetConfigAttrib(display, config[i], configAttrs[j*2], val);
                    if (val[0] >= configAttrs[j*2+1])
                    {
//                        Log.w(JNIConst.LOG_TAG, "setting " + j + ", matches: " + val[0]);
                    }
                }

                score = currScore;
                eglConfig = config[i];
            }
        }
        
        
        printConfig(egl, display, eglConfig);
//    	Log.w(JNIConst.LOG_TAG, "[JNIConfigChooser.chooseConfig] stop");
        return eglConfig;
    }
    

    public static void printConfig(EGL10 egl, EGLDisplay display, EGLConfig config) 
    {
        int[] attributes = 
        {
                EGL10.EGL_BUFFER_SIZE,
                EGL10.EGL_ALPHA_SIZE,
                EGL10.EGL_BLUE_SIZE,
                EGL10.EGL_GREEN_SIZE,
                EGL10.EGL_RED_SIZE,
                EGL10.EGL_DEPTH_SIZE,
                EGL10.EGL_STENCIL_SIZE,
                EGL10.EGL_CONFIG_CAVEAT,
                EGL10.EGL_CONFIG_ID,
                EGL10.EGL_LEVEL,
                EGL10.EGL_MAX_PBUFFER_HEIGHT,
                EGL10.EGL_MAX_PBUFFER_PIXELS,
                EGL10.EGL_MAX_PBUFFER_WIDTH,
                EGL10.EGL_NATIVE_RENDERABLE,
                EGL10.EGL_NATIVE_VISUAL_ID,
                EGL10.EGL_NATIVE_VISUAL_TYPE,
                0x3030, // EGL10.EGL_PRESERVED_RESOURCES,
                EGL10.EGL_SAMPLES,
                EGL10.EGL_SAMPLE_BUFFERS,
                EGL10.EGL_SURFACE_TYPE,
                EGL10.EGL_TRANSPARENT_TYPE,
                EGL10.EGL_TRANSPARENT_RED_VALUE,
                EGL10.EGL_TRANSPARENT_GREEN_VALUE,
                EGL10.EGL_TRANSPARENT_BLUE_VALUE,
                0x3039, // EGL10.EGL_BIND_TO_TEXTURE_RGB,
                0x303A, // EGL10.EGL_BIND_TO_TEXTURE_RGBA,
                0x303B, // EGL10.EGL_MIN_SWAP_INTERVAL,
                0x303C, // EGL10.EGL_MAX_SWAP_INTERVAL,
                EGL10.EGL_LUMINANCE_SIZE,
                EGL10.EGL_ALPHA_MASK_SIZE,
                EGL10.EGL_COLOR_BUFFER_TYPE,
                EGL10.EGL_RENDERABLE_TYPE,
                0x3042 // EGL10.EGL_CONFORMANT
        };
        String[] names = 
        {
                "EGL_BUFFER_SIZE",
                "EGL_ALPHA_SIZE",
                "EGL_BLUE_SIZE",
                "EGL_GREEN_SIZE",
                "EGL_RED_SIZE",
                "EGL_DEPTH_SIZE",
                "EGL_STENCIL_SIZE",
                "EGL_CONFIG_CAVEAT",
                "EGL_CONFIG_ID",
                "EGL_LEVEL",
                "EGL_MAX_PBUFFER_HEIGHT",
                "EGL_MAX_PBUFFER_PIXELS",
                "EGL_MAX_PBUFFER_WIDTH",
                "EGL_NATIVE_RENDERABLE",
                "EGL_NATIVE_VISUAL_ID",
                "EGL_NATIVE_VISUAL_TYPE",
                "EGL_PRESERVED_RESOURCES",
                "EGL_SAMPLES",
                "EGL_SAMPLE_BUFFERS",
                "EGL_SURFACE_TYPE",
                "EGL_TRANSPARENT_TYPE",
                "EGL_TRANSPARENT_RED_VALUE",
                "EGL_TRANSPARENT_GREEN_VALUE",
                "EGL_TRANSPARENT_BLUE_VALUE",
                "EGL_BIND_TO_TEXTURE_RGB",
                "EGL_BIND_TO_TEXTURE_RGBA",
                "EGL_MIN_SWAP_INTERVAL",
                "EGL_MAX_SWAP_INTERVAL",
                "EGL_LUMINANCE_SIZE",
                "EGL_ALPHA_MASK_SIZE",
                "EGL_COLOR_BUFFER_TYPE",
                "EGL_RENDERABLE_TYPE",
                "EGL_CONFORMANT"
        };
        
        int[] value = new int[1];
        for (int i = 0; i < attributes.length; i++) 
        {
            int attribute = attributes[i];
            String name = names[i];
            if ( egl.eglGetConfigAttrib(display, config, attribute, value)) 
            {
//                Log.w(JNIConst.LOG_TAG, String.format("  %s: %d\n", name, value[0]));
            } 
            else 
            {
                // Log.w(TAG, String.format("  %s: failed\n", name));
                while (egl.eglGetError() != EGL10.EGL_SUCCESS);
            }
        }
    }
}
