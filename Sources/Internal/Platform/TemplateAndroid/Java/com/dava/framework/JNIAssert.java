package com.dava.framework;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;

public class JNIAssert {
    
    public static volatile boolean waitUserInputOnAssertDialog = false;
    
    private static boolean breakExecution = false;
    private static boolean alreadyShowingNonModalDialog = false;
    
	public static synchronized boolean Assert(final boolean isModal,
	        final String message)
	{
	    if (!isModal && alreadyShowingNonModalDialog)
	    {
	        // skip follow non modal messages while user looking at first
	        return false;
	    }
	    
		Activity activity = JNIActivity.GetActivity();
		final AlertDialog.Builder alertDialog = new AlertDialog.Builder(activity);
		alertDialog.setMessage(message);
		if (isModal)
		{
		    waitUserInputOnAssertDialog = true;
		    
		    waitUserInput(activity, alertDialog);
		    
		    waitUserInputOnAssertDialog = false;
		    
		    return breakExecution;
		} else
		{
		    showDialogAndContinue(activity, alertDialog);
		    return false;
		}
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
        		alertDialog.setPositiveButton("Ok", new OnClickListener() {
        			
        			public void onClick(DialogInterface dialog, int which) {
        				synchronized (mutex) {
        				    breakExecution = false;
        					mutex.notify();
        				}
        			}
        		});
        		alertDialog.setNegativeButton("Break", new OnClickListener() {
                    
                    public void onClick(DialogInterface dialog, int which) {
                        synchronized (mutex) {
                            breakExecution = true;
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
