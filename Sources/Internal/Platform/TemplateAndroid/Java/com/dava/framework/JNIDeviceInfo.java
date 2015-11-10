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
import android.util.Log;

public class JNIDeviceInfo {
	final static String TAG = "JNIDeviceInfo";

	public static String GetVersion()
	{
		return Build.VERSION.RELEASE;
	}

	public static String GetManufacturer()
	{
		return Build.MANUFACTURER;
	}

	public static String GetModel()
	{
		return Build.MODEL;
	}

	public static String GetLocale()
	{
		return Locale.getDefault().getDisplayLanguage(Locale.US);
	}

	public static String GetRegion()
	{
		return JNIActivity.GetActivity().getResources().getConfiguration().locale.getCountry();
	}

	public static String GetTimeZone()
	{
		return TimeZone.getDefault().getID();
	}

	public static String GetUDID()
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

		return obj.toString().toLowerCase();
	}
	
	public static String GetName()
	{
		String serial = android.os.Build.SERIAL;
		if (serial == null || serial.isEmpty())
			serial = "ErrorGetSerialNumber";
		return serial;
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
		public final boolean removable;
		public final boolean emulated;

		public final long capacity;
		public final long freeSpace;

		StorageInfo(String path, boolean readOnly, boolean removable, boolean emulated, long capacity, long freeSpace)
		{
			this.path = path;
			this.readOnly = readOnly;
			this.removable = removable;
			this.emulated = emulated;
			this.capacity = capacity;
			this.freeSpace = freeSpace;
		}
	}
	
	private static class StorageCapacity
	{
	    public long capacity = 0;
	    public long free = 0;
	}

	private static StorageCapacity getCapacityAndFreeSpace(String path)
	{
        StatFs statFs = new StatFs(path);

		StorageCapacity sc = new StorageCapacity();
		
		fillCapacityAndFreeSpace(statFs, sc);
        return sc;
    }

	
    @SuppressWarnings("deprecation")
    private static void fillCapacityAndFreeSpace(StatFs statFs,
            StorageCapacity st) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) // 4.2
		{
		    st.capacity = statFs.getBlockCountLong() * statFs.getBlockSizeLong();
		    st.free = statFs.getAvailableBlocksLong() * statFs.getBlockSizeLong();
		} else
		{
		    // do not remove (long) conversion may return negative values
		    st.capacity = (long)statFs.getBlockCount() * (long)statFs.getBlockSize();
		    st.free = (long)statFs.getAvailableBlocks() * (long)statFs.getBlockSize();
		}
    }
	
    public static StorageInfo GetInternalStorageInfo()
	{
		String path = Environment.getDataDirectory().getPath();
		path += "/";
		StorageCapacity st = getCapacityAndFreeSpace(path);
		return new StorageInfo(path, false, false, false, st.capacity, st.free);
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
			path += "/";
			
			StorageCapacity st = getCapacityAndFreeSpace(path);

			boolean isRemovable = Environment.isExternalStorageRemovable();
            boolean isEmulated = Environment.isExternalStorageEmulated();
            boolean isReadOnly = Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED_READ_ONLY);

            return new StorageInfo(path, isReadOnly, isRemovable, isEmulated, st.capacity, st.free);
        }

		return new StorageInfo("", false, false, false, -1, -1);
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
				if (!line.contains("/mnt/secure")
						&& !line.contains("/mnt/asec")
						&& !line.contains("/mnt/obb")
						&& !line.contains("/dev/mapper")
						&& !line.contains("emulated")
						&& !line.contains("tmpfs"))
				{
					StringTokenizer tokens = new StringTokenizer(line, " ");
					tokens.nextToken(); //device
					String mountPoint = tokens.nextToken();

					if (paths.contains(mountPoint))
					{
					    continue;
					}

					String fileSystem = tokens.nextToken();

					List<String> flags = Arrays.asList(tokens.nextToken().split(",")); //flags
					boolean readonly = flags.contains("ro");

					if (fileSystem.equals("vfat") || mountPoint.startsWith("/mnt") || mountPoint.startsWith("/storage"))
					{
						mountPoint += "/";
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

						StorageCapacity sc = new StorageCapacity();
						
						fillCapacityAndFreeSpace(statFs, sc);

						infos.add(new StorageInfo(mountPoint, readonly, true, false, sc.capacity, sc.free));
					}
				}
			}
		}
		catch (FileNotFoundException e)
		{
		    Log.e(TAG, e.getMessage());
		}
		catch (IOException e)
		{
		    Log.e(TAG, e.getMessage());
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
				    Log.e(TAG, e.getMessage());
				}
			}
		}

		StorageInfo[] arr = new StorageInfo[infos.size()];
		infos.toArray(arr);
		return arr;
	}
}
