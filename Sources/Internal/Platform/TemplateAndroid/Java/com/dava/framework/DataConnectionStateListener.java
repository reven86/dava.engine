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
        String carrierName = "unknown";

        TelephonyManager tm;
        if (JNIActivity.GetActivity() != null)
        {
            tm = (TelephonyManager)JNIActivity.GetActivity().getSystemService(Context.TELEPHONY_SERVICE);
        }
        else
        {
            tm = (TelephonyManager)DavaActivity.instance().getSystemService(Context.TELEPHONY_SERVICE);
        }

        if (tm != null && tm.getSimOperatorName() != null)
        {
            carrierName = tm.getSimOperatorName();
        }

        return carrierName;
    }
}
