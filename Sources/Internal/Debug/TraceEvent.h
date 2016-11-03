#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include <iosfwd>

namespace DAVA
{
/**
    \brief  Structure that represent trace data in format of Chromium Trace Viewer.
            Also it allows to dump set of events to JSON-format, that can be opened by Trace Viewer.
*/
struct TraceEvent
{
    //! Valid values of event type
    enum EventPhase
    {
        PHASE_BEGIN = 0, ///< Begin of duration event. Must come before the corresponding end event. It's no use 'duration' field
        PHASE_END, ///< End of duration event. It's no use 'duration' field
        PHASE_INSTANCE, ///< The instance event-type. Correspond to something that happens buy has no duration. It's no use 'duration' field
        PHASE_DURATION, ///< Complete event. Logically combines a pair of 'Begin' and 'End' events. Preferably to use this event type instead Begin/End because it reduce the size of the trace.

        PHASE_COUNT ///< Count of implemented event types.
    };

    FastName name; ///< The name of the event, as displayed in Trace Viewer.
    uint64 timestamp; ///< The tracing timestamp of the event.
    uint64 duration; ///< The tracing duration if the event.
    uint64 threadID; ///< The thread ID for the thread that generate this event.
    uint32 processID; ///< The process ID for the process that generate this event.
    EventPhase phase; ///< The event type. The valid values are listed in enum description.
    Vector<std::pair<FastName, uint32>> args; ///< Any arguments provided for the event. Used as 'meta-info'. The arguments are displayed in Trace Viewer.

    /**
        Dump trace events from any type container to stream in JSON-format.
        \param[in] trace Container of events
        \param[out] stream Stream which output will be written
    */
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