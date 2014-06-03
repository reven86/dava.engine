package com.dava.framework;

import java.util.Locale;
import java.util.TimeZone;
import java.security.NoSuchAlgorithmException;
import java.security.MessageDigest;
import java.math.BigInteger;
import android.os.Build;
import android.provider.Settings.Secure;

public class JNIDeviceInfo {
	final static String TAG = "JNIDeviceInfo";

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

	public static native void SetJString(String str);
}
