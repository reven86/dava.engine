package com.dava.framework;

import java.util.Locale;
import java.util.TimeZone;
import java.security.NoSuchAlgorithmException;
import java.security.MessageDigest;
import java.math.BigInteger;
import javax.microedition.khronos.opengles.GL10;
import android.os.Build;
import android.provider.Settings.Secure;

public class JNIDeviceInfo {
	final static String TAG = "JNIDeviceInfo";
	static int gpuFamily = -1;

	public static void GetVersion()
	{
		SetJString(Build.VERSION.RELEASE);
	}

	public static void GetManufacturer()
	{
		SetJString(Build.MANUFACTURER);
	}

	public static void GetModel()
	{
		SetJString(Build.MODEL);
	}

	public static void GetLocale()
	{
		SetJString(Locale.getDefault().getDisplayLanguage(Locale.US));
	}

	public static void GetRegion()
	{
		String country = JNIActivity.GetActivity().getResources().getConfiguration().locale.getCountry();
		SetJString(country);
	}

	public static void GetTimeZone()
	{
		SetJString(TimeZone.getDefault().getDisplayName(Locale.US));
	}

	public static void GetUDID()
	{
		String aid = Secure.getString(JNIActivity.GetActivity().getApplicationContext() .getContentResolver(), Secure.ANDROID_ID);

		Object obj = null;
		try {
			((MessageDigest) (obj = MessageDigest.getInstance("MD5"))).update(aid.getBytes(), 0, aid.length());

			obj = String.format("%040X", new Object[] { new BigInteger(1, ((MessageDigest) obj).digest()) });
		} 
		catch (NoSuchAlgorithmException localNoSuchAlgorithmException) {
			obj = aid.substring(0, 32);
		}

		SetJString(obj.toString().toLowerCase());
	}
	
	public static void GetName()
	{
		String serial = android.os.Build.SERIAL;
		if (serial == null || serial.isEmpty())
			serial = "ErrorGetSerialNumber";
		SetJString(serial);
	}
	
	public static int GetZBufferSize()
	{
		return JNIConfigChooser.GetDepthBufferSize();
	}
	
	public static String GetHTTPProxyHost()
	{		
		return System.getProperty("http.proxyHost");
	}
	
	public static int GetHTTPProxyPort()
	{
		String portStr = System.getProperty("http.proxyPort");
	    int proxyPort = Integer.parseInt((portStr != null ? portStr : "-1"));
	    return proxyPort;
	}
	
	public static String GetHTTPNonProxyHosts()
	{
		return System.getProperty("http.nonProxyHosts");
	}
	
	static final int GPU_UNKNOWN = -1;
	static final int GPU_POWERVR_IOS = 0;
	static final int GPU_POWERVR_ANDROID = 1;
	static final int GPU_TEGRA = 2;
	static final int GPU_MALI = 3;
	static final int GPU_ADRENO = 4;
	
	protected static void SetGPUFamily(GL10 gl)
	{
		String extensions = gl.glGetString(GL10.GL_EXTENSIONS);
		
		if (extensions.indexOf("GL_IMG_texture_compression_pvrtc") >= 0)
			gpuFamily = GPU_POWERVR_ANDROID;
		else if (extensions.indexOf("GL_NV_draw_texture") >= 0)
			gpuFamily = GPU_TEGRA;
		else if (extensions.indexOf("GL_AMD_compressed_ATC_texture") >= 0)
			gpuFamily = GPU_ADRENO;
		else if (extensions.indexOf("GL_OES_compressed_ETC1_RGB8_texture") >= 0)
			gpuFamily = GPU_MALI;
		else
			gpuFamily = GPU_UNKNOWN;
	}
	
	public static int GetGPUFamily()
	{
		return gpuFamily;
	}

	public static native void SetJString(String str);
}
