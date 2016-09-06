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

struct CPUProfiler::TimeCounter
{
    uint64 startTime = 0;
    uint64 endTime = 0;
    const char* name = nullptr;
    Thread::Id tid = 0;
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

bool NameCmp(const char* name1, const char* name2)
{
#ifdef __DAVAENGINE_DEBUG__
    return strcmp(name1, name2);
#else
    return name1 == name2;
#endif
}
}

//////////////////////////////////////////////////////////////////////////

CPUProfiler::ScopeTiming::ScopeTiming(const char* counterName, CPUProfiler* _profiler)
{
    profiler = _profiler;
    if (profiler->started)
    {
        TimeCounter& c = profiler->counters->next();

        endTime = &c.endTime;
        c.startTime = CPUProfilerDetails::TimeStampUs();
        c.endTime = 0;
        c.name = counterName;
        c.tid = Thread::GetCurrentId();
    }
}

CPUProfiler::ScopeTiming::~ScopeTiming()
{
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
        const TimeCounter& c = *it;
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
    TimeCounter* lastDumpedCounter = nullptr;
    for (; it != itEnd; it++)
    {
        if (it->endTime != 0 && (strcmp(counterName, it->name) == 0))
        {
            if (lastDumpedCounter)
                stream << "=== Non-tracked time [" << (lastDumpedCounter->endTime - it->startTime) << " us] ===" << std::endl;
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
        if (it->endTime != 0 && (strcmp(counterName, it->name)))
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
    DVASSERT((snapshot != NO_SNAPSHOT_ID || !started) && "Stop profiler before dumping");

    CounterArray* array = GetCounterArray(snapshot);

    stream << "{ \"traceEvents\": [" << std::endl;

    CounterArray::iterator it = array->begin(), itEnd = array->end();
    CounterArray::iterator last(array->rbegin());
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

    Thread::Id threadID = begin->tid;
    uint64 endTime = begin->endTime;
    Vector<uint64> nodeEndTime;

    CounterTreeNode* node = new CounterTreeNode(nullptr, begin->name, begin->endTime - begin->startTime, 1);
    nodeEndTime.push_back(begin->endTime);

    CPUProfiler::CounterArray::iterator end = array->end();
    for (CPUProfiler::CounterArray::iterator it = begin + 1; it != end; ++it)
    {
        const CPUProfiler::TimeCounter& c = *it;

        if (c.tid != threadID)
            continue;

        if (c.startTime >= endTime || c.endTime == 0)
            break;

        if (c.tid == threadID)
        {
            while (nodeEndTime.size() && (c.startTime >= nodeEndTime.back()))
            {
                DVASSERT(node->parent);

                nodeEndTime.pop_back();
                node = node->parent;
            }

            if (CPUProfilerDetails::NameCmp(node->counterName, c.name))
            {
                node->counterTime += c.endTime - c.startTime;
                node->count++;
            }
            else
            {
                auto found = std::find_if(node->childs.begin(), node->childs.end(), [&c](CounterTreeNode* node) {
                    return (CPUProfilerDetails::NameCmp(c.name, node->counterName));
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
    if (CPUProfilerDetails::NameCmp(root->counterName, node->counterName))
    {
        root->counterTime += node->counterTime;
        root->count++;

        for (CounterTreeNode* c : node->childs)
            MergeTree(root, c);
    }
    else
    {
        auto found = std::find_if(root->childs.begin(), root->childs.end(), [&node](CounterTreeNode* child) {
            return (CPUProfilerDetails::NameCmp(child->counterName, node->counterName));
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
