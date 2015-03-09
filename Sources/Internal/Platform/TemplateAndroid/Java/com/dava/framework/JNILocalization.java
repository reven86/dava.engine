package com.dava.framework;

import java.util.Locale;

public class JNILocalization {

	public static String GetLocale()
	{
		String locale = Locale.getDefault().toString();
		return locale;
	}
}
