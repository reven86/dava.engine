/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#if !defined __PROFILER_H__
#define __PROFILER_H__

    #include <Base/BaseTypes.h>
    using DAVA::uint32;
    using DAVA::uint64;
    #include <Base/Hash.h>

    #define PROFILER_ENABLED        0
    #define TRACER_ENABLED          0


namespace profiler
{

/**


Init the profiler.
Should be called once, on app startup.

@max_counter_count max amount of counters that will be available
@history_length amount of previous counter-values to keep (used for computing average values)
*/

void    EnsureInited( uint32 max_counter_count=64, uint32 history_length=128 );
void    Init( uint32 max_counter_count=64, uint32 history_length=128 );

void    Uninit();


void    Start(); 
void    Stop();

void    Dump();

/**

Dump average values from history.
Note, that average-values can't be dumped at any time, 
only when Profiler has exactly 'history_length' measurements in history.

@return true if average-values successfully dumped, false if average-values can't be dumped at the moment.
*/
bool    DumpAverage(); 



// don't call these directly, use macros defined below
void    SetCounterName( uint32 counter_id, const char* counter_name );
void    StartCounter( uint32 counter_id );
void    StartCounter( uint32 counter_id, const char* counter_name );
void    StopCounter( uint32 counter_id );



//==============================================================================

struct
CounterInfo
{
    const char* name;
    uint64      timeUs;
    uint32      count;
    uint32      parentIndex; // in vector<CounterInfo>
};


void    GetCounters( std::vector<CounterInfo>* info );
bool    GetAverageCounters( std::vector<CounterInfo>* info );

//==============================================================================

struct
ScopedTiming
{
    ScopedTiming( int id )                      : _id(id)   { profiler::StartCounter(id); }
    ScopedTiming( int id, const char* name )    : _id(id)   { profiler::StartCounter(id,name); }
    ~ScopedTiming()                                         { profiler::StopCounter(_id); }
    int _id;
};


//==============================================================================
} // namespace profiler



#if PROFILER_ENABLED

// utils, used by actual timing macros
#define PROF_FUNCTION_ID(name_ptr)      (((int(name_ptr))>>4)&(64-1))
#define PROF_STRING_ID(str)             (((DV_HASH(str))>>4)&(64-1))


// name (regular) counters BEFORE using them
#define NAME_COUNTER(counter_id,name)   profiler::SetCounterName(counter_id,name);

// regular timing macros, minimal overhead
#define START_TIMING(counter_id)        profiler::StartCounter(counter_id);
#define STOP_TIMING(counter_id)         profiler::StopCounter(counter_id);

// arbitrary named timings, a bit slower
#define START_NAMED_TIMING(c_name)      profiler::StartCounter( PROF_STRING_ID(c_name), c_name );
#define STOP_NAMED_TIMING(c_name)       profiler::StopCounter( PROF_STRING_ID(c_name) );



// scoped timing, minimal overhead
#define SCOPED_TIMING(counter_id)           profiler::ScopedTiming st##counter_id(counter_id);

// named scoped timings, a bit slower
#define SCOPED_NAMED_TIMING(counter_name)   profiler::ScopedTiming st##counter_id( PROF_STRING_ID(counter_name), counter_name );
#define SCOPED_FUNCTION_TIMING()            profiler::ScopedTiming st_func(PROF_FUNCTION_ID(__FUNCTION__),__FUNCTION__);


#else

#define NAME_COUNTER(counter_id,name)       
#define START_TIMING(counter_id)            
#define STOP_TIMING(counter_id)             
#define START_NAMED_TIMING(c_name)          
#define STOP_NAMED_TIMING(c_name)           
#define SCOPED_TIMING(counter_id)           
#define SCOPED_NAMED_TIMING(counter_name)   
#define SCOPED_FUNCTION_TIMING()            

#endif // PROFILER_ENABLED


namespace profiler
{

void    BeginEvent( unsigned tid, const char* category, const char* name );
void    EndEvent( unsigned tid, const char* category, const char* name );
void    InstantEvent( unsigned tid, const char* category, const char* name );

void    DumpEvents();
void    SaveEvents( const char* fileName );

}

#if TRACER_ENABLED

#define TRACE_BEGIN_EVENT(tid,cat,name)     profiler::BeginEvent( tid, cat, name );
#define TRACE_END_EVENT(tid,cat,name)       profiler::EndEvent( tid, cat, name );
#define TRACE_INSTANT_EVENT(tid,cat,name)   profiler::InstantEvent( tid, cat, name );

#else

#define TRACE_BEGIN_EVENT(tid,cat,name) 
#define TRACE_END_EVENT(tid,cat,name)   
#define TRACE_INSTANT_EVENT(tid,cat,name)

#endif


#endif // __PROFILER_H__

