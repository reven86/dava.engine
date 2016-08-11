#include "Profiler.h"
#include "Debug/DVAssert.h"
#include "Logger/Logger.h"
#include "Concurrency/Mutex.h"
#include "Platform/SystemTimer.h"
#include "FileSystem/File.h"

//==============================================================================

static inline uint64 CurTimeUs()
{
    return static_cast<DAVA::uint64>(DAVA::SystemTimer::Instance()->GetAbsoluteUs());
}

static bool NameEquals(const char* name1, const char* name2)
{
    if (name1 == name2)
        return true;

    std::size_t n1len = strlen(name1);
    std::size_t n2len = strlen(name2);

    if (n1len != n2len)
        return false;

    return memcmp(name1, name2, n1len) == 0;
}

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

using CounterArray = RingArray<TimeCounter, std::atomic<uint32>, 4096>;

static CounterArray counters;
static bool profilerStarted = false;

void Start()
{
    profilerStarted = true;
}

void Stop()
{
    profilerStarted = false;
}

ScopedCounter::ScopedCounter(const char* counterName)
{
    if (profilerStarted)
    {
        TimeCounter& c = counters.next();
        DVASSERT(((c.startTime == 0 && c.endTime == 0) || c.endTime != 0) && "Too few counters in array");

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
        DVASSERT(*endTime == 0 && "Profiler counter ended but not started");
        *endTime = CurTimeUs();
    }
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

void DumpInternal(CounterArray::iterator begin)
{
    Thread::Id threadID = begin->tid;
    uint64 endTime = begin->endTime;
    Vector<uint64> parents;

    char buf[1024];
    CounterArray::iterator end = counters.end();
    for (CounterArray::iterator it = begin; it != end; ++it)
    {
        const TimeCounter& c = *it;

        if (c.tid != threadID)
            continue;

        if (c.startTime > endTime || c.endTime == 0)
            break;

        if (c.tid == threadID)
        {
            while (parents.size() && (c.startTime >= parents.back()) && c.startTime != c.endTime)
                parents.pop_back();

            Snprintf(buf, countof(buf), "%*s%s", parents.size() * 2, "", c.name);
            Logger::Info("%s [%llu us]", buf, c.endTime - c.startTime);

            parents.push_back(c.endTime);
        }
    }
}

void DumpLast(const char* counterName, uint32 counterCount)
{
    CounterArray::reverse_iterator it = counters.rbegin(), itEnd = counters.rend();
    for (; it != itEnd; it++)
    {
        if (NameEquals(counterName, it->name))
        {
            DumpInternal(CounterArray::iterator(it));
            counterCount--;
        }

        if (counterCount == 0)
            break;
    }
}

void DumpAverage(const char* counterName, uint32 counterCount)
{
    CounterArray::reverse_iterator it = counters.rbegin(), itEnd = counters.rend();
    for (; it != itEnd; it++)
    {
        if (NameEquals(counterName, it->name))
            counterCount--;

        if (counterCount == 0)
            break;
    }

    //TODO
}

void Dump(const char* fileName)
{
    File* json = File::Create(fileName, File::CREATE | File::WRITE);
    json->WriteLine("{ \"traceEvents\": [ ");

    char buf[1024];
    CounterArray::iterator it = counters.begin(), itEnd = counters.end();
    CounterArray::iterator last(counters.rbegin());
    for (; it != itEnd; it++)
    {
        Snprintf(buf, 1024, "{ \"pid\":%u, \"tid\":%llu, \"ts\":%llu, \"ph\":\"%s\", \"cat\":\"%s\", \"name\":\"%s\" }%s",
                 0, uint64(it->tid), it->startTime, "B", "", it->name, ", ");

        json->WriteLine(buf);

        Snprintf(buf, 1024, "{ \"pid\":%u, \"tid\":%llu, \"ts\":%llu, \"ph\":\"%s\", \"cat\":\"%s\", \"name\":\"%s\" }%s",
                 0, uint64(it->tid), it->endTime ? it->endTime : it->startTime, "E", "", it->name, (it == last) ? "" : ", ");

        json->WriteLine(buf);
    }

    json->WriteLine("] }");
    json->Release();
}

#if 0 
void LastChildLeaf(CounterArray::iterator begin, CounterArray::iterator& lastChild, CounterArray::iterator& lastLeaf)
{
    Thread::Id threadID = begin->tid;
    Vector<CounterArray::iterator> parents;

    uint64 endTime = begin->endTime;
    CounterArray::iterator end = counters.end();
    for (CounterArray::iterator it = begin; it != end; it++)
    {
        const TimeCounter& c = *it;
        if (c.tid == threadID)
        {
            if (c.endTime == 0)
                break;

            if (c.startTime <= endTime)
                lastLeaf = it;
            else
                break;

            while (parents.size() && (c.startTime >= parents.back()->endTime) && c.startTime != c.endTime)
                parents.pop_back();

            parents.push_back(it);
        }
    }

    DVASSERT(parents.size());

    if (parents.size() > 1)
        lastChild = parents[1];
    else
        lastChild = begin;
}

void DumpInternal2(CounterArray::iterator begin)
{
    struct CounterNote
    {
        CounterNote(TimeCounter* _counter, CounterNote* _parent)
            : counter(_counter)
            , parent(_parent)
        {}
        ~CounterNote()
        {
            for (CounterNote* c : children)
                SafeDelete(c);
        }

        TimeCounter* counter = nullptr;
        CounterNote* parent = nullptr;
        Vector<CounterNote*> children;
        uint64 dumpMask = 0;
    };

    static uint64 lastLeafMask = 1 << 63;
    uint32 treeDepth = 0;

    CounterNote* root = new CounterNote(nullptr, nullptr);
    CounterNote* current = root;

    Thread::Id threadID = begin->tid;
    uint64 endTime = begin->endTime;

    CounterArray::iterator end = counters.end();
    for (CounterArray::iterator it = begin; it != end; ++it)
    {
        TimeCounter* c = &(*it);
        if (c->tid != threadID)
            continue;

        if (c->startTime > endTime || c->endTime == 0)
            break;

        while (current->counter && (c->startTime >= current->counter->endTime) && c->startTime != c->endTime)
        {
            current = current->parent;
        }

        current->children.push_back(new CounterNote(c, current));
        current = current->children.back();
        ++treeDepth;
    }



    SafeDelete(root);
}


void Dump()
{
    Set<Thread::Id> threadsToProcess;
    Set<Thread::Id> threadsProcessed;

    if ((*counters.rbegin()).tid)
        threadsToProcess.insert((*counters.rbegin()).tid);

    while (threadsToProcess.size())
    {
        Thread::Id threadID = *(threadsToProcess.cbegin());

        Logger::Info("Thread %lld =================================", uint64(threadID));

        Stack<const TimeCounter*> parentStack;
        CounterArray::iterator it = counters.begin(), itEnd = counters.end();
        for (; it != itEnd; it++)
        {
            const TimeCounter& c = *it;

            if (threadsProcessed.find(c.tid) != threadsProcessed.end())
                continue;

            if (c.endTime == 0)
                continue;

            if (c.tid == threadID)
            {
                while (parentStack.size() && (c.startTime >= parentStack.top()->endTime) && c.startTime != c.endTime)
                    parentStack.pop();

                Logger::Info("%*s%s    %llu us", parentStack.size(), "", c.name, c.endTime - c.startTime);

                parentStack.push(&c);
            }
            else
            {
                threadsToProcess.insert(c.tid);
            }
        }

        threadsToProcess.erase(threadID);
        threadsProcessed.insert(threadID);
    }
}
#endif

} //ns Profiler

} //ns DAVA

