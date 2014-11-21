package com.dava.framework;

import android.app.IntentService;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.BroadcastReceiver;
import android.support.v4.app.NotificationCompat;

public class ScheduledNotificationReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        JNIActivity activity = JNIActivity.GetActivity();
        Intent tapIntent = new Intent(activity, activity.getClass());
        PendingIntent pendingIntent = PendingIntent.getActivity(activity, 0, tapIntent, PendingIntent.FLAG_UPDATE_CURRENT);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(context);
        String uid = intent.getStringExtra("uid");
        builder.setContentTitle(intent.getStringExtra("title"));
        builder.setContentText(intent.getStringExtra("text"));
        builder.setSmallIcon(intent.getIntExtra("icon", 0));
        builder.setContentIntent(pendingIntent);
        builder.setAutoCancel(true);
        NotificationManager notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.notify(uid, 0, builder.build());
    }
}
