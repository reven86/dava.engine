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

    #include "Profiler.h"
    #include "FileSystem/Logger.h"
    #include "Platform/FWSpinlock.h"
    #include "BaseTypes.h"
    #include "Platform/SystemTimer.h"

    #ifdef __DAVAENGINE_WIN32__
        #define WIN32_LEAN_AND_MEAN
        #include <windows.h>
    #elif __DAVAENGINE_ANDROID__
        #include <time.h>
        #include <sys/atomics.h>
    #endif

    #include <stdio.h>
    #include <string.h>
    #include <vector>


//==============================================================================

const unsigned                      InvalidIndex = (unsigned)(-1);
#if defined __DAVAENGINE_WIN32__
static LARGE_INTEGER                TimerFreq;
#elif defined(__DAVAENGINE_IPHONE__)  ||  defined(__DAVAENGINE_MACOS__)
static mach_timebase_info_data_t    TimeBase;
#endif 



//==============================================================================

static long
_CurTimeUs()
{
    #ifdef __DAVAENGINE_WIN32__

        LARGE_INTEGER t;

        ::QueryPerformanceCounter( &t );
    
        return long(((t.QuadPart)*1000000) / TimerFreq.QuadPart);

    #elif defined __DAVAENGINE_ANDROID__

        timespec    ts;

        clock_gettime( CLOCK_REALTIME, &ts );
        //   this gives more correct time, but slow-as-Hell on lots of devices
        //   clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts );

        return long(ts.tv_sec*1000000 + ts.tv_nsec/1000);
    
    #elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
	    
	    return (mach_absolute_time() * timebase.numer) / timebase.denom;

    #else

    #error high-resolution timer must be defined!

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
//static CASLock          CounterSync;
static Spinlock         CounterSync;



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
                    used(false),
                    useCount(0)
                {
                }

    void        setName( const char* n )        { name = n; }

    void        reset()                         
                                                { 
                                                    CounterSync.Lock();

                                                    t         = 0; 
                                                    count     = 0; 
                                                    used      = false;
                                                    useCount  = 0;
                                                    parentId  = InvalidIndex;
                                                    
                                                    CounterSync.Unlock();
                                                }
    void        start()
                                                {
                                                    CounterSync.Lock();

                                                    if( !useCount ) 
                                                    {
                                                        if( ActiveCounterCount )
                                                            parentId = ActiveCounter[ActiveCounterCount-1]->id;
                                                        else
                                                            parentId = InvalidIndex;

                                                        ActiveCounter[ActiveCounterCount] = this;
                                                        ++ActiveCounterCount;
                                                        
                                                        t0 = _CurTimeUs();
                                                    }

                                                    used = true;
                                                    ++count;
                                                    ++useCount;
                                                    
                                                    CounterSync.Unlock();
                                                }
    void        stop()                          
                                                { 
                                                    CounterSync.Lock();

                                                    if( useCount == 1 )
                                                    {
                                                        if( ActiveCounterCount )
                                                            --ActiveCounterCount; 
                                                    }

                                                    if( --useCount == 0 )
                                                        t += _CurTimeUs() - t0; 
                                                    
                                                    CounterSync.Unlock();
                                                }


    const char* getName() const                 { return name; }

    unsigned    getCount() const                { return count; }
    long        getTimeUs() const               { return t; }

    uint32      getId() const                   { return id; }
    uint32      getParentId() const             { return parentId; }
    bool        isUsed() const                  { return (used)  ? true  : false; }
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

    int         used:1;
    int         useCount:4;
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
            c->id        = c - counter;
            c->parentId  = InvalidIndex;
            c->used      = false;
        }

        counter += MaxCounterCount;
    }

    #if defined __DAVAENGINE_WIN32__

        ::QueryPerformanceFrequency( &TimerFreq );
    
    #elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
	    
        (void) mach_timebase_info( &TimeBase );
    
	    while( ((TimeBase.numer % 10) == 0)  &&  ((TimeBase.denom % 10) == 0) )
	    {
		    TimeBase.numer /= 10;
		    TimeBase.denom /= 10;
	    }
    
    #endif
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
        unsigned    pi      = result[i].parentIndex;
        unsigned    indent  = 0;
        unsigned    len     = 0;

        while( pi != unsigned(-1) )
        {
            pi = result[pi].parentIndex;
            ++indent;
        }
        
        if( result[i].name )
            len += strlen( result[i].name );

        len += indent*2;
        
        if( len > max_name_len )
            max_name_len = len;
    }

    Logger::Info( "===================================================" );
    for( unsigned i=0; i!=result.size(); ++i )
    {
        unsigned    pi          = result[i].parentIndex;
        unsigned    indent      = 0;\
        char        text[256];  memset( text, ' ', sizeof(text) );
        unsigned    len         = 0;

        while( pi != unsigned(-1) )
        {
            pi = result[pi].parentIndex;
            ++indent;
        }

        int  text_len = 0;       

        if( result[i].name )
            text_len = Snprinf( text+indent*2, sizeof(text)-indent*2, "%s", result[i].name );
        else
            text_len = Snprinf( text+indent*2, sizeof(text)-indent*2, "%u", i );
        
        text[indent*2+text_len] = ' ';
        text_len = max_name_len+2+Snprinf( text+max_name_len+2, sizeof(text)-max_name_len-2, " %-5u  %u us", result[i].count, result[i].timeUs );

        if( show_percents )
        {
            float               pg  = (TotalTime)  
                                      ? 100.0f*float(result[i].timeUs)/float(TotalTime)
                                      : 0;
            const CounterInfo*  pc  = (result[i].parentIndex != InvalidIndex)  ? &(result[0]) + result[i].parentIndex  : 0;
            float               pl  = (pc  &&  pc->timeUs)  
                                      ? 100.0f*float(result[i].timeUs)/float(pc->timeUs)
                                      : 0;

            text[text_len] = ' ';
            text_len = max_name_len + 2 + 1 + 5 + 2 + 5+1+2;

            if( pc )    text_len += Snprinf( text+text_len, sizeof(text)-text_len, "   %02i.%i    %02i.%i", int(pl),flt_dec(pl), int(pg),flt_dec(pg) );
            else        text_len += Snprinf( text+text_len, sizeof(text)-text_len, "   %02i.%i    %02i.%i", int(pg),flt_dec(pg), int(pg),flt_dec(pg) );
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
        (*info)[i].name         = result[i]->getName();
        (*info)[i].count        = result[i]->getCount();
        (*info)[i].timeUs       = result[i]->getTimeUs();
        (*info)[i].parentIndex  = InvalidIndex;
        
        for( unsigned k=0; k!=info->size(); ++k )
        {
            if( result[i]->getParentId() == result[k]->getId() )
            {
                (*info)[i].parentIndex = k;
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

            c->id        = src->id;
            c->parentId  = src->parentId;
            c->t0        = 0;
            c->t         = 0;
            c->used      = src->used;
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
            (*info)[i].name         = result[i]->getName();
            (*info)[i].count        = result[i]->getCount();
            (*info)[i].timeUs       = result[i]->getTimeUs();
            (*info)[i].parentIndex  = InvalidIndex;
        
            for( unsigned k=0; k!=info->size(); ++k )
            {
                if( result[i]->getParentId() == result[k]->getId() )
                {
                    (*info)[i].parentIndex = k;
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

