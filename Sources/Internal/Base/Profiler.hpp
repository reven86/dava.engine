#if !defined __PROFILER_HPP__
#define __PROFILER_HPP__
//==============================================================================
//
//  Simple intrusive profiler
//
//==============================================================================
//
//  externals:

    #include <base/BaseTypes.h>
    using namespace DAVA;
    #include "Hash.hpp"


    #define PROF__FRAME             0
    #define PROF__FRAME_UPDATE      1
    #define PROF__FRAME_DRAW        2
    #define PROF__RHI_SETUNIFORM    10
    #define PROF__RHI_SETDYNPARAM   11
    #define PROF__RHI_SETMATPARAM   12
    #define PROF__GL_DIP            20
    #define PROF__GL_SET_PROG       21
    #define PROF__GL_SET_UNIFORM    22
    #define PROF__GL_SET_UNIFORM2   23

    #define PROF__TEST1             40

#define DV_MATERIAL_UNIFORM_CACHING 1
#define DV_SHADER_UNIFORM_CACHING   0

    #define PROF_ENABLED            1


namespace profiler
{
//==============================================================================
//
//  publics:
//

void    Init();

void    Start();
void    Stop();

void    SetCounterName( unsigned counter_id, const char* counter_name );
void    StartCounter( unsigned counter_id );
void    StartCounter( unsigned counter_id, const char* counter_name );
void    StopCounter( unsigned counter_id );

void    ScheduleDump();
void    Dump();
bool    DumpAverage();


//==============================================================================

struct
CounterInfo
{
    const char* name;
    unsigned    time_us;
    unsigned    count;
    unsigned    parent_i;
};


void    GetCounters( std::vector<CounterInfo>* info );
bool    GetAverageCounters( std::vector<CounterInfo>* info );


//==============================================================================

#if PROF_ENABLED

// utils, used by 'real' timing macros
#define PROF_FUNCTION_ID(name_ptr)      (((int(name_ptr))>>4)&(64-1))
#define PROF_STRING_ID(str)             (((L_HASH(str))>>4)&(64-1))


// name (regular) counters BEFORE using them
#define NAME_COUNTER(counter_id,name)   profiler::SetCounterName(counter_id,name);

// regular timing macros, minimal overhead
#define START_TIMING(counter_id)        profiler::StartCounter(counter_id);
#define STOP_TIMING(counter_id)         profiler::StopCounter(counter_id);

// arbitrary named timings, a bit slower
#define START_NAMED_TIMING(c_name)      profiler::StartCounter( PROF_STRING_ID(c_name), c_name );
#define STOP_NAMED_TIMING(c_name)       profiler::StopCounter( PROF_STRING_ID(c_name) );


struct
ScopedTiming
{
    ScopedTiming( int id )                      : _id(id)   { profiler::StartCounter(id); }
    ScopedTiming( int id, const char* name )    : _id(id)   { profiler::StartCounter(id,name); }
    ~ScopedTiming()                                         { profiler::StopCounter(_id); }
    int _id;
};

// scoped timing, minimal overhead
#define SCOPED_TIMING(counter_id)           profiler::ScopedTiming st##counter_id(counter_id);

// named scoped timings, a bit slower
#define SCOPED_NAMED_TIMING(counter_name)   profiler::ScopedTiming st##counter_id( PROF_STRING_ID(counter_name), counter_name );
#define PROF_ENTER_FUNCTION()               profiler::ScopedTiming st_func(PROF_FUNCTION_ID(__FUNCTION__),__FUNCTION__);


#else

#define NAME_COUNTER(counter_id,name)   
#define BEGIN_TIMING(counter_id)        
#define BEGIN_TIMING_EX(c_id,c_name)    
#define END_TIMING(counter_id)          
#define SCOPED_TIMING(counter_id)       
#define SCOPED_TIMING_EX(counter_name)  
#define PROF_ENTER_FUNCTION()           

#endif

//==============================================================================
} // namespace profiler
#endif // __PROFILER_HPP__

