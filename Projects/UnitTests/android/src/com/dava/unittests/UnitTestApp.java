package com.dava.unittests;

import com.dava.framework.JNIApplication;

public class UnitTestApp extends JNIApplication {
	
	@Override
	public void onCreate() {
		super.onCreate();
	}
	
	static {
		System.loadLibrary("UnitTests");
	}
}
