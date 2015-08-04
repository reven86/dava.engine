package com.dava.testbed;

import com.dava.framework.JNIApplication;

public class TestBedApp extends JNIApplication {
	
	@Override
	public void onCreate() {
		super.onCreate();
	}
	
	static {
		System.loadLibrary("TestBed");
	}
}
