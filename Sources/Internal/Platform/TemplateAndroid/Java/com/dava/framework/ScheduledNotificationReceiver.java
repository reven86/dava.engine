package com.dava.framework;

import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.media.RingtoneManager;
import android.net.Uri;
import android.support.v4.app.NotificationCompat;
import android.util.Log;

public class ScheduledNotificationReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d("Local Notifications", "ScheduledNotificationReceiver onReceive");
        JNIActivity activity = JNIActivity.GetActivity();
        Intent tapIntent;
        if(activity != null) {
            tapIntent = new Intent(context, activity.getClass());
        } else {
            String activityClassName = intent.getStringExtra("activityClassName");
            try {
                Class<?> activityClass = Class.forName(activityClassName);
                tapIntent = new Intent(context, activityClass);
            } catch (ClassNotFoundException e) {
                Log.d("Local Notifications", "Incorrect activityClassName");
                return;
            }
        }

        PendingIntent pendingIntent = PendingIntent.getActivity(context, 0, tapIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        NotificationCompat.Builder builder = new NotificationCompat.Builder(context);
        
        Uri uri = null;
        if (intent.getBooleanExtra("useSound", false))
        {
            uri = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);
        }
        String uid = intent.getStringExtra("uid");
        builder.setContentTitle(intent.getStringExtra("title"))
        	   .setContentText(intent.getStringExtra("text"))
               .setSmallIcon(intent.getIntExtra("icon", 0))
               .setContentIntent(pendingIntent)
               .setAutoCancel(true)
               .setSound(uri);

        NotificationManager notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.notify(uid, 0, builder.build());
    }
}
