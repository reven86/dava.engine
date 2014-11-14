//==============================================================================
//
//  Profiler implementation
//
//==============================================================================
//
//  externals:

    #include "Profiler.hpp"
    #include "FileSystem/Logger.h"

    #include <stdio.h>
    #include <string.h>


    #ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
        #include <windows.h>
    #elif __ANDROID_API__
        #include <time.h>
        #include <android/log.h>
    #endif

    #include <vector>


//==============================================================================

const unsigned  InvalidIndex        = (unsigned)(-1);




//==============================================================================

static long
_CurTimeUs()
{
    #ifdef _WIN32

    LARGE_INTEGER freq;
    LARGE_INTEGER t;

    ::QueryPerformanceFrequency( &freq );
    ::QueryPerformanceCounter( &t );
    
    return long(((t.QuadPart)*1000000) / freq.QuadPart);

    #elif defined __ANDROID_API__

    timespec    ts;

    clock_gettime( CLOCK_REALTIME, &ts );
// this gives more correct time, but slow-as-Hell on lots of devices
//   clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts );

    return long(ts.tv_sec*1000000 + ts.tv_nsec/1000);

    #endif
}


namespace profiler
{
//==============================================================================

class Counter;

Counter*    GetCounter( uint32 id );

static unsigned         MaxCounterCount     = 64;
static unsigned         HistoryCount        = 100;

static Counter*         CurCounter          = 0;
static Counter**        ActiveCounter       = 0;
static unsigned         ActiveCounterCount  = 0;
static bool             DumpPending         = false;
static long             TotalTime0          = 0;
static long             TotalTime           = 0;
static unsigned         MaxNameLen          = 32;



class 
Counter
{
public:
                Counter()
                  : t0(0),
                    t(0),
                    count(0),
                    id(0),
                    parentId(InvalidIndex),
                    name(0),
                    used(false)
                {
                }

    void        setName( const char* n )        { name = n; }

    void        reset()                         { t = 0; count = 0; }
    void        start()
                                                {
                                                    if( ActiveCounterCount )
                                                        parentId = ActiveCounter[ActiveCounterCount-1]->id;
                                                    else
                                                        parentId = InvalidIndex;

                                                    ActiveCounter[ActiveCounterCount] = this;
                                                    ++ActiveCounterCount;

                                                    ++count;
                                                    t0 = _CurTimeUs();
                                                    used = true;
                                                }
    void        stop()                          { t += _CurTimeUs() - t0; --ActiveCounterCount; }


    const char* getName() const                 { return name; }

    unsigned    getCount() const                { return count; }
    long        getTimeUs() const               { return t; }

    uint32      getId() const                   { return id; }
    uint32      getParentId() const             { return parentId; }
    bool        isUsed() const                  { return used; }
    unsigned    nestingLevel() const            { return _nesting_level(); }



private :
friend void Init( unsigned max_counter_count, unsigned history_len );
friend void Start();
friend void Stop();
friend bool DumpAverage();
friend bool GetAverageCounters( std::vector<CounterInfo>* info );

private:

    unsigned    _nesting_level() const          { return (parentId != InvalidIndex)  ? GetCounter(parentId)->_nesting_level()+1  : 0; }

    long        t0;
    long        t;
    unsigned    count;

    uint32      id;
    uint32      parentId;
    const char* name;      // ref-only, expected to be immutable string

    unsigned    used:1;
};

static Counter*         _Counter    = 0;
static Counter*         _Average    = 0;




//==============================================================================

static Counter*
GetCounter( uint32 id )
{
    return (id < MaxCounterCount)  ? CurCounter+id  : 0;
}


//------------------------------------------------------------------------------

void 
Init( unsigned max_counter_count, unsigned history_len )
{
    HistoryCount    = history_len;
    MaxCounterCount = max_counter_count;

   _Counter         = new Counter[MaxCounterCount*HistoryCount];
   _Average         = new Counter[MaxCounterCount];
   ActiveCounter    = new Counter*[MaxCounterCount];

    Counter*    counter = _Counter;
    
    for( unsigned h=0; h!=HistoryCount; ++h )
    {
        for( Counter* c=counter,*c_end=counter+MaxCounterCount; c!=c_end; ++c )
        {        
            c->id       = c - counter;
            c->parentId = InvalidIndex;
            c->used     = false;
        }

        counter += MaxCounterCount;
    }
}


//------------------------------------------------------------------------------

void
SetCounterName( unsigned counter_id, const char* name )
{
    if( counter_id < MaxCounterCount )
    {
        Counter*    counter = _Counter + counter_id;

        for( unsigned h=0; h!=HistoryCount; ++h )
        {
            counter->setName( name );
            counter += MaxCounterCount;
        }
    }
}


//------------------------------------------------------------------------------

void
StartCounter( unsigned counter_id )
{
    if( counter_id < MaxCounterCount )
    {
        Counter*    counter = CurCounter + counter_id;
        
        counter->start();
    }
}


//------------------------------------------------------------------------------

void
StartCounter( unsigned counter_id, const char* counter_name )
{
    if( counter_id < MaxCounterCount )
    {
        Counter*    counter = CurCounter + counter_id;
        
        counter->setName( counter_name );
        counter->start();
    }
}


//------------------------------------------------------------------------------

void
StopCounter( unsigned counter_id )
{
    if( counter_id < MaxCounterCount )
    {
        Counter*    counter = CurCounter + counter_id;
        
        counter->stop();
    }
}


//------------------------------------------------------------------------------

void
Start()
{
    if( CurCounter )
    {
        CurCounter += MaxCounterCount;
        if( CurCounter >= _Counter+MaxCounterCount*HistoryCount )
        {
            CurCounter = _Counter;
        }
    }
    else
    {
        CurCounter = _Counter;
    }

    for( Counter* c=CurCounter,*c_end=CurCounter+MaxCounterCount; c!=c_end; ++c )
    {
        c->reset();
        c->used = false;
    }

    ActiveCounterCount  = 0;
    TotalTime0          = _CurTimeUs();
}


//------------------------------------------------------------------------------

void
Stop()
{
    TotalTime = _CurTimeUs() - TotalTime0;

    if( DumpPending )
    {
        Dump();
        DumpPending = false;
    }
}


//------------------------------------------------------------------------------

static inline int
flt_dec( float f )
{
    return int((f - float(int(f)))*10.0f);
}

static void
_Dump( const std::vector<CounterInfo>& result, bool show_percents=false )
{
    unsigned    max_name_len = 0;
    
    for( unsigned i=0; i!=result.size(); ++i )
    {
        unsigned    pi      = result[i].parent_i;
        unsigned    indent  = 0;
        unsigned    len     = 0;

        while( pi != unsigned(-1) )
        {
            pi = result[pi].parent_i;
            ++indent;
        }
        
        if( result[i].name )
            len += strlen( result[i].name );

        len += indent*2;
        
        if( len > max_name_len )
            max_name_len = len;
    }

    Logger::Info( "---\n" );
    for( unsigned i=0; i!=result.size(); ++i )
    {
        unsigned    pi          = result[i].parent_i;
        unsigned    indent      = 0;
        char        text[256];  memset( text, ' ', sizeof(text) );
        unsigned    len         = 0;

        while( pi != unsigned(-1) )
        {
            pi = result[pi].parent_i;
            ++indent;
        }

        int  text_len = 0;       

        if( result[i].name )
            text_len = sprintf( text+indent*2, "%s", result[i].name );
        else
            text_len = sprintf( text+indent*2, "%u", i );
        
        text[indent*2+text_len] = ' ';
        text_len = max_name_len+2+sprintf( text+max_name_len+2, " %-5u  %u us", result[i].count, result[i].time_us );

        if( show_percents )
        {
            float               pg  = (TotalTime)  
                                      ? 100.0f*float(result[i].time_us)/float(TotalTime)
                                      : 0;
            const CounterInfo*  pc  = (result[i].parent_i != InvalidIndex)  ? &(result[0]) + result[i].parent_i  : 0;
            float               pl  = (pc  &&  pc->time_us)  
                                      ? 100.0f*float(result[i].time_us)/float(pc->time_us)
                                      : 0;

            text[text_len] = ' ';
            text_len = max_name_len + 2 + 1 + 5 + 2 + 5+1+2;

            if( pc )    text_len += sprintf( text+text_len, "   %02i.%i    %02i.%i", int(pl),flt_dec(pl), int(pg),flt_dec(pg) );
            else        text_len += sprintf( text+text_len, "   %02i.%i    %02i.%i", int(pg),flt_dec(pg), int(pg),flt_dec(pg) );
        }

        Logger::Info( text );
    }
    Logger::Info( "\n" );
}


//------------------------------------------------------------------------------

void
Dump()
{
    static std::vector<CounterInfo> result;

    GetCounters( &result );
    _Dump( result, true );
}


//------------------------------------------------------------------------------

bool
DumpAverage()
{
    bool                            success = false;
    static std::vector<CounterInfo> result;

    if( GetAverageCounters( &result ) )
    {
        _Dump( result, false );
        success = true;
    }

    return success;
}


//------------------------------------------------------------------------------

static void 
_CollectCountersWithChilds( const Counter* base, const Counter* counter, std::vector<Counter*>* result )
{
    for( const Counter* c=base,*c_end=base+MaxCounterCount; c!=c_end; ++c )
    {
        if(     c->isUsed()  
            &&  c->getParentId() == counter->getId() 
          )
        {
            result->push_back( (Counter*)c );
            _CollectCountersWithChilds( base, c, result );
        }
    }
}


//------------------------------------------------------------------------------

static void
_CollectActiveCounters( Counter* cur_counter, std::vector<Counter*>* result )
{
    // get top-level counters, sorted by time

    static std::vector<Counter*>  top;

    top.clear();
    for( Counter* c=cur_counter,*c_end=cur_counter+MaxCounterCount; c!=c_end; ++c )
    {
        if( !c->isUsed() )
          continue;

        bool    do_add = true;

        if( c->getParentId() == InvalidIndex )
        {
            for( unsigned i=0; i!=top.size(); ++i )
            {
                if( c->getTimeUs() > top[i]->getTimeUs() )
                {
                    top.insert( top.begin()+i, c );
                    do_add = false;
                    break;
                }
            }

            if( do_add )
                top.push_back( c );
                        
        }
    }    
        

    // fill active-counter list

    result->clear();
    for( unsigned i=0; i!=top.size(); ++i )
    {
        result->push_back( top[i] );
        _CollectCountersWithChilds( cur_counter, top[i], result );
    }
}


//------------------------------------------------------------------------------

void    
GetCounters( std::vector<CounterInfo>* info )
{
    static std::vector<Counter*>  result;
    
    _CollectActiveCounters( CurCounter, &result );

    info->resize( result.size() );
    for( unsigned i=0; i!=result.size(); ++i )
    {
        (*info)[i].name     = result[i]->getName();
        (*info)[i].count    = result[i]->getCount();
        (*info)[i].time_us  = result[i]->getTimeUs();
        (*info)[i].parent_i = InvalidIndex;
        
        for( unsigned k=0; k!=info->size(); ++k )
        {
            if( result[i]->getParentId() == result[k]->getId() )
            {
                (*info)[i].parent_i = k;
                break;
            }
        }
    }
}


//------------------------------------------------------------------------------

bool
GetAverageCounters( std::vector<CounterInfo>* info )
{
    bool    success = false;

    if( CurCounter == _Counter + MaxCounterCount*(HistoryCount-1) )
    {
        static std::vector<Counter*>    result;

        for( Counter* c=_Average,*c_end=_Average+MaxCounterCount; c!=c_end; ++c )
        {
            Counter*    src = _Counter + (c-_Average);
            
            c->reset();
            c->setName( src->getName() );

            c->id       = src->id;
            c->parentId = src->parentId;
            c->t0       = 0;
            c->t        = 0;
            c->used     = src->used;
        }
    
        for( unsigned h=0; h!=HistoryCount; ++h )
        {
            Counter*    counter = _Counter + h*MaxCounterCount;
            
            for( Counter* c=counter,*c_end=counter+MaxCounterCount,*a=_Average; c!=c_end; ++c,++a )
            {        
                a->count += c->count;
                a->t0     = 0;
                a->t     += c->getTimeUs();
            }
        }

        for( Counter* c=_Average,*c_end=_Average+MaxCounterCount; c!=c_end; ++c )
        {
            c->count /= HistoryCount;
            c->t     /= HistoryCount;
        }

        
        _CollectActiveCounters( _Average, &result );

        info->resize( result.size() );
        for( unsigned i=0; i!=result.size(); ++i )
        {
            (*info)[i].name     = result[i]->getName();
            (*info)[i].count    = result[i]->getCount();
            (*info)[i].time_us  = result[i]->getTimeUs();
            (*info)[i].parent_i = InvalidIndex;
        
            for( unsigned k=0; k!=info->size(); ++k )
            {
                if( result[i]->getParentId() == result[k]->getId() )
                {
                    (*info)[i].parent_i = k;
                    break;
                }
            }
        }

        success = true;
    }
    
    return success;
}


//==============================================================================
} // namespace profiler

