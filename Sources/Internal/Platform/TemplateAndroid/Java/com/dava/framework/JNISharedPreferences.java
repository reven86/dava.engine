package com.dava.framework;

import com.dava.engine.DavaActivity;

import android.app.Activity;
import android.content.SharedPreferences;

public class JNISharedPreferences {
	
	SharedPreferences preferences = null;
	SharedPreferences.Editor editor = null;
	static String name = "DavaPreferences";

	public static String GetName()
	{
		return name;
	}
	
	public static Object GetSharedPreferences()
	{
		JNISharedPreferences self = new JNISharedPreferences();
		return self;
	}

	public JNISharedPreferences()
	{
		// TODO: Just use DavaActivity when CoreV1 is removed
		Activity activity = JNIActivity.GetActivity();
		if (activity == null)
		{
			activity = DavaActivity.instance();
		}

		preferences = activity.getSharedPreferences(name, 0);
		editor = preferences.edit();
	}

	public String GetString(String key, String defaultValue)
	{	
		return preferences.getString(key, defaultValue); 
	}
	
	public long GetLong(String key, long defaultValue)
	{
		return preferences.getLong(key,  defaultValue);
	}
	
	public void PutString(String key, String value)
	{	
		editor.putString(key, value);
	}

	public void PutLong(String key, long value)
	{	
		editor.putLong(key, value);
	}
	
	public void Remove(String key)
	{
		editor.remove(key);
	}
	
	public void Clear()
	{
		editor.clear();
		editor.apply();
	}
	
	public void Push()
	{	
		editor.apply();
	}
}
