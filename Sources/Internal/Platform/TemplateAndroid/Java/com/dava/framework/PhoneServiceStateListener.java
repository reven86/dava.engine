package com.dava.framework;

import com.dava.engine.DavaActivity;

import android.content.Context;
import android.telephony.PhoneStateListener;
import android.util.Log;
import android.telephony.TelephonyManager;
import android.telephony.ServiceState;

public class PhoneServiceStateListener extends PhoneStateListener {
    final static String TAG = "PhoneServiceStateListener";

    private String carrierName;
    
    @Override
    public void onServiceStateChanged(ServiceState state) {
		
		Log.e(TAG, "!!!!!! onServiceStateChanged 1");
        carrierName = GetCarrierName();
		Log.e(TAG, "!!!!!! onServiceStateChanged 2");
        super.onServiceStateChanged(state);
		Log.e(TAG, "!!!!!! onServiceStateChanged 3");
    }

    @Override
    public void onDataConnectionStateChanged(int state, int networkType) {
		
		Log.e(TAG, "!!!!!! onDataConnectionStateChanged 1");
        carrierName = GetCarrierName();
		Log.e(TAG, "!!!!!! onDataConnectionStateChanged 2");
        super.onDataConnectionStateChanged(state, networkType);
		Log.e(TAG, "!!!!!! onDataConnectionStateChanged 3");
    }

	public String GetCarrierName()
	{
        TelephonyManager manager;
		String carrierName1;

        if (JNIActivity.GetActivity() != null)
        {
            manager = (TelephonyManager)JNIActivity.GetActivity().getSystemService(Context.TELEPHONY_SERVICE);
        }
        else
        {
            manager = (TelephonyManager)DavaActivity.instance().getSystemService(Context.TELEPHONY_SERVICE);
        }
       
		carrierName1 = manager.getNetworkOperatorName();
        Log.e(TAG, "!!!!!! manager.getNetworkOperatorName" + carrierName1);
		carrierName1 = manager.getNetworkOperator();
        Log.e(TAG, "!!!!!! manager.getNetworkOperator" + carrierName1);
		carrierName1 = manager.getSimOperatorName();
        Log.e(TAG, "!!!!!! manager.getSimOperatorName" + carrierName1);

		return carrierName1;
	}
}
