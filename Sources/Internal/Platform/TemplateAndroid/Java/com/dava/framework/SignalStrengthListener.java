package com.dava.framework;

import android.telephony.PhoneStateListener;
import android.telephony.SignalStrength;

public class SignalStrengthListener extends PhoneStateListener {
    
    private int signalStrength = 0;
    
    @Override
    public void onSignalStrengthsChanged(SignalStrength signalStrength) {
        this.signalStrength = signalStrength.getGsmSignalStrength();
        super.onSignalStrengthsChanged(signalStrength);
    }

    public int GetSignalStrength() {
        return signalStrength;
    }
}
