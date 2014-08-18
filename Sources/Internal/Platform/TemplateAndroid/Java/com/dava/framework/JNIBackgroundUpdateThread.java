package com.dava.framework;

public class JNIBackgroundUpdateThread {
	private static Object needExit = null;
	private static Thread thread = null;
	
	
	protected void StartBackgroundDownloadThread() {
		if (null != thread)
			return;
		
		needExit = null;
		thread = new Thread(new Runnable() {
			
			@Override
			public void run() {
				RegisterBackgroundThread();
				while (needExit == null)
				{
					try {
						Thread.sleep(500);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
					JNIDownloadManager.UpdateDownloadManager();
					//JNINotification.UpdateNotification();
				}
				UnRegisterBackgroundThread();
				
				Thread.yield();
				synchronized (needExit) {
					needExit.notify();
				}
				thread = null;
			}
		});
		thread.start();
	}
	
	protected void WaitExitDownloadTread() {
		if (thread != null) {
			needExit = new Object();
			synchronized (needExit) {
				try {
					needExit.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
	}
	
	native void RegisterBackgroundThread();
	native void UnRegisterBackgroundThread();
}
