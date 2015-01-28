package com.dava.framework;

import android.content.SharedPreferences;

public class JNISharedPreferences {
	
	SharedPreferences preferences = null;
	SharedPreferences.Editor editor = null;
	static String name = "DavaPreferences";

	static String GetName()
	{
		return name;
	}
	
	static Object GetSharedPreferences()
	{
		JNISharedPreferences self = new JNISharedPreferences();
		return self;
	}

	JNISharedPreferences()
	{
		preferences = JNIActivity.GetActivity().getSharedPreferences(name, 0);
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
