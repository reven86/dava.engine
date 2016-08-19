package com.dava.engine;

import android.os.Handler;
import android.os.Message;

class DavaCommandHandler extends Handler
{
    public void sendCommand(int command, Object param)
    {
        Message msg = obtainMessage();
        msg.arg1 = command;
        msg.obj = param;
        sendMessage(msg);
    }
    
    @Override
    public void handleMessage(Message msg)
    {
        switch (msg.arg1)
        {
        default:
            super.handleMessage(msg);
            break;
        }
    }
}
