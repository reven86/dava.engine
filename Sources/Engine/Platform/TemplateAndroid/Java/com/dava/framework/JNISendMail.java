package com.dava.framework;

import android.content.Intent;
import android.util.Log;


public class JNISendMail {
	final static String TAG = "JNISendMail";
	
	static boolean res;
	public static boolean SendEMail(final String address, final String subject, final String messageText)
	{
		Log.d(TAG, address + subject + messageText);
		
		res = false;
		final Object mutex = new Object();
		
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
			
			@Override
			public void run() {
				Intent i = new Intent(Intent.ACTION_SEND);
				i.setType("message/rfc822");
				i.putExtra(Intent.EXTRA_EMAIL  , new String[]{address});
				i.putExtra(Intent.EXTRA_SUBJECT, subject);
				i.putExtra(Intent.EXTRA_TEXT   , messageText);
				try {
					JNIActivity.GetActivity().startActivity(Intent.createChooser(i, "Send mail..."));
					res = true;
				} catch (android.content.ActivityNotFoundException ex) {
					//Toast.makeText(MyActivity.this, "There are no email clients installed.", Toast.LENGTH_SHORT).show();
					Log.d(TAG, "There are no email clients installed.");
				}
				
				synchronized (mutex) {
					mutex.notify();
				}
			}
		});
		
		synchronized (mutex) {
			try {
				mutex.wait();
			} catch (InterruptedException e) {
				//e.printStackTrace();
			}
		}
		
		return res;
	}
}
