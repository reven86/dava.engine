package com.dava.framework;

import android.app.AlarmManager;
import android.content.res.AssetManager;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.media.RingtoneManager;
import android.net.Uri;
import android.support.v4.app.NotificationCompat;

import java.util.Calendar;

public class JNINotificationProvider {
	private static NotificationCompat.Builder builder = null;
	private static NotificationManager notificationManager = null;
	private static AssetManager assetsManager = null;
	private static boolean isInited = false;
    private static int icon;

	private native static void onNotificationPressed(String uid);

    public static void SetNotificationIcon(int value) { icon = value; }

    static void Init() {
		Context context = JNIApplication.GetApplication();
		assetsManager = context.getAssets();
		notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
		builder = new NotificationCompat.Builder(context);

		isInited = null != builder && null != notificationManager && null != assetsManager;
	}
    
    static void CleanBuilder()
    {
    	if (null != builder)
    	{
    		builder.setContentTitle("")
    			.setContentText("")
    			.setProgress(0, 0, false);
    	}
    }

    static void EnableTapAction(String uid) {
		if (isInited) {
			CleanBuilder();
			JNIActivity activity = JNIActivity.GetActivity();
			
			Intent intent = new Intent(activity, activity.getClass());
			intent.putExtra("uid", uid);
			PendingIntent pIntent = PendingIntent.getActivity(activity, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT);
			builder.setContentIntent(pIntent);

			notificationManager.notify(uid, 0, builder.build());
		}
	}
	
    static void NotificationPressed(String uid)
    {
    	if (isInited) {
    		onNotificationPressed(uid);
    	}
    }
    
	static void NotifyProgress(String uid, String title, String text, int maxValue, int value, boolean useSound) {
		if (isInited) {
			CleanBuilder();
			
			Uri uri = null;
	        if (useSound)
	        {
	            uri = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);    
	        }
	        
			builder.setContentTitle(title)
				.setContentText(text)
				.setProgress(maxValue, value, false)
				.setSound(uri);
			
			notificationManager.notify(uid, 0, builder.build());
		}
	}
	
    static void NotifyText(String uid, String title, String text, boolean useSound) {
		if (isInited) {
			CleanBuilder();
			
			Uri uri = null;
	        if (useSound)
	        {
	            uri = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);    
	        }
	        
			builder.setContentTitle(title)
					.setContentText(text)
					.setSound(uri);

	        
			notificationManager.notify(uid, 0, builder.build());
		}
	}

    static void NotifyDelayed(String uid, String title, String text, int delaySeconds, boolean useSound) {
        Context context = JNIApplication.GetApplication();
        AlarmManager alarmManager = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        Intent intent = new Intent(context, ScheduledNotificationReceiver.class);
        intent.putExtra("uid", uid);
        intent.putExtra("icon", icon);
        intent.putExtra("title", title);
        intent.putExtra("text", text);
        intent.putExtra("useSound", useSound);
		if(JNIActivity.GetActivity() != null) {
			intent.putExtra("activityClassName", JNIActivity.GetActivity().getClass().getName());
		}
        PendingIntent pendingIntent = PendingIntent.getBroadcast(context, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT);
        alarmManager.set(AlarmManager.RTC_WAKEUP, Calendar.getInstance().getTimeInMillis() + delaySeconds * 1000, pendingIntent);
    }

    static void RemoveAllDelayedNotifications() {
        Context context = JNIApplication.GetApplication();
        Intent intent = new Intent(context, ScheduledNotificationReceiver.class);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(context, 0, intent, PendingIntent.FLAG_CANCEL_CURRENT);
        AlarmManager alarmManager = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        alarmManager.cancel(pendingIntent);
		notificationManager.cancelAll();
    }

    static void HideNotification(String uid) {
		if (isInited){
			CleanBuilder();
			notificationManager.cancel(uid, 0);
		}
	}

	public static void AttachToActivity(JNIActivity activity) {
		if (isInited) {
            icon = activity.GetNotificationIcon();
            builder.setSmallIcon(icon);
		}
	}
    

}
