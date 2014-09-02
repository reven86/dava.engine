package com.dava.framework;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;

public class JNIAssert {
	public static void Assert(final String message)
	{
		Activity activity = JNIActivity.GetActivity();
		final AlertDialog.Builder adBuilder = new AlertDialog.Builder(activity);
		adBuilder.setMessage(message);
		final Object mutex = new Object();
		activity.runOnUiThread(new Runnable() {
			
			@Override
			public void run() {
				adBuilder.setNeutralButton("Ok", new OnClickListener() {
					
					public void onClick(DialogInterface dialog, int which) {
						synchronized (mutex) {
							mutex.notify();
						}
					}
				});
				adBuilder.show();
			}
		});
		synchronized (mutex) {
			try {
				mutex.wait();
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}
}
