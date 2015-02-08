
#ifndef __DAVAENGINE_MEMPROFSERVICE_H__
#define __DAVAENGINE_MEMPROFSERVICE_H__

#include "Base/BaseTypes.h"
#include "Network/NetService.h"

namespace DAVA
{
namespace Net
{

class MemProfService : public NetService
{
public:
    MemProfService();
    virtual ~MemProfService();

    void ChannelOpen() override;
    void ChannelClosed(const char8* message) override;
    void PacketReceived(const void* packet, size_t length) override;
    void PacketSent() override;
    void PacketDelivered() override;
    
private:
};

}   // namespace Net
}   // namespace DAVA

#endif // __DAVAENGINE_MEMPROFSERVICE_H__

