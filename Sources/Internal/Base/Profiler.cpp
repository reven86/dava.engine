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
    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    #include "Platform/FWSpinlock.h"
    #include "BaseTypes.h"
    #include "Platform/SystemTimer.h"
    using namespace DAVA;


    #include <stdio.h>
    #include <string.h>
    #include <vector>


//==============================================================================

const unsigned  InvalidIndex = (unsigned)(-1);



//==============================================================================

static inline long
CurTimeUs()
{
    return (long)(SystemTimer::Instance()->GetAbsoluteUs());
}



namespace profiler
{
//==============================================================================

class Counter;

static Counter* GetCounter( uint32 id );

static bool             profilerInited      = false;
static uint32           maxCounterCount     = 0;
static uint32           historyCount        = 0;

static Counter*         profCounter         = 0;
static Counter*         profAverage         = 0;
static Counter*         curCounter          = 0;
static bool             profStarted         = false;
static Counter**        activeCounter       = 0;
static uint32           activeCounterCount  = 0;
static uint64           totalTime0          = 0;
static uint64           totalTime           = 0;
static uint32           maxNameLen          = 32;
static Spinlock         counterSync;



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
                                                    counterSync.Lock();

                                                    t         = 0; 
                                                    count     = 0; 
                                                    used      = false;
                                                    useCount  = 0;
                                                    parentId  = InvalidIndex;
                                                    
                                                    counterSync.Unlock();
                                                }
    void        start()
                                                {
                                                    counterSync.Lock();

                                                    if( !useCount ) 
                                                    {
                                                        if( activeCounterCount )
                                                            parentId = activeCounter[activeCounterCount-1]->id;
                                                        else
                                                            parentId = InvalidIndex;

                                                        activeCounter[activeCounterCount] = this;
                                                        ++activeCounterCount;
                                                        
                                                        t0 = CurTimeUs();
                                                    }

                                                    used = true;
                                                    ++count;
                                                    ++useCount;
                                                    
                                                    counterSync.Unlock();
                                                }
    void        stop()                          
                                                { 
                                                    counterSync.Lock();

                                                    if( useCount == 1 )
                                                    {
                                                        if( activeCounterCount )
                                                            --activeCounterCount; 
                                                    }

                                                    if( --useCount == 0 )
                                                        t += CurTimeUs() - t0; 
                                                    
                                                    counterSync.Unlock();
                                                }


    const char* getName() const                 { return name; }

    uint32      getCount() const                { return count; }
    uint64      getTimeUs() const               { return t; }

    uint32      getId() const                   { return id; }
    uint32      getParentId() const             { return parentId; }
    bool        isUsed() const                  { return (used)  ? true  : false; }
    uint32      nestingLevel() const            { return _nesting_level(); }



private :
friend void Init( uint32 max_counter_count, uint32 history_len );
friend void Start();
friend void Stop();
friend bool DumpAverage();
friend bool GetAverageCounters( std::vector<CounterInfo>* info );

private:

    uint32      _nesting_level() const          { return (parentId != InvalidIndex)  ? GetCounter(parentId)->_nesting_level()+1  : 0; }

    uint64      t0;
    uint64      t;
    uint32      count;

    uint32      id;
    uint32      parentId;
    const char* name;      // ref-only, expected to be immutable string

    uint32      used:1;
    uint32      useCount:4;
};





//==============================================================================

static Counter*
GetCounter( uint32 id )
{
    return (id < maxCounterCount)  ? curCounter+id  : 0;
}


//------------------------------------------------------------------------------

void 
Init( uint32 max_counter_count, uint32 history_len )
{
    DVASSERT(!profilerInited);

    historyCount    = history_len;
    maxCounterCount = max_counter_count;

    profCounter     = new Counter[maxCounterCount*historyCount];
    profAverage     = new Counter[maxCounterCount];
    activeCounter   = new Counter*[maxCounterCount];

    Counter*    counter = profCounter;
    
    for( uint32 h=0; h!=historyCount; ++h )
    {
        for( Counter* c=counter,*c_end=counter+maxCounterCount; c!=c_end; ++c )
        {        
            c->id        = c - counter;
            c->parentId  = InvalidIndex;
            c->used      = false;
        }

        counter += maxCounterCount;
    }

    profilerInited = true;
}


//------------------------------------------------------------------------------

void
EnsureInited( uint32 max_counter_count, uint32 history_length )
{
    if( !profilerInited )
    {
        Init( max_counter_count, history_length );
    }
}


//------------------------------------------------------------------------------

void
Uninit()
{
    if( profCounter )
    {
        delete[] profCounter;
        profCounter = 0;
    }

    if( profAverage )
    {
        delete[] profAverage;
        profAverage = 0;
    }

    if( activeCounter )
    {
        delete[] activeCounter;
        activeCounter = 0;
    }

    historyCount    = 0;
    maxCounterCount = 0;

    profilerInited = false;
}


//------------------------------------------------------------------------------

