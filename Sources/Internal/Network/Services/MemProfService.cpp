#include "Network/Services/MemProfService.h"

namespace DAVA
{
namespace Net
{
 
MemProfService::MemProfService()
    : period(100)
    , passed(0)
    , timestamp(0)
    , maxQueueSize(20)
{
    
}

MemProfService::~MemProfService()
{
    for (auto* x : queue)
    {
        alloc.destroy(x);
        alloc.deallocate(x, sizeof(net_mem_stat_t));
        //delete x;
    }
}
    
void MemProfService::OnUpdate(float32 timeElapsed)
{
    passed += static_cast<uint32>(timeElapsed * 1000.0f);
    if (passed < period) return;

#if defined(MEMPROF_ENABLE)
    net_mem_stat_t* stat = alloc.allocate(1);
    alloc.construct(stat, net_mem_stat_t());
    //net_mem_stat_t* stat = new net_mem_stat_t;
    mem_profiler::get_memstat(stat);
    
    stat->timestamp = timestamp;
    timestamp += 1;
    if (Enqueue(stat))
        SendNextRecord();
#endif
    
    passed = 0;
}

void MemProfService::ChannelOpen()
{
    SendNextRecord();
}

void MemProfService::ChannelClosed(const char8* message)
{
    
}

void MemProfService::PacketReceived(const void* packet, size_t length)
{
    
}

void MemProfService::PacketSent()
{
    
}

void MemProfService::PacketDelivered()
{
    RemoveFirstMessage();
    SendNextRecord();
}

void MemProfService::SendNextRecord()
{
    net_mem_stat_t* stat = nullptr;
    if (IsChannelOpen() && (stat = GetFirstMessage()) != nullptr)
    {
        Send(stat, sizeof(net_mem_stat_t));
    }
}

bool MemProfService::Enqueue(net_mem_stat_t* h)
{
    bool wasEmpty = queue.empty();
    if (maxQueueSize <= queue.size())
        RemoveFirstMessage();
    queue.push_back(h);
    return wasEmpty;
}

net_mem_stat_t* MemProfService::GetFirstMessage()
{
    if(!queue.empty())
    {
        return queue.front();
    }
    return nullptr;
}

void MemProfService::RemoveFirstMessage()
{
    if(!queue.empty())
    {
        net_mem_stat_t* h = queue.front();
        alloc.destroy(h);
        alloc.deallocate(h, sizeof(net_mem_stat_t));
        //delete h;
        queue.pop_front();
    }
}

}   // namespace Net
}   // namespace DAVA
