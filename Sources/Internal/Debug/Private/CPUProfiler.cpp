#include "Debug/CPUProfiler.h"
#include "Platform/SystemTimer.h"
#include "Concurrency/Thread.h"
#include "Base/AllocatorFactory.h"
#include "Debug/DVAssert.h"
#include "ProfilerRingArray.h"
#include <ostream>

//==============================================================================

namespace DAVA
{

#if CPU_PROFILER_ENABLED
static CPUProfiler GLOBAL_TIME_PROFILER;
CPUProfiler* const CPUProfiler::globalProfiler = &GLOBAL_TIME_PROFILER;
#else
CPUProfiler* const CPUProfiler::globalProfiler = nullptr;
#endif

//////////////////////////////////////////////////////////////////////////
//Internal Declaration

struct CPUProfiler::Counter
{
    uint64 startTime = 0;
    uint64 endTime = 0;
    const char* name = nullptr;
    uint64 threadID = 0;
};

namespace CPUProfilerDetails
{
struct CounterTreeNode
{
    IMPLEMENT_POOL_ALLOCATOR(CounterTreeNode, 128);

    static CounterTreeNode* BuildTree(CPUProfiler::CounterArray::iterator begin, CPUProfiler::CounterArray* array);
    static CounterTreeNode* CopyTree(const CounterTreeNode* node);
    static void MergeTree(CounterTreeNode* root, const CounterTreeNode* node);
    static void DumpTree(const CounterTreeNode* node, std::ostream& stream, bool average);
    static void SafeDeleteTree(CounterTreeNode*& node);

protected:
    CounterTreeNode(CounterTreeNode* _parent, const char* _name, uint64 _time, uint32 _count)
        : counterTime(_time)
        , counterName(_name)
        , parent(_parent)
        , count(_count)
    {
    }

    uint64 counterTime = 0;
    const char* counterName;

