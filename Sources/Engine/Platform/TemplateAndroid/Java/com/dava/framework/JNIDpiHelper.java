package com.dava.framework;

import android.util.DisplayMetrics;

public class JNIDpiHelper {
	public static int GetScreenDPI()
	{
		DisplayMetrics dm = new DisplayMetrics();
		JNIActivity.GetActivity().getWindowManager().getDefaultDisplay().getMetrics(dm);
		return (int)dm.ydpi;
	}
}
