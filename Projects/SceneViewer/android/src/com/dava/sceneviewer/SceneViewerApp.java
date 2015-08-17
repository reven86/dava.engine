package com.dava.sceneviewer;

import com.dava.framework.JNIApplication;

public class SceneViewerApp extends JNIApplication {
	
	@Override
	public void onCreate() {
		super.onCreate();
	}
	
	static {
		System.loadLibrary("gnustl_shared");
		System.loadLibrary("SceneViewer");
	}
}
