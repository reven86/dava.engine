package com.dava.framework;

import android.app.NotificationManager;
import android.content.Context;
import android.support.v4.app.NotificationCompat;
import android.support.v4.app.NotificationCompat.Builder;

public class JNINotification {
	private static Builder builder = null;
	private static NotificationManager notificationManager = null;
	
	static void Init() {
		Context context = JNIApplication.GetApplication();
		notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
		builder = new NotificationCompat.Builder(JNIActivity.GetActivity());
		JNIActivity.GetActivity().InitNotification(builder);
	}
	
	public static void ShowNotifitaionWithProgress(
			int id,
			String title, 
			String text, 
			int maxValue, 
			int value) {
		if (builder != null && notificationManager != null) {
			builder.setContentTitle(title)
				.setContentText(text)
				.setProgress(maxValue, value, false);
			notificationManager.notify(id, builder.build());
		}
	}
	
	public static void HideNotification(int id) {
		if (notificationManager != null)
			notificationManager.cancel(id);
	}
	
	protected static native void UpdateNotification();
}