    CounterTreeNode* parent;
    Vector<CounterTreeNode*> childs;
    uint32 count; //recursive and duplicate counters
};

uint64 TimeStampUs()
{
    return DAVA::SystemTimer::Instance()->GetAbsoluteUs();
}

bool NameEquals(const char* name1, const char* name2)
{
#ifdef __DAVAENGINE_DEBUG__
    return (strcmp(name1, name2) == 0);
#else
    return name1 == name2;
#endif
}
}

//////////////////////////////////////////////////////////////////////////

CPUProfiler::ScopedCounter::ScopedCounter(const char* counterName, CPUProfiler* _profiler)
{
    profiler = _profiler;
    if (profiler->started)
    {
        Counter& c = profiler->counters->next();

        endTime = &c.endTime;
        c.startTime = CPUProfilerDetails::TimeStampUs();
        c.endTime = 0;
        c.name = counterName;
        c.threadID = Thread::GetCurrentIdAsUInt64();
    }
}

CPUProfiler::ScopedCounter::~ScopedCounter()
{
    // We don't write end time if profiler stopped cause
    // in this moment thread may other dump counters.
    // Potentially due to 'pseudo-thread-safe' (see ProfilerRingArray.h)
    // we can get invalid counter (only one, therefore there is 'if(started)' ).
    // We know it. But it performance reason.
    if (profiler->started && endTime)
    {
        *endTime = CPUProfilerDetails::TimeStampUs();
    }
}

CPUProfiler::CPUProfiler(uint32 countersCount)
{
    counters = new CounterArray(countersCount);
}

CPUProfiler::~CPUProfiler()
{
    DeleteSnapshots();
    SafeDelete(counters);
}

void CPUProfiler::Start()
{
    started = true;
}

void CPUProfiler::Stop()
{
    started = false;
}

int32 CPUProfiler::MakeSnapshot()
{
    //CPU profiler use 'pseudo-thread-safe' ring array (see ProfilerRingArray.h)
    //So we can't read array when other thread may write
    //For performance reasons we should stop profiler before dumping or snapshotting
    DVASSERT(!started && "Stop profiler before make snapshot");

    snapshots.push_back(new CounterArray(*counters));
    return int32(snapshots.size() - 1);
}

void CPUProfiler::DeleteSnapshot(int32 snapshot)
{
    if (snapshot != NO_SNAPSHOT_ID)
    {
        DVASSERT(snapshot >= 0 && snapshot < int32(snapshots.size()));
        SafeDelete(snapshots[snapshot]);
        snapshots.erase(snapshots.begin() + snapshot);
    }
}

void CPUProfiler::DeleteSnapshots()
{
    for (CounterArray*& c : snapshots)
        SafeDelete(c);
    snapshots.clear();
}

uint64 CPUProfiler::GetLastCounterTime(const char* counterName)
{
    uint64 timeDelta = 0;
    CounterArray::reverse_iterator it = counters->rbegin(), itEnd = counters->rend();
    for (; it != itEnd; it++)
    {
        const Counter& c = *it;
        if (c.endTime != 0 && (strcmp(counterName, c.name) == 0))
        {
            timeDelta = c.endTime - c.startTime;
            break;
        }
    }

    return timeDelta;
}

void CPUProfiler::DumpLast(const char* counterName, uint32 counterCount, std::ostream& stream, int32 snapshot)
{
    DVASSERT((snapshot != NO_SNAPSHOT_ID || !started) && "Stop profiler before dumping");

    stream << "================================================================" << std::endl;

    CounterArray* array = GetCounterArray(snapshot);
    CounterArray::reverse_iterator it = array->rbegin(), itEnd = array->rend();
    Counter* lastDumpedCounter = nullptr;
    for (; it != itEnd; it++)
    {
        if (it->endTime != 0 && (strcmp(counterName, it->name) == 0))
        {
            if (lastDumpedCounter)
                stream << "=== Non-tracked time [" << (lastDumpedCounter->startTime - it->endTime) << " us] ===" << std::endl;
            lastDumpedCounter = &(*it);

            CPUProfilerDetails::CounterTreeNode* treeRoot = CPUProfilerDetails::CounterTreeNode::BuildTree(CounterArray::iterator(it), array);
            CPUProfilerDetails::CounterTreeNode::DumpTree(treeRoot, stream, false);
            SafeDelete(treeRoot);

            counterCount--;
        }

        if (counterCount == 0)
            break;
    }

    stream << "================================================================" << std::endl;
    stream.flush();
}

void CPUProfiler::DumpAverage(const char* counterName, uint32 counterCount, std::ostream& stream, int32 snapshot)
{
    DVASSERT((snapshot != NO_SNAPSHOT_ID || !started) && "Stop profiler before dumping");

    stream << "================================================================" << std::endl;
    stream << "=== Average time for " << counterCount << " counter(s):" << std::endl;

    CounterArray* array = GetCounterArray(snapshot);
    CounterArray::reverse_iterator it = array->rbegin(), itEnd = array->rend();
    CPUProfilerDetails::CounterTreeNode* treeRoot = nullptr;
    for (; it != itEnd; it++)
    {
        if (it->endTime != 0 && (strcmp(counterName, it->name) == 0))
        {
            CPUProfilerDetails::CounterTreeNode* node = CPUProfilerDetails::CounterTreeNode::BuildTree(CounterArray::iterator(it), array);

            if (treeRoot)
            {
                CPUProfilerDetails::CounterTreeNode::MergeTree(treeRoot, node);
                CPUProfilerDetails::CounterTreeNode::SafeDeleteTree(node);
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
        CPUProfilerDetails::CounterTreeNode::DumpTree(treeRoot, stream, true);
        CPUProfilerDetails::CounterTreeNode::SafeDeleteTree(treeRoot);
    }

    stream << "================================================================" << std::endl;
    stream.flush();
}

void CPUProfiler::DumpJSON(std::ostream& stream, int32 snapshot)
{
    TraceEvent::DumpJSON(GetTrace(snapshot), stream);
}

Vector<TraceEvent> CPUProfiler::GetTrace(int32 snapshot)
{
    DVASSERT((snapshot != NO_SNAPSHOT_ID || !started) && "Stop profiler before tracing");

    CounterArray* array = GetCounterArray(snapshot);
    Vector<TraceEvent> trace;
    for (const Counter& c : *array)
    {
        if (!c.name)
            continue;

        trace.push_back(TraceEvent(FastName(c.name), 0, c.threadID, c.startTime, c.endTime ? (c.endTime - c.startTime) : 0, TraceEvent::PHASE_DURATION));
    }

    return trace;
}

Vector<TraceEvent> CPUProfiler::GetTrace(const char* counterName, uint32 counterCount, int32 snapshot)
{
    DVASSERT((snapshot != NO_SNAPSHOT_ID || !started || counterCount == 1) && "Stop profiler before tracing");

    CounterArray* array = GetCounterArray(snapshot);
    CounterArray::reverse_iterator begin = array->rbegin(), end = array->rend();
    CounterArray::reverse_iterator it = begin;
    for (; it != end; ++it)
    {
        if (it->endTime != 0 && (strcmp(counterName, it->name) == 0))
            counterCount--;

        if (counterCount == 0)
            break;
    }

    uint64 threadID = it->threadID;
    Vector<TraceEvent> trace;
    for (; it != begin; --it)
    {
        if (it->threadID == threadID)
        {
            if (it->endTime == 0)
                break;

            trace.push_back(TraceEvent(FastName(it->name), 0, it->threadID, it->startTime, it->endTime - it->startTime, TraceEvent::PHASE_DURATION));
        }
    }

    return trace;
}

CPUProfiler::CounterArray* CPUProfiler::GetCounterArray(int32 snapshot)
{
    if (snapshot != CPUProfiler::NO_SNAPSHOT_ID)
    {
        DVASSERT(snapshot >= 0 && snapshot < int32(snapshots.size()));
        return snapshots[snapshot];
    }

    return counters;
}

/////////////////////////////////////////////////////////////////////////////////
//Internal Definition
namespace CPUProfilerDetails
{
CounterTreeNode* CounterTreeNode::BuildTree(CPUProfiler::CounterArray::iterator begin, CPUProfiler::CounterArray* array)
{
    DVASSERT(begin->endTime);

    uint64 threadID = begin->threadID;
    uint64 endTime = begin->endTime;
    Vector<uint64> nodeEndTime;

    CounterTreeNode* node = new CounterTreeNode(nullptr, begin->name, begin->endTime - begin->startTime, 1);
    nodeEndTime.push_back(begin->endTime);

    CPUProfiler::CounterArray::iterator end = array->end();
    for (CPUProfiler::CounterArray::iterator it = begin + 1; it != end; ++it)
    {
        const CPUProfiler::Counter& c = *it;

        if (c.threadID != threadID)
            continue;

        if (c.startTime >= endTime || c.endTime == 0)
            break;

        if (c.threadID == threadID)
        {
            while (nodeEndTime.size() && (c.startTime >= nodeEndTime.back()))
            {
                DVASSERT(node->parent);

                nodeEndTime.pop_back();
                node = node->parent;
            }

            if (CPUProfilerDetails::NameEquals(node->counterName, c.name))
            {
                node->counterTime += c.endTime - c.startTime;
                node->count++;
            }
            else
            {
                auto found = std::find_if(node->childs.begin(), node->childs.end(), [&c](CounterTreeNode* node) {
                    return (CPUProfilerDetails::NameEquals(c.name, node->counterName));
                });

                if (found != node->childs.end())
                {
                    (*found)->counterTime += c.endTime - c.startTime;
                    (*found)->count++;
                }
                else
                {
                    node->childs.push_back(new CounterTreeNode(node, c.name, c.endTime - c.startTime, 1));
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
    if (CPUProfilerDetails::NameEquals(root->counterName, node->counterName))
    {
        root->counterTime += node->counterTime;
        root->count++;

        for (CounterTreeNode* c : node->childs)
            MergeTree(root, c);
    }
    else
    {
        auto found = std::find_if(root->childs.begin(), root->childs.end(), [&node](CounterTreeNode* child) {
            return (CPUProfilerDetails::NameEquals(child->counterName, node->counterName));
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
    CounterTreeNode* nodeCopy = new CounterTreeNode(nullptr, node->counterName, node->counterTime, node->count);
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

} //ns CPUProfilerDetails

} //ns DAVA
