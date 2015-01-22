package com.dava.framework;

import android.content.SharedPreferences;

public class JNISharedPreferences {
	
	SharedPreferences preferences = null;
	SharedPreferences.Editor editor = null;

	static Object GetSharedPreferences(String name, int mode)
	{
		JNISharedPreferences self = new JNISharedPreferences(name, mode);
		return self;
	}

	JNISharedPreferences(String name, int mode)
	{
		preferences = JNIActivity.GetActivity().getSharedPreferences(name, mode);
		editor = preferences.edit();
	}

	String GetString(String key, String defaultValue)
	{	
		return preferences.getString(key, defaultValue); 
	}
	
	void PutString(String key, String value)
	{	
		editor.putString(key, value);
	}
	
	void Remove(String key)
	{
		editor.remove(key);
	}
	
	void Clear()
	{
		editor.clear();
	}
	
	void Push()
	{	
		editor.apply();
	}
}
