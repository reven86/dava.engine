#include "Debug/Profiler.h"
#include "Platform/SystemTimer.h"
#include "Concurrency/Thread.h"
#include "Base/AllocatorFactory.h"
#include "Debug/DVAssert.h"
#include "ProfilerRingArray.h"
#include <ostream>

//==============================================================================

namespace DAVA
{
namespace Profiler
{
const static uint32 TIME_COUNTERS_COUNT = 2048;

struct TimeCounter
{
    uint64 startTime = 0;
    uint64 endTime = 0;
    const char* name = nullptr;
    Thread::Id tid = 0;
    uint32 nameID = 0;
};

using CounterArray = RingArray<TimeCounter, TIME_COUNTERS_COUNT>;

static CounterArray counters;
static bool profilerStarted = false;

Vector<CounterArray> snapshots;

//////////////////////////////////////////////////////////////////////////
//Internal Declaration

struct CounterTreeNode
{
    IMPLEMENT_POOL_ALLOCATOR(CounterTreeNode, 128);

    static CounterTreeNode* BuildTree(CounterArray::iterator begin, CounterArray& array);
    static CounterTreeNode* CopyTree(const CounterTreeNode* node);
    static void MergeTree(CounterTreeNode* root, const CounterTreeNode* node);
    static void DumpTree(const CounterTreeNode* node, std::ostream& stream, bool average);
    static void SafeDeleteTree(CounterTreeNode*& node);

protected:
    CounterTreeNode(CounterTreeNode* _parent, const char* _name, uint32 _nameID, uint64 _time, uint32 _count)
        : counterTime(_time)
        , counterName(_name)
        , counterNameID(_nameID)
        , parent(_parent)
        , count(_count)
    {
    }

    uint64 counterTime = 0;
    const char* counterName;
    uint32 counterNameID;

