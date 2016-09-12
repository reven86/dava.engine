package com.dava.engine;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import com.dava.framework.JNIDeviceInfo;

import android.content.Context;
import android.opengl.GLSurfaceView;

public class DavaSplashView extends GLSurfaceView {
    /*// from RenderBase.h
    enum eGPUFamily : uint8
    {
        GPU_POWERVR_IOS = 0,
        GPU_POWERVR_ANDROID,
        GPU_TEGRA,
        GPU_MALI,
        GPU_ADRENO,
        GPU_DX11,
        GPU_ORIGIN, // not a device - for development only
        GPU_FAMILY_COUNT,

        GPU_DEVICE_COUNT = GPU_ORIGIN,
        GPU_INVALID = 127
    };
    */
    public static byte GPU_POWERVR_ANDROID = 1;
    public static byte GPU_TEGRA = 2;
    public static byte GPU_MALI = 3;
    public static byte GPU_ADRENO = 4;
    public static byte GPU_INVALID = 127;
    
    private static class Renderer implements GLSurfaceView.Renderer
    {
        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            gl.glClearColor(0.f, 0.f, 0.f, 0.f);
            
            getZBufferSize(gl);
            getGPUFamily(gl);
            
            // Now we collect all device info and while show splash to user
            // continue engine initialization
            DavaActivity.instance().onFinishCollectDeviceInfo();
        }
        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            gl.glViewport(0, 0, width, height);
        }
        @Override
        public void onDrawFrame(GL10 gl) {
            gl.glClear(GL10.GL_COLOR_BUFFER_BIT);
        }
        
        private void getZBufferSize(GL10 gl) {
            int[] params = new int[1];
            gl.glGetIntegerv(GL10.GL_DEPTH_BITS, params, 0);
            JNIDeviceInfo.zBufferSize = params[0];
        }
        private void getGPUFamily(GL10 gl) {
            String extensions = gl.glGetString(GL10.GL_EXTENSIONS);
            byte gpuArchitecture = getSupportedTextures(extensions);
            JNIDeviceInfo.gpuFamily = gpuArchitecture;
        }
        
        private byte getSupportedTextures(String ext)
        {
            // next code from: gles_check_GL_extensions(rhi_GLES2.cpp)
            boolean ATC_Supported = ext.contains("GL_AMD_compressed_ATC_texture");
            boolean PVRTC_Supported = ext.contains("GL_IMG_texture_compression_pvrtc");
            boolean PVRTC2_Supported = ext.contains("GL_IMG_texture_compression_pvrtc2");
            boolean ETC1_Supported = ext.contains("GL_OES_compressed_ETC1_RGB8_texture");
            boolean ETC2_Supported = ext.contains("GL_OES_compressed_ETC2_RGB8_texture");
            // boolean EAC_Supported = ETC2_Supported;
            boolean DXT_Supported = ext.contains("GL_EXT_texture_compression_s3tc") || ext.contains("GL_NV_texture_compression_s3tc");

            byte gpuFamily = GPU_INVALID;
            // next code from: GetGPUFamily(DeviceInfoAndroid.cpp) and from: gles2_TextureFormatSupported(rhi_GLES2.cpp)
            if (PVRTC_Supported || PVRTC2_Supported) // TEXTURE_FORMAT_PVRTC_4BPP_RGBA
            {
                gpuFamily = GPU_POWERVR_ANDROID;
            }
            else if (DXT_Supported) // TEXTURE_FORMAT_DXT1
            {
                gpuFamily = GPU_TEGRA;
            }
            else if (ATC_Supported) // TEXTURE_FORMAT_ATC_RGB
            {
                gpuFamily = GPU_ADRENO;
            }
            else if (ETC1_Supported || ETC2_Supported) // TEXTURE_FORMAT_ETC1
            {
                gpuFamily = GPU_MALI;
            }
            return gpuFamily;
        }
    }
    
    public DavaSplashView(Context context) {
        super(context);
        
        setEGLContextClientVersion(1);
        
        setRenderer(new Renderer());
    }

}
