#ifndef UWP_LOG_CONSUMER_H
#define UWP_LOG_CONSUMER_H

#include "Base/BaseTypes.h"
#include "Network/Services/LogConsumer.h"

class UWPLogConsumer : public DAVA::Net::LogConsumer
{
public:
    UWPLogConsumer();

    bool IsSessionEnded();
    bool HasReceivedData();
    DAVA::Signal<const DAVA::String&> newMessageNotifier;

private:
    //NetService method implementation
    void ChannelOpen() override;
    void ChannelClosed(const DAVA::char8* message) override;

    void OnNewData(const DAVA::String& str);

    bool channelOpened;
    bool dataReceived = false;
};

#endif // UWP_LOG_CONSUMER_H