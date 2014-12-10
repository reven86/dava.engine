package com.dava.framework;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;

public class JNIAssert {
    
    public static boolean breakExecution = false;
    public static boolean alreadyShowingNonModalDialog = false;
    
	public static boolean Assert(final boolean isModal,
	        final String message)
	{
	    if (!isModal && alreadyShowingNonModalDialog)
	    {
	        // skip follow non modal messages while user looking at first
	        return false;
	    }
	    JNIAssert.breakExecution = false;
	    
		Activity activity = JNIActivity.GetActivity();
		final AlertDialog.Builder alertDialog = new AlertDialog.Builder(activity);
		alertDialog.setMessage(message);
		if (isModal)
		{
		    waitUserInput(activity, alertDialog);
		} else
		{
		    showDialogAndContinue(activity, alertDialog);
		}
		return breakExecution;
	}

    private static void showDialogAndContinue(final Activity activity,
            final AlertDialog.Builder alertDialog) {
        alreadyShowingNonModalDialog = true;
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                // close dialog on click outside
                alertDialog.setCancelable(true);
                alertDialog.setPositiveButton("Ok", new OnClickListener() {
                    
                    public void onClick(DialogInterface dialog, int which) {
                        alreadyShowingNonModalDialog = false;
                    }
                });
                alertDialog.setOnCancelListener(new DialogInterface.OnCancelListener() {
                    
                    @Override
                    public void onCancel(DialogInterface dialog) {
                        alreadyShowingNonModalDialog = false;
                    }
                });
                alertDialog.show();
            }
        });
    }

    private static void waitUserInput(final Activity activity,
            final AlertDialog.Builder alertDialog) {
        final Object mutex = new Object();
        activity.runOnUiThread(new Runnable() {
        	@Override
        	public void run() {
        	    // click outside dialog do nothing
        	    alertDialog.setCancelable(false);
        		alertDialog.setPositiveButton("Break", new OnClickListener() {
        			
        			public void onClick(DialogInterface dialog, int which) {
        				synchronized (mutex) {
        				    breakExecution = true;
        					mutex.notify();
        				}
        			}
        		});
        		alertDialog.setNegativeButton("Ok", new OnClickListener() {
                    
                    public void onClick(DialogInterface dialog, int which) {
                        synchronized (mutex) {
                            mutex.notify();
                            
                        }
                    }
                });
        		alertDialog.show();
        	}
        });
        synchronized (mutex) {
        	try {
        		mutex.wait();
        	} catch (InterruptedException e) {
        		e.printStackTrace();
        	}
        }
    }
}
