package com.dava.sceneviewer;

import com.dava.framework.JNIApplication;

public class SceneViewerApp extends JNIApplication {
	
	@Override
	public void onCreate() {
		super.onCreate();
	}
	
	static {
		System.loadLibrary("SceneViewer");
		
		/*try {
			Thread.sleep(10000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}*/
	}
}
