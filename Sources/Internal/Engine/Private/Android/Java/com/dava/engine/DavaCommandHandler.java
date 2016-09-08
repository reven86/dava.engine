package com.dava.engine;

import android.os.Handler;
import android.os.Message;

class eHandlerCommand
{
    static final int TRIGGER_PROCESS_EVENTS = 1;
}

final class DavaCommandHandler extends Handler
{
    public void sendCommand(int command, Object param)
    {
        Message msg = obtainMessage();
        msg.what = command;
        msg.obj = param;
        sendMessage(msg);
    }

    public void sendTriggerProcessEvents(DavaSurfaceView view)
    {
        sendCommand(eHandlerCommand.TRIGGER_PROCESS_EVENTS, view);
    }
    
    @Override
    public void handleMessage(Message msg)
    {
        switch (msg.what)
        {
        case eHandlerCommand.TRIGGER_PROCESS_EVENTS:
            DavaSurfaceView view = (DavaSurfaceView)msg.obj;
            view.processEvents();
            break;
        default:
            super.handleMessage(msg);
            break;
        }
    }
}
