package com.dava.performancetests;

import com.dava.framework.JNIApplication;

public class PerformanceTestApp extends JNIApplication {
	
	@Override
	public void onCreate() {
		super.onCreate();
	}
	
	static {
		System.loadLibrary("PerformanceTests");
	}
}
