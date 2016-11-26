package com.dava.engine;

import android.content.Context;
import android.app.Application;

import android.app.AlarmManager;
import android.content.res.AssetManager;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Intent;
import android.media.RingtoneManager;
import android.net.Uri;
import android.support.v4.app.NotificationCompat;

import android.util.Log;

import java.util.Calendar;

public class DavaNotificationProvider {
    private static NotificationCompat.Builder builder = null;
    private static NotificationManager notificationManager = null;
    private static AssetManager assetsManager = null;
    private static boolean isInited = false;
    private static int icon;

    private static Context context;
    private static DavaActivity activity;
 
    static void Init(DavaActivity davaActivity)
    {
        Log.d(DavaActivity.LOG_TAG, "DavaNotificationProvider.Init");
        activity = davaActivity;
        context = davaActivity.getApplication();
        
        assetsManager = context.getAssets();
        notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        builder = new NotificationCompat.Builder(context);

        isInited = (null != notificationManager && null != assetsManager);
        if (!isInited)
        {
            Log.d(DavaActivity.LOG_TAG, "DavaNotificationProvider not inited!");
            return;
        }
        icon = android.R.drawable.sym_def_app_icon;
        builder.setSmallIcon(icon);
    }

    static void SetNotificationIcon(int value)
    {
        icon = value;
        builder.setSmallIcon(icon);
    }

    static int GetNotificationIcon()
    {
        return icon;
    }

    static void CleanBuilder()
    {
        Log.d(DavaActivity.LOG_TAG, "DavaNotificationProvider.CleanBuilder");
        builder.setContentTitle("").setContentText("").setProgress(0, 0, false);
    }

    static void EnableTapAction(String uid)
    {
        Log.d(DavaActivity.LOG_TAG, "DavaNotificationProvider.EnableTapAction");
        if (isInited)
        {
            CleanBuilder();
            
            Intent intent = new Intent(activity, activity.getClass());
            intent.putExtra("uid", uid);
            PendingIntent pIntent = PendingIntent.getActivity(activity, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT);
            builder.setContentIntent(pIntent);

            notificationManager.notify(uid, 0, builder.build());
        }
    }
    
    static void NotifyProgress(String uid, String title, String text, int maxValue, int value, boolean useSound)
    {
        if (isInited)
        {
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
    
    static void NotifyText(String uid, String title, String text, boolean useSound)
    {
        Log.d(DavaActivity.LOG_TAG, "DavaNotificationProvider.NotifyText");
        if (isInited)
        {
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

    static void NotifyDelayed(String uid, String title, String text, int delaySeconds, boolean useSound)
    {
        Log.d(DavaActivity.LOG_TAG, "DavaNotificationProvider.NotifyDelayed");
        AlarmManager alarmManager = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        Intent intent = new Intent(context, ScheduledNotificationReceiver.class);
        intent.putExtra("uid", uid);
        intent.putExtra("icon", icon);
        intent.putExtra("title", title);
        intent.putExtra("text", text);
        intent.putExtra("useSound", useSound);
        if(activity != null)
        {
            intent.putExtra("activityClassName", activity.getClass().getName());
        }
        PendingIntent pendingIntent = PendingIntent.getBroadcast(context, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT);
        alarmManager.set(AlarmManager.RTC_WAKEUP, Calendar.getInstance().getTimeInMillis() + delaySeconds * 1000, pendingIntent);
    }

    static void RemoveAllDelayedNotifications()
    {
        Log.d(DavaActivity.LOG_TAG, "DavaNotificationProvider.RemoveAllDelayedNotifications");
        Intent intent = new Intent(context, ScheduledNotificationReceiver.class);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(context, 0, intent, PendingIntent.FLAG_CANCEL_CURRENT);
        AlarmManager alarmManager = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        alarmManager.cancel(pendingIntent);
        notificationManager.cancelAll();
    }

    static void HideNotification(String uid)
    {
        Log.d(DavaActivity.LOG_TAG, "DavaNotificationProvider.HideNotification");
        if (isInited)
        {
            CleanBuilder();
            notificationManager.cancel(uid, 0);
        }
    }

}