#if 0
namespace profiler
{
//==============================================================================

class Counter;

static Counter* GetCounter(uint32 id);

static bool profilerInited = false;
static uint32 maxCounterCount = 0;
static uint32 historyCount = 0;

static Counter* profCounter = 0;
static Counter* profAverage = 0;
static Counter* curCounter = 0;
static bool profStarted = false;
static Counter** activeCounter = 0;
static uint32 activeCounterCount = 0;
static uint64 totalTime0 = 0;
static uint64 totalTime = 0;
static DAVA::Spinlock counterSync;

class
Counter
{
public:
    Counter()
        : t0(0)
        , t(0)
        , count(0)
        , id(0)
        , parentId(DAVA::InvalidIndex)
        , name(0)
        , used(false)
        , useCount(0)
    {
    }

    void SetName(const char* n)
    {
        name = n;
    }

    void Reset()
    {
        counterSync.Lock();

        t = 0;
        count = 0;
        used = false;
        useCount = 0;
        parentId = DAVA::InvalidIndex;

        counterSync.Unlock();
    }
    void Start()
    {
        counterSync.Lock();

        if (!useCount)
        {
            if (activeCounterCount)
                parentId = activeCounter[activeCounterCount - 1]->id;
            else
                parentId = DAVA::InvalidIndex;

            activeCounter[activeCounterCount] = this;
            ++activeCounterCount;

            t0 = CurTimeUs();
        }

        used = true;
        ++count;
        ++useCount;

        counterSync.Unlock();
    }
    void Stop()
    {
        counterSync.Lock();

        if (useCount == 1)
        {
            if (activeCounterCount)
                --activeCounterCount;
        }

        if (--useCount == 0)
            t += CurTimeUs() - t0;

        counterSync.Unlock();
    }

    const char* GetName() const
    {
        return name;
    }

    uint32 GetCount() const
    {
        return count;
    }
    uint64 GetTimeUs() const
    {
        return t;
    }

    uint32 GetId() const
    {
        return id;
    }
    uint32 GetParentId() const
    {
        return parentId;
    }
    bool IsUsed() const
    {
        return (used) ? true : false;
    }
    uint32 NestingLevel() const
    {
        return (parentId != DAVA::InvalidIndex) ? GetCounter(parentId)->NestingLevel() + 1 : 0;
    }

private:
    friend void Init(uint32, uint32);
    friend void Start();
    friend void Stop();
    friend bool DumpAverage();
    friend bool GetAverageCounters(std::vector<CounterInfo>*);

private:
    uint64 t0;
    uint64 t;
    uint32 count;

    uint32 id;
    uint32 parentId;
    const char* name; // ref-only, expected to be immutable string

    uint32 used : 1;
    uint32 useCount : 4;
};

//==============================================================================

static Counter*
GetCounter(uint32 id)
{
    return (id < maxCounterCount) ? curCounter + id : 0;
}

//------------------------------------------------------------------------------

void
Init(uint32 _maxCounterCount, uint32 _historyCount)
{
    DVASSERT(!profilerInited);

    historyCount = _historyCount;
    maxCounterCount = _maxCounterCount;

    profCounter = new Counter[maxCounterCount * historyCount];
    profAverage = new Counter[maxCounterCount];
    activeCounter = new Counter*[maxCounterCount];

    Counter* counter = profCounter;

    for (uint32 h = 0; h != historyCount; ++h)
    {
        for (Counter *c = counter, *c_end = counter + maxCounterCount; c != c_end; ++c)
        {
            c->id = static_cast<uint32>(c - counter);
            c->parentId = DAVA::InvalidIndex;
            c->used = false;
        }

        counter += maxCounterCount;
    }

    profilerInited = true;
}

//------------------------------------------------------------------------------

void
EnsureInited(uint32 _maxCounterCount, uint32 _historyCount)
{
    if (!profilerInited)
    {
        Init(_maxCounterCount, _historyCount);
    }
}

//------------------------------------------------------------------------------

void
Uninit()
{
    if (profCounter)
    {
        delete[] profCounter;
        profCounter = 0;
    }

    if (profAverage)
    {
        delete[] profAverage;
        profAverage = 0;
    }

    if (activeCounter)
    {
        delete[] activeCounter;
        activeCounter = 0;
    }

    historyCount = 0;
    maxCounterCount = 0;

    profilerInited = false;
}

//------------------------------------------------------------------------------

void
SetCounterName(unsigned counterId, const char* name)
{
    DVASSERT(counterId < maxCounterCount);

    Counter* counter = profCounter + counterId;

    for (uint32 h = 0; h != historyCount; ++h)
    {
        counter->SetName(name);
        counter += maxCounterCount;
    }
}

//------------------------------------------------------------------------------

void
StartCounter(uint32 counterId)
{
    DVASSERT(counterId < maxCounterCount);

    Counter* counter = curCounter + counterId;

    counter->Start();
}

//------------------------------------------------------------------------------

void
StartCounter(uint32 counterId, const char* counterName)
{
    DVASSERT(counterId < maxCounterCount);

    if (curCounter)
    {
        Counter* counter = curCounter + counterId;

        counter->SetName(counterName);
        counter->Start();
    }
}

//------------------------------------------------------------------------------

void
StopCounter(uint32 counterId)
{
    DVASSERT(counterId < maxCounterCount);

    if (curCounter)
    {
        Counter* counter = curCounter + counterId;

        counter->Stop();
    }
}

//------------------------------------------------------------------------------

void
Start()
{
    DVASSERT(!profStarted);

    if (curCounter)
    {
        curCounter += maxCounterCount;
        if (curCounter >= profCounter + maxCounterCount * historyCount)
        {
            curCounter = profCounter;
        }
    }
    else
    {
        curCounter = profCounter;
    }

    for (Counter *c = curCounter, *c_end = curCounter + maxCounterCount; c != c_end; ++c)
    {
        c->Reset();
        c->used = false;
    }

    profStarted = true;
    activeCounterCount = 0;
    totalTime0 = CurTimeUs();
}

//------------------------------------------------------------------------------

void
Stop()
{
    DVASSERT(profStarted);

    totalTime = CurTimeUs() - totalTime0;
    profStarted = false;
}

//------------------------------------------------------------------------------

static inline int
fltDec(float f)
{
    return int((f - float(int(f))) * 10.0f);
}

static void
DumpInternal(const std::vector<CounterInfo>& result, bool showPercents = false)
{
    size_t max_name_len = 0;

    for (size_t i = 0, i_end = result.size(); i != i_end; ++i)
    {
        uint32 pi = result[i].parentIndex;
        uint32 indent = 0;
        size_t len = 0;

        while (pi != DAVA::InvalidIndex)
        {
            pi = result[pi].parentIndex;
            ++indent;
        }

        if (result[i].name)
            len += strlen(result[i].name);

        len += indent * 2;

        if (len > max_name_len)
            max_name_len = len;
    }

    Logger::Info("===================================================");
    for (size_t i = 0, i_end = result.size(); i != i_end; ++i)
    {
        uint32 pi = result[i].parentIndex;
        uint32 indent = 0;
        char text[256];
        memset(text, ' ', sizeof(text));

        while (pi != DAVA::InvalidIndex)
        {
            pi = result[pi].parentIndex;
            ++indent;
        }

        size_t text_len = 0;

        if (result[i].name)
            text_len = Snprintf(text + indent * 2, sizeof(text) - indent * 2, "%s", result[i].name);
        else
            text_len = Snprintf(text + indent * 2, sizeof(text) - indent * 2, "%u", static_cast<uint32>(i));

        text[indent * 2 + text_len] = ' ';
        text_len = max_name_len + 2 + Snprintf(text + max_name_len + 2, sizeof(text) - max_name_len - 2, " %-5u  %llu us", result[i].count, result[i].timeUs);

        if (showPercents)
        {
            float pg = (totalTime)
            ?
            100.0f * float(result[i].timeUs) / float(totalTime)
            :
            0;
            const CounterInfo* pc = (result[i].parentIndex != DAVA::InvalidIndex) ? &(result[0]) + result[i].parentIndex : 0;
            float pl = (pc && pc->timeUs)
            ?
            100.0f * float(result[i].timeUs) / float(pc->timeUs)
            :
            0;

            text[text_len] = ' ';
            text_len = max_name_len + 2 + 1 + 5 + 2 + 5 + 1 + 2;

            if (pc)
                text_len += Snprintf(text + text_len, sizeof(text) - text_len, "   %02i.%i    %02i.%i", int(pl), fltDec(pl), int(pg), fltDec(pg));
            else
                text_len += Snprintf(text + text_len, sizeof(text) - text_len, "   %02i.%i    %02i.%i", int(pg), fltDec(pg), int(pg), fltDec(pg));
        }

        Logger::Info(text);
    }
    Logger::Info("\n");
}

//------------------------------------------------------------------------------

void
Dump()
{
    DVASSERT(!profStarted);

    static std::vector<CounterInfo> result;

    GetCounters(&result);
    DumpInternal(result, true);
}

//------------------------------------------------------------------------------

bool
DumpAverage()
{
    bool success = false;
    static std::vector<CounterInfo> result;

    if (GetAverageCounters(&result))
    {
        DumpInternal(result, false);
        success = true;
    }

    return success;
}

//------------------------------------------------------------------------------

static void
CollectCountersWithChilds(const Counter* base, const Counter* counter, std::vector<const Counter*>* result)
{
    for (const Counter *c = base, *c_end = base + maxCounterCount; c != c_end; ++c)
    {
        if (c->IsUsed() && c->GetParentId() == counter->GetId())
        {
            result->push_back(c);
            CollectCountersWithChilds(base, c, result);
        }
    }
}

//------------------------------------------------------------------------------

static void
CollectActiveCounters(Counter* cur_counter, std::vector<const Counter*>* result)
{
    // get top-level counters, sorted by time

    static std::vector<const Counter*> top;

    top.clear();
    for (Counter *c = cur_counter, *c_end = cur_counter + maxCounterCount; c != c_end; ++c)
    {
        if (!c->IsUsed())
            continue;

        bool do_add = true;

        if (c->GetParentId() == DAVA::InvalidIndex)
        {
            for (size_t i = 0, i_end = top.size(); i != i_end; ++i)
            {
                if (c->GetTimeUs() > top[i]->GetTimeUs())
                {
                    top.insert(top.begin() + i, c);
                    do_add = false;
                    break;
                }
            }

            if (do_add)
                top.push_back(c);
        }
    }

    // fill active-counter list

    result->clear();
    for (uint32 i = 0; i != top.size(); ++i)
    {
        result->push_back(top[i]);
        CollectCountersWithChilds(cur_counter, top[i], result);
    }
}

//------------------------------------------------------------------------------

void
GetCounters(std::vector<CounterInfo>* info)
{
    static std::vector<const Counter*> result;

    CollectActiveCounters(curCounter, &result);

    info->resize(result.size());
    for (size_t i = 0, i_end = result.size(); i != i_end; ++i)
    {
        (*info)[i].name = result[i]->GetName();
        (*info)[i].count = result[i]->GetCount();
        (*info)[i].timeUs = result[i]->GetTimeUs();
        (*info)[i].parentIndex = DAVA::InvalidIndex;

        for (size_t k = 0, k_end = info->size(); k != k_end; ++k)
        {
            if (result[i]->GetParentId() == result[k]->GetId())
            {
                (*info)[i].parentIndex = static_cast<uint32>(k);
                break;
            }
        }
    }
}

//------------------------------------------------------------------------------

bool
GetAverageCounters(std::vector<CounterInfo>* info)
{
    bool success = false;

    if (curCounter == profCounter + maxCounterCount * (historyCount - 1))
    {
        for (Counter *c = profAverage, *c_end = profAverage + maxCounterCount; c != c_end; ++c)
        {
            Counter* src = profCounter + (c - profAverage);

            c->Reset();
            c->SetName(src->GetName());

            c->id = src->id;
            c->parentId = src->parentId;
            c->t0 = 0;
            c->t = 0;
            c->used = src->used;
        }

        for (uint32 h = 0; h != historyCount; ++h)
        {
            Counter* counter = profCounter + h * maxCounterCount;

            for (Counter *c = counter, *c_end = counter + maxCounterCount, *a = profAverage; c != c_end; ++c, ++a)
            {
                a->count += c->count;
                a->t0 = 0;
                a->t += c->GetTimeUs();
            }
        }

        for (Counter *c = profAverage, *c_end = profAverage + maxCounterCount; c != c_end; ++c)
        {
            c->count /= historyCount;
            c->t /= historyCount;
        }

        static std::vector<const Counter*> result;

        CollectActiveCounters(profAverage, &result);

        info->resize(result.size());
        for (size_t i = 0, i_end = result.size(); i != i_end; ++i)
        {
            (*info)[i].name = result[i]->GetName();
            (*info)[i].count = result[i]->GetCount();
            (*info)[i].timeUs = result[i]->GetTimeUs();
            (*info)[i].parentIndex = DAVA::InvalidIndex;

            for (size_t k = 0, k_end = info->size(); k != k_end; ++k)
            {
                if (result[i]->GetParentId() == result[k]->GetId())
                {
                    (*info)[i].parentIndex = static_cast<uint32>(k);
                    break;
                }
            }
        }

        success = true;
    }

    return success;
}

//------------------------------------------------------------------------------

struct
Event
{
    enum Phase
    {
        phaseBegin = 1,
        phaseEnd = 2,
        phaseInstant = 3
    };

    uint16 pid;
    uint16 tid;
    uint64 time;
    const char* category;
    const char* name;
    Phase phase;
};

static std::vector<Event> _Event;
static DAVA::Mutex _EventSync;
static bool traceEventsStarted = false;

void StartTraceEvents()
{
#if defined TRACER_ENABLED
    traceEventsStarted = true;
    _Event.reserve(2 * 1024 * 1024);
#endif
}

void StopTraceEvents()
{
#if defined TRACER_ENABLED
    traceEventsStarted = false;
#endif
}

void BeginEvent(unsigned tid, const char* category, const char* name)
{
    if (!traceEventsStarted)
        return;

    Event evt;

    evt.pid = 1;
    evt.tid = tid;
    evt.time = CurTimeUs();
    evt.category = category;
    evt.name = name;
    evt.phase = Event::phaseBegin;

    _EventSync.Lock();
    _Event.push_back(evt);
    _EventSync.Unlock();
}

//------------------------------------------------------------------------------

void EndEvent(unsigned tid, const char* category, const char* name)
{
    if (!traceEventsStarted)
        return;

    Event evt;

    evt.pid = 1;
    evt.tid = tid;
    evt.time = CurTimeUs();
    evt.category = category;
    evt.name = name;
    evt.phase = Event::phaseEnd;

    _EventSync.Lock();
    _Event.push_back(evt);
    _EventSync.Unlock();
}

//------------------------------------------------------------------------------

void InstantEvent(unsigned tid, const char* category, const char* name)
{
    if (!traceEventsStarted)
        return;

    Event evt;

    evt.pid = 1;
    evt.tid = tid;
    evt.time = CurTimeUs();
    evt.category = category;
    evt.name = name;
    evt.phase = Event::phaseInstant;

    _EventSync.Lock();
    _Event.push_back(evt);
    _EventSync.Unlock();
}

//------------------------------------------------------------------------------

void DumpEvents()
{
    Logger::Info("{ \"traceEvents\": [ ");
    for (std::vector<Event>::const_iterator e = _Event.begin(), e_end = _Event.end(); e != e_end; ++e)
    {
        const char* ph = "";

        switch (e->phase)
        {
        case Event::phaseBegin:
            ph = "B";
            break;
        case Event::phaseEnd:
            ph = "E";
            break;
        case Event::phaseInstant:
            ph = "I";
            break;
        }
        Logger::Info("{ \"pid\":%u, \"tid\":%u, \"ts\":%lu, \"ph\":\"%s\", \"cat\":\"%s\", \"name\":\"%s\" }%s",
                     unsigned(e->pid),
                     unsigned(e->tid),
                     long(e->time),
                     ph,
                     e->category,
                     e->name,
                     (e != _Event.end() - 1) ? ", " : "");
    }
    Logger::Info("] }");
}

//------------------------------------------------------------------------------

void SaveEvents(const char* fileName)
{
    File* json = File::Create(fileName, File::CREATE | File::WRITE);
    json->WriteLine("{ \"traceEvents\": [ ");

    _EventSync.Lock();
    for (std::vector<Event>::const_iterator e = _Event.begin(), e_end = _Event.end(); e != e_end; ++e)
    {
        char buf[1024];
        const char* ph = "";

        switch (e->phase)
        {
        case Event::phaseBegin:
            ph = "B";
            break;
        case Event::phaseEnd:
            ph = "E";
            break;
        case Event::phaseInstant:
            ph = "I";
            break;
        }
        Snprintf(buf, 1024,
                 "{ \"pid\":%u, \"tid\":%u, \"ts\":%lu, \"ph\":\"%s\", \"cat\":\"%s\", \"name\":\"%s\" }%s",
                 unsigned(e->pid),
                 unsigned(e->tid),
                 long(e->time),
                 ph,
                 e->category,
                 e->name,
                 (e != _Event.end() - 1) ? ", " : "");
        json->WriteLine(buf);
    }
    _EventSync.Unlock();

    json->WriteLine("] }");
    json->Release();
}

//==============================================================================
} // namespace profiler

#endif