#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include <iosfwd>

namespace DAVA
{
struct TraceEvent
{
    enum EventPhase
    {
        PHASE_BEGIN = 0,
        PHASE_END,
        PHASE_INSTANCE,
        PHASE_DURATION,

        PHASE_COUNT
    };

    FastName name;
    uint64 timestamp;
    uint64 duration;
    uint64 threadID;
    uint32 processID;
    EventPhase phase;
    Vector<std::pair<FastName, uint32>> args;

    template <template <typename, typename> class Container, class TAlloc>
    static void DumpJSON(const Container<TraceEvent, TAlloc>& trace, std::ostream& stream);
};

template <template <typename, typename> class Container, class TAlloc>
void TraceEvent::DumpJSON(const Container<TraceEvent, TAlloc>& trace, std::ostream& stream)
{
    static const char* const PHASE_STR[PHASE_COUNT] = {
        "B", "E", "I", "X"
    };

    stream << "{ \"traceEvents\": [" << std::endl;

    auto begin = trace.begin(), end = trace.end();
    for (auto it = begin; it != end; ++it)
    {
        const TraceEvent& event = (*it);

        if (it != begin)
            stream << "," << std::endl;

        stream << "{ ";
        stream << "\"pid\": " << event.processID << ", ";
        stream << "\"tid\": " << event.threadID << ", ";
        stream << "\"ts\": " << event.timestamp << ", ";

        if (event.phase == PHASE_DURATION)
            stream << "\"dur\": " << event.duration << ", ";

        stream << "\"ph\": \"" << PHASE_STR[event.phase] << "\", ";
        stream << "\"name\": \"" << event.name.c_str() << "\"";

        for (const std::pair<FastName, uint32>& arg : event.args)
            stream << ", \"args\": { \"" << arg.first.c_str() << "\": " << arg.second << " }";

        stream << " }";
    }

    stream << std::endl
           << "] }" << std::endl;

    stream.flush();
}

}; //ns DAVA