    CounterTreeNode* parent;
    Vector<CounterTreeNode*> childs;
    uint32 count; //recursive and duplicate counters
};

uint64 TimeStampUs();
CounterArray& GetCounterArray(int32 snapshot);

//////////////////////////////////////////////////////////////////////////

ScopedCounter::ScopedCounter(const char* counterName, uint32 counterNameID)
{
    if (profilerStarted)
    {
        TimeCounter& c = counters.next();

        endTime = &c.endTime;
        c.startTime = TimeStampUs();
        c.endTime = 0;
        c.name = counterName;
        c.tid = Thread::GetCurrentId();
        c.nameID = counterNameID;
    }
}

ScopedCounter::~ScopedCounter()
{
    if (profilerStarted && endTime)
    {
        *endTime = TimeStampUs();
    }
}

void Start()
{
    profilerStarted = true;
}

void Stop()
{
    profilerStarted = false;
}

int32 MakeSnapshot()
{
    snapshots.push_back(counters);
    return int32(snapshots.size() - 1);
}

void DeleteSnapshot(int32 snapshot)
{
    if (snapshot != NO_SNAPSHOT_ID)
    {
        DVASSERT(snapshot >= 0 && snapshot < int32(snapshots.size()))
        snapshots.erase(snapshots.begin() + snapshot);
    }
}

void DeleteSnapshots()
{
    snapshots.clear();
}

uint64 GetLastCounterTime(const char* counterName)
{
    uint32 counterNameID = HashValue_N(counterName, uint32(strlen(counterName)));
    uint64 timeDelta = 0;
    CounterArray::reverse_iterator it = counters.rbegin(), itEnd = counters.rend();
    for (; it != itEnd; it++)
    {
        const TimeCounter& c = *it;
        if (c.endTime != 0 && counterNameID == c.nameID)
        {
            timeDelta = c.endTime - c.startTime;
            break;
        }
    }

    return timeDelta;
}

void DumpLast(const char* counterName, uint32 counterCount, std::ostream& stream, int32 snapshot)
{
    DVASSERT((snapshot != NO_SNAPSHOT_ID || !profilerStarted) && "Stop profiler before dumping");

    stream << "================================================================" << std::endl;

    uint32 counterNameID = HashValue_N(counterName, uint32(strlen(counterName)));
    CounterArray& array = GetCounterArray(snapshot);
    CounterArray::reverse_iterator it = array.rbegin(), itEnd = array.rend();
    TimeCounter* lastDumpedCounter = nullptr;
    for (; it != itEnd; it++)
    {
        if (it->endTime != 0 && counterNameID == it->nameID)
        {
            if (lastDumpedCounter)
                stream << "=== Non-tracked time [" << (lastDumpedCounter->endTime - it->startTime) << " us] ===" << std::endl;
            lastDumpedCounter = &(*it);

            CounterTreeNode* treeRoot = CounterTreeNode::BuildTree(CounterArray::iterator(it), array);
            CounterTreeNode::DumpTree(treeRoot, stream, false);
            SafeDelete(treeRoot);

            counterCount--;
        }

        if (counterCount == 0)
            break;
    }

    stream << "================================================================" << std::endl;
    stream.flush();
}

void DumpAverage(const char* counterName, uint32 counterCount, std::ostream& stream, int32 snapshot)
{
    DVASSERT((snapshot != NO_SNAPSHOT_ID || !profilerStarted) && "Stop profiler before dumping");

    stream << "================================================================" << std::endl;
    stream << "=== Average time for " << counterCount << " counter(s):" << std::endl;

    uint32 counterNameID = HashValue_N(counterName, uint32(strlen(counterName)));
    CounterArray& array = GetCounterArray(snapshot);
    CounterArray::reverse_iterator it = array.rbegin(), itEnd = array.rend();
    CounterTreeNode* treeRoot = nullptr;
    for (; it != itEnd; it++)
    {
        if (it->endTime != 0 && counterNameID == it->nameID)
        {
            CounterTreeNode* node = CounterTreeNode::BuildTree(CounterArray::iterator(it), array);

            if (treeRoot)
            {
                CounterTreeNode::MergeTree(treeRoot, node);
                CounterTreeNode::SafeDeleteTree(node);
            }
            else
            {
                treeRoot = node;
            }

            counterCount--;
        }

        if (counterCount == 0)
            break;
    }

    if (treeRoot)
    {
        CounterTreeNode::DumpTree(treeRoot, stream, true);
        CounterTreeNode::SafeDeleteTree(treeRoot);
    }

    stream << "================================================================" << std::endl;
    stream.flush();
}

void DumpJSON(std::ostream& stream, int32 snapshot)
{
    DVASSERT((snapshot != NO_SNAPSHOT_ID || !profilerStarted) && "Stop profiler before dumping");

    CounterArray& array = GetCounterArray(snapshot);

    stream << "{ \"traceEvents\": [" << std::endl;

    CounterArray::iterator it = array.begin(), itEnd = array.end();
    CounterArray::iterator last(array.rbegin());
    for (; it != itEnd; it++)
    {
        stream << "{ \"pid\":0, \"tid\":" << uint64(it->tid) << ", \"ts\":" << it->startTime << ", \"ph\":\"B\", \"cat\":\"\", \"name\":\"" << it->name << "\" }," << std::endl;
        stream << "{ \"pid\":0, \"tid\":" << uint64(it->tid) << ", \"ts\":" << (it->endTime ? it->endTime : it->startTime) << ", \"ph\":\"E\", \"cat\":\"\", \"name\":\"" << it->name << "\" }";
        if (it != last)
            stream << ",";
        stream << std::endl;
    }

    stream << "] }" << std::endl;

    stream.flush();
}

/////////////////////////////////////////////////////////////////////////////////
//Internal Definition

CounterTreeNode* CounterTreeNode::BuildTree(CounterArray::iterator begin, CounterArray& array)
{
    DVASSERT(begin->endTime);

    Thread::Id threadID = begin->tid;
    uint64 endTime = begin->endTime;
    Vector<uint64> nodeEndTime;

    CounterTreeNode* node = new CounterTreeNode(nullptr, begin->name, begin->nameID, begin->endTime - begin->startTime, 1);
    nodeEndTime.push_back(begin->endTime);

    CounterArray::iterator end = array.end();
    for (CounterArray::iterator it = begin + 1; it != end; ++it)
    {
        const TimeCounter& c = *it;

        if (c.tid != threadID)
            continue;

        if (c.startTime > endTime || c.endTime == 0)
            break;

        if (c.tid == threadID)
        {
            while (nodeEndTime.size() && (c.startTime >= nodeEndTime.back()))
            {
                DVASSERT(node->parent);

                nodeEndTime.pop_back();
                node = node->parent;
            }

            if (node->counterNameID == c.nameID)
            {
                node->counterTime += c.endTime - c.startTime;
                node->count++;
            }
            else
            {
                auto found = std::find_if(node->childs.begin(), node->childs.end(), [&c](CounterTreeNode* node) {
                    return (c.nameID == node->counterNameID);
                });

                if (found != node->childs.end())
                {
                    (*found)->counterTime += c.endTime - c.startTime;
                    (*found)->count++;
                }
                else
                {
                    node->childs.push_back(new CounterTreeNode(node, c.name, c.nameID, c.endTime - c.startTime, 1));
                    node = node->childs.back();

                    nodeEndTime.push_back(c.endTime);
                }
            }
        }
    }

    while (node->parent)
        node = node->parent;

    return node;
}

void CounterTreeNode::MergeTree(CounterTreeNode* root, const CounterTreeNode* node)
{
    if (root->counterNameID == node->counterNameID)
    {
        root->counterTime += node->counterTime;
        root->count++;

        for (CounterTreeNode* c : node->childs)
            MergeTree(root, c);
    }
    else
    {
        auto found = std::find_if(root->childs.begin(), root->childs.end(), [&node](CounterTreeNode* child) {
            return (child->counterNameID == node->counterNameID);
        });

        if (found != root->childs.end())
        {
            (*found)->counterTime += node->counterTime;
            (*found)->count++;

            for (CounterTreeNode* c : node->childs)
                MergeTree(*found, c);
        }
        else
        {
            root->childs.push_back(CounterTreeNode::CopyTree(node));
        }
    }
}

CounterTreeNode* CounterTreeNode::CopyTree(const CounterTreeNode* node)
{
    CounterTreeNode* nodeCopy = new CounterTreeNode(nullptr, node->counterName, node->counterNameID, node->counterTime, node->count);
    for (CounterTreeNode* c : node->childs)
    {
        nodeCopy->childs.push_back(CopyTree(c));
        nodeCopy->childs.back()->parent = nodeCopy;
    }

    return nodeCopy;
}

void CounterTreeNode::DumpTree(const CounterTreeNode* node, std::ostream& stream, bool average)
{
    if (!node)
        return;

    const CounterTreeNode* c = node;
    while (c->parent)
    {
        c = c->parent;
        stream << "  ";
    }

    stream << node->counterName << " [" << (average ? node->counterTime / node->count : node->counterTime) << " us | x" << node->count << "]" << std::endl;

    for (CounterTreeNode* c : node->childs)
        DumpTree(c, stream, average);
}

void CounterTreeNode::SafeDeleteTree(CounterTreeNode*& node)
{
    if (node)
    {
        for (CounterTreeNode* c : node->childs)
        {
            SafeDeleteTree(c);
        }
        node->childs.clear();
        SafeDelete(node);
    }
}

uint64 TimeStampUs()
{
    return DAVA::SystemTimer::Instance()->GetAbsoluteUs();
}

CounterArray& GetCounterArray(int32 snapshot)
{
    if (snapshot != NO_SNAPSHOT_ID)
    {
        DVASSERT(snapshot >= 0 && snapshot < int32(snapshots.size()));
        return snapshots[snapshot];
    }

    return counters;
}

} //ns Profiler
} //ns DAVA
