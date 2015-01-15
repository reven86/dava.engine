package com.dava.levelperformancetest;

import com.dava.framework.JNIApplication;

public class LevelPerformanceTestApp extends JNIApplication {
	@Override
	public void onCreate() {
		super.onCreate();
	}
	
	static {
		System.loadLibrary("LevelPerformanceTest");
		
		/*try {
			Thread.sleep(5000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}*/
	}
}
