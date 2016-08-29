#include "Profiler.h"
#include "Debug/DVAssert.h"
#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"
#include "FileSystem/File.h"
#include "Concurrency/Thread.h"
#include "Base/RingArray.h"

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
//Internal

struct CounterTreeNode
{
    CounterTreeNode(CounterTreeNode* _parent, const char* _name, uint64 _time)
        : counterName(_name)
        , counterTime(_time)
        , parent(_parent)
        , count(1)
    {
    }

    ~CounterTreeNode()
    {
        for (CounterTreeNode* c : childs)
            SafeDelete(c);

        childs.clear();
    }

    const char* counterName = nullptr;
    uint64 counterTime = 0;

    CounterTreeNode* parent = nullptr;
    Vector<CounterTreeNode*> childs;
    uint32 count = 0; //recursive and duplicate counters
};

uint64 CurTimeUs();
bool NameEquals(const char* name1, const char* name2);
void DumpInternal(CounterArray::iterator begin, CounterArray& array, File* file);
CounterArray& GetSnapshotArray(int32 snapshot);
CounterTreeNode* BuildTree(CounterArray::iterator begin, CounterArray& array);

//////////////////////////////////////////////////////////////////////////

ScopedCounter::ScopedCounter(const char* counterName)
{
    if (profilerStarted)
    {
        TimeCounter& c = counters.next();

        endTime = &c.endTime;
        c.startTime = CurTimeUs();
        c.endTime = 0;
        c.name = counterName;
        c.tid = Thread::GetCurrentId();
    }
}

ScopedCounter::~ScopedCounter()
{
    if (profilerStarted && endTime)
    {
        *endTime = CurTimeUs();
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

    CounterArray& array = GetSnapshotArray(snapshot);
    CounterArray::reverse_iterator it = array.rbegin(), itEnd = array.rend();
    for (; it != itEnd; it++)
    {
        if (it->endTime != 0 && NameEquals(counterName, it->name))
        {
            DumpInternal(CounterArray::iterator(it), array, file);
            counterCount--;
        }

        if (counterCount == 0)
            break;
    }
}

void DumpAverage(const char* counterName, uint32 counterCount, File* file, int32 snapshot)
{
    DVASSERT((snapshot != NO_SNAPSHOT_ID || !profilerStarted) && "Stop profiler before dumping");

    //TODO
}

void DumpJSON(File* file, int32 snapshot)
{
    DVASSERT((snapshot != NO_SNAPSHOT_ID || !profilerStarted) && "Stop profiler before dumping");

    CounterArray& array = GetSnapshotArray(snapshot);

    file->WriteLine("{ \"traceEvents\": [ ");

    char buf[1024];
    CounterArray::iterator it = array.begin(), itEnd = array.end();
    CounterArray::iterator last(array.rbegin());
    for (; it != itEnd; it++)
    {
        Snprintf(buf, 1024, "{ \"pid\":%u, \"tid\":%llu, \"ts\":%llu, \"ph\":\"%s\", \"cat\":\"%s\", \"name\":\"%s\" }%s",
                 0, uint64(it->tid), it->startTime, "B", "", it->name, ", ");

        file->WriteLine(buf);

        Snprintf(buf, 1024, "{ \"pid\":%u, \"tid\":%llu, \"ts\":%llu, \"ph\":\"%s\", \"cat\":\"%s\", \"name\":\"%s\" }%s",
                 0, uint64(it->tid), it->endTime ? it->endTime : it->startTime, "E", "", it->name, (it == last) ? "" : ", ");

        file->WriteLine(buf);
    }

    file->WriteLine("] }");
}

/////////////////////////////////////////////////////////////////////////////////
//internal functions

uint64 CurTimeUs()
{
    return DAVA::SystemTimer::Instance()->GetAbsoluteUs();
}

bool NameEquals(const char* name1, const char* name2)
{
    if (name1 == name2)
        return true;

    return strcmp(name1, name2) == 0;
}

CounterArray& GetSnapshotArray(int32 snapshot)
{
    if (snapshot != NO_SNAPSHOT_ID)
    {
        DVASSERT(snapshot >= 0 && snapshot < int32(snapshots.size()));
        return snapshots[snapshot];
    }

    return counters;
}

void DumpCounter(const TimeCounter& counter, int32 indent, File* file)
{
    char buf[1024];
    Snprintf(buf, countof(buf), "%*s%s [%llu us]", indent, "", counter.name, counter.endTime - counter.startTime);
    if (file)
    {
        file->WriteLine(buf);
    }
    else
    {
        Logger::Info(buf);
    }
}

void DumpCounterNode(const CounterTreeNode* node, int32 treeDepth, File* file)
{
    char buf[1024];
    if (node->count == 1)
    {
        Snprintf(buf, countof(buf), "%*s%s [%llu us]", treeDepth * 2, "", node->counterName, node->counterTime);
    }
    else
    {
        Snprintf(buf, countof(buf), "%*s%s [%llu us | x%d]", treeDepth * 2, "", node->counterName, node->counterTime, node->count);
    }

    if (file)
    {
        file->WriteLine(buf);
    }
    else
    {
        Logger::Info(buf);
    }

    for (CounterTreeNode* c : node->childs)
        DumpCounterNode(c, treeDepth + 1, file);
}

void DumpInternal(CounterArray::iterator begin, CounterArray& array, File* file)
{
    CounterTreeNode* treeRoot = BuildTree(begin, array);
    DumpCounterNode(treeRoot, 0, file);
    SafeDelete(treeRoot);
}

CounterTreeNode* BuildTree(CounterArray::iterator begin, CounterArray& array)
{
    DVASSERT(begin->endTime);

    Thread::Id threadID = begin->tid;
    uint64 endTime = begin->endTime;
    Vector<uint64> nodeEndTime;

    CounterTreeNode* node = new CounterTreeNode(nullptr, begin->name, begin->endTime - begin->startTime);
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
            while (nodeEndTime.size() && (c.startTime >= nodeEndTime.back()) && c.startTime != c.endTime)
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
                    node->childs.push_back(new CounterTreeNode(node, c.name, c.endTime - c.startTime));
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

} //ns Profiler
} //ns DAVA
