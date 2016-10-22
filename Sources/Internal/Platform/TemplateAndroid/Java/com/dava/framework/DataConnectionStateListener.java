package com.dava.framework;

import com.dava.engine.DavaActivity;

import android.content.Context;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;

public class DataConnectionStateListener extends PhoneStateListener {
    private String carrierName = GetCarrierName();
    
    private native void OnCarrierNameChanged(); 

    @Override
    public void onDataConnectionStateChanged(int state, int networkType) {
        String newCarrierName = GetCarrierName();
        if (!newCarrierName.equals(carrierName))
        {
            carrierName = newCarrierName;
            if (JNIActivity.GetActivity() != null)
            {
                JNIActivity.GetActivity().RunOnMainLoopThread(new Runnable() 
                {
                    public void run()
                    {
                        OnCarrierNameChanged();
                    }
                });
            }
            else
            {
                // TODO: add callback in Core V2
                OnCarrierNameChanged();
            }
        }
        super.onDataConnectionStateChanged(state, networkType);
    }

    public String GetCarrierName()
    {
        TelephonyManager manager;
        if (JNIActivity.GetActivity() != null)
        {
            manager = (TelephonyManager)JNIActivity.GetActivity().getSystemService(Context.TELEPHONY_SERVICE);
        }
        else
        {
            manager = (TelephonyManager)DavaActivity.instance().getSystemService(Context.TELEPHONY_SERVICE);
        }
        return manager.getSimOperatorName();
    }
}
