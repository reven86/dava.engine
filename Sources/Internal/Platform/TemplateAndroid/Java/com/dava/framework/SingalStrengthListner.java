package com.dava.framework;

import android.telephony.PhoneStateListener;
import android.telephony.SignalStrength;

public class SingalStrengthListner extends PhoneStateListener {
	
	private int singalStrength = 0;
	
	@Override
	public void onSignalStrengthsChanged(SignalStrength signalStrength) {
		singalStrength = signalStrength.getGsmSignalStrength();
		super.onSignalStrengthsChanged(signalStrength);
	}

	public int GetSignalStrength() {
		return singalStrength;
	}
}
