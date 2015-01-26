package com.dava.tests;

import com.dava.framework.JNIApplication;

public class TestBedApp extends JNIApplication {
	
	@Override
	public void onCreate() {
		super.onCreate();
	}
	
	static {
		System.loadLibrary("TestBedLib");
		
		/*try {
			Thread.sleep(10000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}*/
	}
}
