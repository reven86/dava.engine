#include "Profiler.h"
#include "Debug/DVAssert.h"
#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"
#include "FileSystem/File.h"
#include "Concurrency/Thread.h"
#include "Base/RingArray.h"
#include "Base/AllocatorFactory.h"

//==============================================================================

const static uint32 TIME_COUNTERS_COUNT = 2048;

namespace DAVA
{
namespace Profiler
{
struct TimeCounter
{
    uint64 startTime = 0;
    uint64 endTime = 0;
    const char* name = nullptr;
    Thread::Id tid = 0;
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
    static void DumpTree(const CounterTreeNode* node, int32 treeDepth, File* file, bool average);
    static void SafeDeleteTree(CounterTreeNode*& node);

protected:
    uint64 counterTime = 0;
    const char* counterName;

    CounterTreeNode* parent;
    Vector<CounterTreeNode*> childs;
    uint32 count; //recursive and duplicate counters
};

uint64 TimeStampUs();
bool NameEquals(const char* name1, const char* name2);
CounterArray& GetCounterArray(int32 snapshot);
void DumpLine(const char* buf, File* file);

//////////////////////////////////////////////////////////////////////////

ScopedCounter::ScopedCounter(const char* counterName)
{
    if (profilerStarted)
    {
        TimeCounter& c = counters.next();

        endTime = &c.endTime;
        c.startTime = TimeStampUs();
        c.endTime = 0;
        c.name = counterName;
        c.tid = Thread::GetCurrentId();
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
    uint64 timeDelta = 0;
    CounterArray::reverse_iterator it = counters.rbegin(), itEnd = counters.rend();
    for (; it != itEnd; it++)
    {
        const TimeCounter& c = *it;
        if (c.endTime != 0 && NameEquals(counterName, c.name))
        {
            timeDelta = c.endTime - c.startTime;
            break;
        }
    }

    return timeDelta;
}

void DumpLast(const char* counterName, uint32 counterCount, File* file, int32 snapshot)
{
    DVASSERT((snapshot != NO_SNAPSHOT_ID || !profilerStarted) && "Stop profiler before dumping");

    DumpLine("================================================================", file);

    CounterArray& array = GetCounterArray(snapshot);
    CounterArray::reverse_iterator it = array.rbegin(), itEnd = array.rend();
    TimeCounter* lastDumpedCounter = nullptr;
    for (; it != itEnd; it++)
    {
        if (it->endTime != 0 && NameEquals(counterName, it->name))
        {
            if (lastDumpedCounter)
                DumpLine(DAVA::Format("=== Non-tracked time [%llu us] ===", lastDumpedCounter->endTime - it->startTime).c_str(), file);
            lastDumpedCounter = &(*it);

            CounterTreeNode* treeRoot = CounterTreeNode::BuildTree(CounterArray::iterator(it), array);
            CounterTreeNode::DumpTree(treeRoot, 0, file, false);
            SafeDelete(treeRoot);

            counterCount--;
        }

        if (counterCount == 0)
            break;
    }

    DumpLine("================================================================", file);
}

void DumpAverage(const char* counterName, uint32 counterCount, File* file, int32 snapshot)
{
    DVASSERT((snapshot != NO_SNAPSHOT_ID || !profilerStarted) && "Stop profiler before dumping");

    DumpLine("================================================================", file);
    DumpLine(DAVA::Format("=== Average time for %d counter(s):", counterCount).c_str(), file);

    CounterArray& array = GetCounterArray(snapshot);
    CounterArray::reverse_iterator it = array.rbegin(), itEnd = array.rend();
    CounterTreeNode* treeRoot = nullptr;
    for (; it != itEnd; it++)
    {
        if (it->endTime != 0 && NameEquals(counterName, it->name))
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

    CounterTreeNode::DumpTree(treeRoot, 0, file, true);
    CounterTreeNode::SafeDeleteTree(treeRoot);

    DumpLine("================================================================", file);
}

void DumpJSON(File* file, int32 snapshot)
{
    DVASSERT((snapshot != NO_SNAPSHOT_ID || !profilerStarted) && "Stop profiler before dumping");
    DVASSERT(file);

    CounterArray& array = GetCounterArray(snapshot);

    file->WriteLine("{ \"traceEvents\": [ ");

    char strbuf[1024];
    CounterArray::iterator it = array.begin(), itEnd = array.end();
    CounterArray::iterator last(array.rbegin());
    for (; it != itEnd; it++)
    {
        Snprintf(strbuf, sizeof(strbuf), "{ \"pid\":%u, \"tid\":%llu, \"ts\":%llu, \"ph\":\"%s\", \"cat\":\"%s\", \"name\":\"%s\" }%s",
                 0, uint64(it->tid), it->startTime, "B", "", it->name, ", ");

        file->WriteLine(strbuf);

        Snprintf(strbuf, sizeof(strbuf), "{ \"pid\":%u, \"tid\":%llu, \"ts\":%llu, \"ph\":\"%s\", \"cat\":\"%s\", \"name\":\"%s\" }%s",
                 0, uint64(it->tid), it->endTime ? it->endTime : it->startTime, "E", "", it->name, (it == last) ? "" : ", ");

        file->WriteLine(strbuf);
    }

    file->WriteLine("] }");
}

/////////////////////////////////////////////////////////////////////////////////
//Internal Definition

CounterTreeNode* CounterTreeNode::BuildTree(CounterArray::iterator begin, CounterArray& array)
{
    DVASSERT(begin->endTime);

    Thread::Id threadID = begin->tid;
    uint64 endTime = begin->endTime;
    Vector<uint64> nodeEndTime;

    CounterTreeNode* node = new CounterTreeNode();
    node->parent = nullptr;
    node->counterName = begin->name;
    node->counterTime = begin->endTime - begin->startTime;
    node->count = 1;
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

            if (NameEquals(node->counterName, c.name))
            {
                node->counterTime += c.endTime - c.startTime;
                node->count++;
            }
            else
            {
                auto found = std::find_if(node->childs.begin(), node->childs.end(), [&c](CounterTreeNode* node) {
                    return NameEquals(c.name, node->counterName);
                });

                if (found != node->childs.end())
                {
                    (*found)->counterTime += c.endTime - c.startTime;
                    (*found)->count++;
                }
                else
                {
                    CounterTreeNode* child = new CounterTreeNode();
                    child->parent = node;
                    child->counterName = c.name;
                    child->counterTime = c.endTime - c.startTime;
                    child->count = 1;

                    node->childs.push_back(child);
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
    if (NameEquals(root->counterName, node->counterName))
    {
        root->counterTime += node->counterTime;
        root->count++;

        for (CounterTreeNode* c : node->childs)
            MergeTree(root, c);
    }
    else
    {
        auto found = std::find_if(root->childs.begin(), root->childs.end(), [&node](CounterTreeNode* child) {
            return NameEquals(child->counterName, node->counterName);
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
    CounterTreeNode* nodeCopy = new CounterTreeNode();
    nodeCopy->parent = nullptr;
    nodeCopy->counterName = node->counterName;
    nodeCopy->counterTime = node->counterTime;
    nodeCopy->count = node->count;

    for (CounterTreeNode* c : node->childs)
    {
        nodeCopy->childs.push_back(CopyTree(c));
        nodeCopy->childs.back()->parent = nodeCopy;
    }

    return nodeCopy;
}

void CounterTreeNode::DumpTree(const CounterTreeNode* node, int32 treeDepth, File* file, bool average)
{
    char strbuf[1024];
    Snprintf(strbuf, sizeof(strbuf), "%*s%s [%llu us | x%d]", treeDepth * 2, "", node->counterName, (average ? node->counterTime / node->count : node->counterTime), node->count);
    DumpLine(strbuf, file);

    for (CounterTreeNode* c : node->childs)
        DumpTree(c, treeDepth + 1, file, average);
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

bool NameEquals(const char* name1, const char* name2)
{
    if (name1 == name2)
        return true;

    return strcmp(name1, name2) == 0;
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

void DumpLine(const char* strbuf, File* file)
{
    if (file)
    {
        file->WriteLine(strbuf);
    }
    else
    {
        Logger::Info(strbuf);
    }
}

} //ns Profiler
} //ns DAVA