void
SetCounterName( unsigned counter_id, const char* name )
{
    DVASSERT( counter_id < maxCounterCount );

    Counter*    counter = profCounter + counter_id;

    for( uint32 h=0; h!=historyCount; ++h )
    {
        counter->setName( name );
        counter += maxCounterCount;
    }
}


//------------------------------------------------------------------------------

void
StartCounter( uint32 counter_id )
{
    DVASSERT( counter_id < maxCounterCount );

    Counter*    counter = curCounter + counter_id;
        
    counter->start();
}


//------------------------------------------------------------------------------

void
StartCounter( uint32 counter_id, const char* counter_name )
{
    DVASSERT( counter_id < maxCounterCount );

    Counter*    counter = curCounter + counter_id;
        
    counter->setName( counter_name );
    counter->start();
}


//------------------------------------------------------------------------------

void
StopCounter( uint32 counter_id )
{
    DVASSERT( counter_id < maxCounterCount );

    Counter*    counter = curCounter + counter_id;
        
    counter->stop();
}


//------------------------------------------------------------------------------

void
Start()
{
    DVASSERT(!profStarted);

    if( curCounter )
    {
        curCounter += maxCounterCount;
        if( curCounter >= profCounter+maxCounterCount*historyCount )
        {
            curCounter = profCounter;
        }
    }
    else
    {
        curCounter = profCounter;
    }

    for( Counter* c=curCounter,*c_end=curCounter+maxCounterCount; c!=c_end; ++c )
    {
        c->reset();
        c->used = false;
    }

    profStarted         = true;
    activeCounterCount  = 0;
    totalTime0          = CurTimeUs();
}


//------------------------------------------------------------------------------

void
Stop()
{
    DVASSERT(profStarted);

    totalTime   = CurTimeUs() - totalTime0;
    profStarted = false;
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
    
    for( unsigned i=0,i_end=result.size(); i!=i_end; ++i )
    {
        uint32  pi      = result[i].parentIndex;
        uint32  indent  = 0;
        uint32  len     = 0;

        while( pi != InvalidIndex )
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
    for( unsigned i=0,i_end=result.size(); i!=i_end; ++i )
    {
        uint32  pi          = result[i].parentIndex;
        uint32  indent      = 0;
        char    text[256];  memset( text, ' ', sizeof(text) );
        uint32  len         = 0;

        while( pi != InvalidIndex )
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
            float               pg  = (totalTime)  
                                      ? 100.0f*float(result[i].timeUs)/float(totalTime)
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
    DVASSERT(!profStarted);

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
    for( const Counter* c=base,*c_end=base+maxCounterCount; c!=c_end; ++c )
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
    for( Counter* c=cur_counter,*c_end=cur_counter+maxCounterCount; c!=c_end; ++c )
    {
        if( !c->isUsed() )
          continue;

        bool    do_add = true;

        if( c->getParentId() == InvalidIndex )
        {
            for( uint32 i=0,i_end=top.size(); i!=i_end; ++i )
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
    for( uint32 i=0; i!=top.size(); ++i )
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
    
    _CollectActiveCounters( curCounter, &result );

    info->resize( result.size() );
    for( uint32 i=0,i_end=result.size(); i!=i_end; ++i )
    {
        (*info)[i].name         = result[i]->getName();
        (*info)[i].count        = result[i]->getCount();
        (*info)[i].timeUs       = result[i]->getTimeUs();
        (*info)[i].parentIndex  = InvalidIndex;
        
        for( unsigned k=0,k_end=info->size(); k!=k_end; ++k )
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

    if( curCounter == profCounter + maxCounterCount*(historyCount-1) )
    {
        for( Counter* c=profAverage,*c_end=profAverage+maxCounterCount; c!=c_end; ++c )
        {
            Counter*    src = profCounter + (c-profAverage);
            
            c->reset();
            c->setName( src->getName() );

            c->id        = src->id;
            c->parentId  = src->parentId;
            c->t0        = 0;
            c->t         = 0;
            c->used      = src->used;
        }
    
        for( uint32 h=0; h!=historyCount; ++h )
        {
            Counter*    counter = profCounter + h*maxCounterCount;
            
            for( Counter* c=counter,*c_end=counter+maxCounterCount,*a=profAverage; c!=c_end; ++c,++a )
            {        
                a->count += c->count;
                a->t0     = 0;
                a->t     += c->getTimeUs();
            }
        }

        for( Counter* c=profAverage,*c_end=profAverage+maxCounterCount; c!=c_end; ++c )
        {
            c->count /= historyCount;
            c->t     /= historyCount;
        }

        
        static std::vector<Counter*>    result;
        
        _CollectActiveCounters( profAverage, &result );

        info->resize( result.size() );
        for( uint32 i=0,i_end=result.size(); i!=i_end; ++i )
        {
            (*info)[i].name         = result[i]->getName();
            (*info)[i].count        = result[i]->getCount();
            (*info)[i].timeUs       = result[i]->getTimeUs();
            (*info)[i].parentIndex  = InvalidIndex;
        
            for( uint32 k=0,k_end=info->size(); k!=k_end; ++k )
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

