package com.dava.template;

import com.dava.framework.JNIApplication;

public class TemplateApp extends JNIApplication {
	
	@Override
	public void onCreate() {
		super.onCreate();
	}
	
	static {
		System.loadLibrary("gnustl_shared");
		System.loadLibrary("TemplateLib");
		
		/*try {
			Thread.sleep(10000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}*/
	}
}
