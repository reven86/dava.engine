package com.dava.framework;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.math.BigInteger;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.StringTokenizer;
import java.util.TimeZone;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Environment;
import android.os.StatFs;
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
	
	private static final int NETWORK_TYPE_NOT_CONNECTED = 0;
	private static final int NETWORK_TYPE_UNKNOWN = 1;
	private static final int NETWORK_TYPE_MOBILE = 2;
	private static final int NETWORK_TYPE_WIFI = 3;
	private static final int NETWORK_TYPE_WIMAX = 4;
	private static final int NETWORK_TYPE_ETHERNET = 5;
	private static final int NETWORK_TYPE_BLUETOOTH = 6;
	
	public static int GetNetworkType() {
		ConnectivityManager cm = (ConnectivityManager)JNIActivity.GetActivity().getSystemService(Context.CONNECTIVITY_SERVICE);
		NetworkInfo info = cm.getActiveNetworkInfo();
		if (info == null || !info.isConnected())
			return NETWORK_TYPE_NOT_CONNECTED;
		
		int netType = info.getType();
		switch (netType) {
		case ConnectivityManager.TYPE_MOBILE:
			return NETWORK_TYPE_MOBILE;
		case ConnectivityManager.TYPE_WIFI:
			return NETWORK_TYPE_WIFI;
		case ConnectivityManager.TYPE_WIMAX:
			return NETWORK_TYPE_WIMAX;
		case ConnectivityManager.TYPE_ETHERNET:
			return NETWORK_TYPE_ETHERNET;
		case ConnectivityManager.TYPE_BLUETOOTH:
			return NETWORK_TYPE_BLUETOOTH;
		}
		return NETWORK_TYPE_UNKNOWN;
	}
	
	private static final int MaxSignalLevel = 100; 
	
	public static int GetSignalStrength(int networkType) {
		switch (networkType) {
		case NETWORK_TYPE_WIFI: {
			WifiManager wifiManager = (WifiManager) JNIActivity.GetActivity().getSystemService(Context.WIFI_SERVICE);
			WifiInfo wifiInfo = wifiManager.getConnectionInfo();
			return WifiManager.calculateSignalLevel(wifiInfo.getRssi(), MaxSignalLevel);
		}
		
		case NETWORK_TYPE_MOBILE: {
			if (JNIActivity.singalStrengthListner != null) {
				//Get the GSM Signal Strength, valid values are (0-31, 99) as defined in TS 27.007 8.5
				int sign = JNIActivity.singalStrengthListner.GetSignalStrength();
				if (sign == 99)
					return -1;
				return (int)(MaxSignalLevel * sign / 31.f); 
			}
			else
				return 0;
		}
		
		case NETWORK_TYPE_ETHERNET:
			return MaxSignalLevel;
		}
		return 0;
	}

	public static class StorageInfo
	{
		public final String path;

		public final boolean readOnly;
		public final boolean emulated;

		public final long capacity;
		public final long freeSpace;

		StorageInfo(String path, boolean readOnly, boolean emulated, long capacity, long freeSpace)
		{
			this.path = path;
			this.readOnly = readOnly;
			this.emulated = emulated;
			this.capacity = capacity;
			this.freeSpace = freeSpace;
		}
	}

	public static StorageInfo GetInternalStorageInfo()
	{
		String path = Environment.getDataDirectory().getPath();
		StatFs statFs = new StatFs(path);

		long capacity = (long)statFs.getBlockCount() * (long)statFs.getBlockSize();
		long free = (long)statFs.getAvailableBlocks() * (long)statFs.getBlockSize();

		return new StorageInfo(path, false, false, capacity, free);
	}

	public static boolean IsPrimaryExternalStoragePresent()
	{
		String state = Environment.getExternalStorageState();
		if (state.equals(Environment.MEDIA_MOUNTED) || state.equals(Environment.MEDIA_MOUNTED_READ_ONLY))
		{
			return true;
		}

		return false;
	}

	public static StorageInfo GetPrimaryExternalStorageInfo()
	{
		if (IsPrimaryExternalStoragePresent())
		{
			String path = Environment.getExternalStorageDirectory().getPath();
			StatFs statFs = new StatFs(path);

            long capacity = (long)statFs.getBlockCount() * (long)statFs.getBlockSize();
            long free = (long)statFs.getAvailableBlocks() * (long)statFs.getBlockSize();

            boolean isEmulated = Environment.isExternalStorageEmulated();
            boolean isReadOnly = Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED_READ_ONLY);

            return new StorageInfo(path, isReadOnly, isEmulated, capacity, free);
        }

		return new StorageInfo("", false, false, -1, -1);
	}

	public static StorageInfo[] GetSecondaryExternalStoragesList()
	{
		List<StorageInfo> infos = new ArrayList<StorageInfo>();

		HashSet<String> paths = new HashSet<String>();
		paths.add(Environment.getExternalStorageDirectory().getPath());

		BufferedReader reader = null;
		try
		{
			reader = new BufferedReader(new FileReader("/proc/mounts"));

			String line;
			while ((line = reader.readLine()) != null)
			{
				if (line.contains("vfat") || line.contains("/mnt"))
				{
					StringTokenizer tokens = new StringTokenizer(line, " ");
					String unused = tokens.nextToken(); //device
					String mountPoint = tokens.nextToken(); //mount point

					if (paths.contains(mountPoint))
					{
					    continue;
					}

					unused = tokens.nextToken(); //file system

					List<String> flags = Arrays.asList(tokens.nextToken().split(",")); //flags
					boolean readonly = flags.contains("ro");

					if (!line.contains("/mnt/secure")
						&& !line.contains("/mnt/asec")
						&& !line.contains("/mnt/obb")
						&& !line.contains("/dev/mapper")
						&& !line.contains("tmpfs"))
					{
						paths.add(mountPoint);

						StatFs statFs = null;
						try
						{
							statFs = new StatFs(mountPoint);
						}
						catch (Exception e)
						{
							continue;
						}

			            long capacity = (long)statFs.getBlockCount() * (long)statFs.getBlockSize();
			            long free = (long)statFs.getAvailableBlocks() * (long)statFs.getBlockSize();

						infos.add(new StorageInfo(mountPoint, readonly, false, capacity, free));
					}
				}
			}
		}
		catch (FileNotFoundException e)
		{
		}
		catch (IOException e)
		{
		}
		finally
		{
			if (reader != null)
			{
				try
				{
					reader.close();
				}
				catch (IOException e)
				{
				}
			}
		}

		StorageInfo[] arr = new StorageInfo[infos.size()];
		infos.toArray(arr);
		return arr;
	}
	
	public static native void SetJString(String str);
}
