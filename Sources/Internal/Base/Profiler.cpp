//==============================================================================
//
//  Profiler implementation
//
//==============================================================================
//
//  externals:

    #include "Profiler.hpp"

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
    #define Array std::vector


//==============================================================================

const unsigned  InvalidIndex        = (unsigned)(-1);
#define         countof(array)      (sizeof(array)/sizeof(array[0]))


void     
Logs( const char* str )
{
    #ifdef _WIN32
        ::OutputDebugStringA( str );
    #elif defined __ANDROID_API__
        __android_log_print( ANDROID_LOG_INFO, "prof", "%s", str );
    #endif
}

void     
Log( const char* format, ... )
{
    va_list     arglist;
    char        msg[1024];
    
    va_start( arglist, format );
    vsnprintf( msg, countof(msg)-1, format, arglist );
    va_end( arglist );

    Logs( msg );
}


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
// this gives more correct time, but slow as Hell on lots of devices
//   clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts );

    return long(ts.tv_sec*1000000 + ts.tv_nsec/1000);

    #endif
}


namespace profiler
{
//==============================================================================

class Counter;

Counter*    GetCounter( uint32 id );

static const unsigned   MaxCounterCount                 = 64;
static const unsigned   HistoryCount                    = 100;

static Counter*         _CurCounter                     = 0;
static Counter*         _ActiveCounter[MaxCounterCount];
static unsigned         _ActiveCounterCount             = 0;
static bool             _DumpPending                    = false;
static long             _TotalTime0                     = 0;
static long             _TotalTime                      = 0;
static unsigned         _MaxNameLen                     = 32;



class 
Counter
{
public:
                Counter()
                  : _t0(0),
                    _t(0),
                    _count(0),
                    _id(0),
                    _parent_id(InvalidIndex),
                    _name(0),
                    _used(false)
                {
                }

    void        set_name( const char* n )       { _name = n; }

    void        reset()                         { _t = 0; _count = 0; }
    void        start()
                                                {
                                                    if( _ActiveCounterCount )
                                                        _parent_id = _ActiveCounter[_ActiveCounterCount-1]->_id;
                                                    else
                                                        _parent_id = InvalidIndex;

                                                    _ActiveCounter[_ActiveCounterCount] = this;
                                                    ++_ActiveCounterCount;

                                                    ++_count;
                                                    _t0 = _CurTimeUs();
                                                    _used = true;
                                                }
    void        stop()                          { _t += _CurTimeUs() - _t0; --_ActiveCounterCount; }


    const char* name() const                    { return _name; }

    unsigned    count() const                   { return _count; }
    long        time_us() const                 { return _t; }

    uint32      id() const                      { return _id; }
    uint32      parent_id() const               { return _parent_id; }
    bool        is_used() const                 { return _used; }
    unsigned    nesting_level() const           { return _nesting_level(); }



private :
friend void Init();
friend void Start();
friend void Stop();
friend bool DumpAverage();
friend bool GetAverageCounters( std::vector<CounterInfo>* info );

private:

    unsigned    _nesting_level() const          { return (_parent_id != InvalidIndex)  ? GetCounter(_parent_id)->_nesting_level()+1  : 0; }

    long        _t0;
    long        _t;
    unsigned    _count;

    uint32      _id;
    uint32      _parent_id;
    const char* _name;      // ref-only, expected to be immutable string

    unsigned    _used:1;
};

static Counter          _Counter[MaxCounterCount*HistoryCount];




//==============================================================================

Counter*
GetCounter( uint32 id )
{
    return (id < MaxCounterCount)  ? _CurCounter+id  : 0;
}


//------------------------------------------------------------------------------

void 
Init()
{
    Counter*    counter = _Counter;
    
    for( unsigned h=0; h!=HistoryCount; ++h )
    {
        for( Counter* c=counter,*c_end=counter+MaxCounterCount; c!=c_end; ++c )
        {        
            c->_id          = c - counter;
            c->_parent_id   = InvalidIndex;
            c->_used        = false;
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
            counter->set_name( name );
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
        Counter*    counter = _CurCounter + counter_id;
        
        counter->start();
    }
}


//------------------------------------------------------------------------------

void
StartCounter( unsigned counter_id, const char* counter_name )
{
    if( counter_id < MaxCounterCount )
    {
        Counter*    counter = _CurCounter + counter_id;
        
        counter->set_name( counter_name );
        counter->start();
    }
}


//------------------------------------------------------------------------------

void
StopCounter( unsigned counter_id )
{
    if( counter_id < MaxCounterCount )
    {
        Counter*    counter = _CurCounter + counter_id;
        
        counter->stop();
    }
}


//------------------------------------------------------------------------------

void
Start()
{
    if( _CurCounter )
    {
        _CurCounter += MaxCounterCount;
        if( _CurCounter >= _Counter+MaxCounterCount*HistoryCount )
        {
            _CurCounter = _Counter;
        }
    }
    else
    {
        _CurCounter = _Counter;
    }

    for( Counter* c=_CurCounter,*c_end=_CurCounter+MaxCounterCount; c!=c_end; ++c )
    {
        c->reset();
        c->_used = false;
    }

    _ActiveCounterCount = 0;
    _TotalTime0         = _CurTimeUs();
}


//------------------------------------------------------------------------------

void
Stop()
{
    _TotalTime = _CurTimeUs() - _TotalTime0;

    if( _DumpPending )
    {
        Dump();
        _DumpPending = false;
    }
}


//------------------------------------------------------------------------------

void
ScheduleDump()
{
    _DumpPending = true;
}

static inline int
flt_dec( float f )
{
    return int((f - float(int(f)))*10.0f);
}


//------------------------------------------------------------------------------

void 
_DumpWithChilds( const Counter* base, const Counter* counter, unsigned indent=0, bool show_percents=true )
{
    char    line[256];
    int     line_len    = 0;
    char    name[128];  /*memset(name,0,sizeof(name));//*/memset( name, ' ', indent*2 );
    int     l           = (counter->name()) 
                        ? sprintf( name+indent*2, "%s", counter->name() )
                        : sprintf( name+indent*2, "%02u", counter->id() );
/*
if(indent*2+l > _MaxNameLen)
{
Logs("bla!");
}
*/
    memset( name+indent*2+l, ' ', _MaxNameLen-(indent*2+l) );
    name[_MaxNameLen] = '\0';
    
    line_len += sprintf( line+line_len, "%s  %-4u  %-5i us", name, counter->count(), int(counter->time_us()) );

    if( show_percents )
    {
        float           pg  = (_TotalTime)  
                              ? 100.0f*float(counter->time_us())/float(_TotalTime)
                              : 0;
        const Counter*  pc  = (counter->parent_id() != InvalidIndex)  ? base + counter->parent_id()  : 0;
        float           pl  = (pc  &&  pc->time_us())  
                              ? 100.0f*float(counter->time_us())/float(pc->time_us())
                              : 0;

        if( pc )    line_len += sprintf( line+line_len, "   %02i.%i    %02i.%i \n", int(pl),flt_dec(pl), int(pg),flt_dec(pg) );
        else        line_len += sprintf( line+line_len, "   %02i.%i    %02i.%i \n", int(pg),flt_dec(pg), int(pg),flt_dec(pg) );
    }
    else
    {
        strcat( line, "\n" );
        ++line_len;
    }

    Logs( line );

    for( const Counter* c=base,*c_end=base+MaxCounterCount; c!=c_end; ++c )
    {
        if( !c->is_used() )
          continue;
        
        if( c->parent_id() == counter->id() )
            _DumpWithChilds( base, c, indent+1, show_percents );
    }
}


//------------------------------------------------------------------------------

void
_Dump( Counter* cur_counter, bool show_percents=true )
{
    static Array<Counter*>  counter;


    // sort top-level counters by time

    counter.clear();
    _MaxNameLen = 4;
    for( Counter* c=cur_counter,*c_end=cur_counter+MaxCounterCount; c!=c_end; ++c )
    {
        if( !c->is_used() )
          continue;

        bool    do_add = true;

        if( c->parent_id() == InvalidIndex )
        {
            for( unsigned i=0; i<counter.size(); ++i )
            {
                if( c->time_us() > counter[i]->time_us() )
                {
                    counter.insert( counter.begin()+i, c );
                    do_add = false;
                    break;
                }
            }

            if( do_add )
                counter.push_back( c );
        }

        int l = ((c->name()) ? strlen( c->name() ) : 2) + c->nesting_level()*2;

        if( unsigned(l) > _MaxNameLen )
            _MaxNameLen = l;
    }

    char        name[128];
    char        dash[128];
    int         dash_l   = _MaxNameLen + 33;
    char        header[128];

    memset( dash, '-', dash_l );
    dash[dash_l]   = '\n';
    dash[dash_l+1] = '\0';
    strcpy( name, "Name" );
    memset( name+4, ' ', _MaxNameLen-4 );
    name[_MaxNameLen] = '\0';
    Log( dash );
//    sprintf( header, "%s  cnt.  time(us)   %% parent  %% global\n", name );
    sprintf( header, "%s  cnt.  time(us)   parent  global\n", name );
    Logs( header );
    Log( dash );
    
    
    for( unsigned i=0; i<counter.size(); ++i )
        _DumpWithChilds( cur_counter, counter[i], 0, show_percents );
    Log( dash );
    Log( "\n" );
}


//------------------------------------------------------------------------------

void
Dump()
{
    _Dump( _CurCounter );
}


//------------------------------------------------------------------------------

bool
DumpAverage()
{
    bool    success = false;

    if( _CurCounter == _Counter + MaxCounterCount*(HistoryCount-1) )
    {
        Counter     avg[MaxCounterCount];

        for( Counter* c=avg,*c_end=avg+MaxCounterCount; c!=c_end; ++c )
        {
            Counter*    src = _Counter + (c-avg);
            
            c->reset();
            c->set_name( src->name() );

            c->_id          = src->_id;
            c->_parent_id   = src->_parent_id;
            c->_t0          = 0;
            c->_t           = 0;
            c->_used        = src->_used;
        }
    
        for( unsigned h=0; h!=HistoryCount; ++h )
        {
            Counter*    counter = _Counter + h*MaxCounterCount;
            
            for( Counter* c=counter,*c_end=counter+MaxCounterCount,*a=avg; c!=c_end; ++c,++a )
            {        
                a->_count += c->_count;
                a->_t0     = 0;
                a->_t     += c->time_us();
            }
        }

        for( Counter* c=avg,*c_end=avg+MaxCounterCount; c!=c_end; ++c )
        {
            c->_count /= HistoryCount;
            c->_t     /= HistoryCount;
        }

        _Dump( avg, false );
        success = true;
    }
    
    return success;
}



//------------------------------------------------------------------------------

void 
_CollectCountersWithChilds( const Counter* base, const Counter* counter, Array<Counter*>* result )
{
    for( const Counter* c=base,*c_end=base+MaxCounterCount; c!=c_end; ++c )
    {
        if(     c->is_used()  
            &&  c->parent_id() == counter->id() 
          )
        {
            result->push_back( (Counter*)c );
            _CollectCountersWithChilds( base, c, result );
        }
    }
}



//------------------------------------------------------------------------------

static void
_CollectActiveCounters( Counter* cur_counter, Array<Counter*>* result )
{
    // get top-level counters, sorted by time

    static Array<Counter*>  top;

    top.clear();
    for( Counter* c=cur_counter,*c_end=cur_counter+MaxCounterCount; c!=c_end; ++c )
    {
        if( !c->is_used() )
          continue;

        bool    do_add = true;

        if( c->parent_id() == InvalidIndex )
        {
            for( unsigned i=0; i!=top.size(); ++i )
            {
                if( c->time_us() > top[i]->time_us() )
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
    static Array<Counter*>  result;
    
    _CollectActiveCounters( _CurCounter, &result );

    info->resize( result.size() );
    for( unsigned i=0; i!=result.size(); ++i )
    {
        (*info)[i].name     = result[i]->name();
        (*info)[i].count    = result[i]->count();
        (*info)[i].time_us  = result[i]->time_us();
        (*info)[i].parent_i = InvalidIndex;
        
        for( unsigned k=0; k!=info->size(); ++k )
        {
            if( result[i]->parent_id() == result[k]->id() )
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

    if( _CurCounter == _Counter + MaxCounterCount*(HistoryCount-1) )
    {
        Counter                 avg[MaxCounterCount];
        static Array<Counter*>  result;

        for( Counter* c=avg,*c_end=avg+MaxCounterCount; c!=c_end; ++c )
        {
            Counter*    src = _Counter + (c-avg);
            
            c->reset();
            c->set_name( src->name() );

            c->_id          = src->_id;
            c->_parent_id   = src->_parent_id;
            c->_t0          = 0;
            c->_t           = 0;
            c->_used        = src->_used;
        }
    
        for( unsigned h=0; h!=HistoryCount; ++h )
        {
            Counter*    counter = _Counter + h*MaxCounterCount;
            
            for( Counter* c=counter,*c_end=counter+MaxCounterCount,*a=avg; c!=c_end; ++c,++a )
            {        
                a->_count += c->_count;
                a->_t0     = 0;
                a->_t     += c->time_us();
            }
        }

        for( Counter* c=avg,*c_end=avg+MaxCounterCount; c!=c_end; ++c )
        {
            c->_count /= HistoryCount;
            c->_t     /= HistoryCount;
        }

        
        _CollectActiveCounters( avg, &result );

        info->resize( result.size() );
        for( unsigned i=0; i!=result.size(); ++i )
        {
            (*info)[i].name     = result[i]->name();
            (*info)[i].count    = result[i]->count();
            (*info)[i].time_us  = result[i]->time_us();
            (*info)[i].parent_i = InvalidIndex;
        
            for( unsigned k=0; k!=info->size(); ++k )
            {
                if( result[i]->parent_id() == result[k]->id() )
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

