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


    #include "dbg_StatSet.h"
    
    #include "Concurrency/Spinlock.h"

    #include <vector>


//==============================================================================

namespace statset
{

struct
Stat
{
    char        full_name[64];
    char        short_name[32];
    unsigned    parent_id;
    unsigned    value;
    unsigned    is_permanent:1;
};

typedef std::vector<Stat> StatArray;

static StatArray        _Stat;
static DAVA::Spinlock   _StatSync;


//==============================================================================
} // namespace statset


//==============================================================================
//
//  publics:

void     
StatSet::ResetAll()
{
    using namespace statset;
    
    _StatSync.Lock();    
    for( StatArray::iterator s=_Stat.begin(),s_end=_Stat.end(); s!=s_end; ++s )
    {
        if( !s->is_permanent )
            s->value = 0;
    }
    _StatSync.Unlock();    
}


//------------------------------------------------------------------------------

unsigned 
StatSet::AddStat( const char* full_name, const char* short_name, unsigned parent_id )
{
    using namespace statset;
    
    unsigned    id = InvalidIndex;

    _StatSync.Lock();    
    {
    _Stat.resize( _Stat.size()+1 );

    Stat&   stat = _Stat.back();

    strncpy( stat.full_name, full_name, sizeof(stat.full_name) );
    strncpy( stat.short_name, short_name, sizeof(stat.short_name) );
    stat.parent_id     = parent_id;
    stat.value         = 0;
    stat.is_permanent  = false;

    id = _Stat.size()-1;
    }
    _StatSync.Unlock();    


    return id;
}


//------------------------------------------------------------------------------

unsigned 
StatSet::AddPermanentStat( const char* full_name, const char* short_name, unsigned parent_id )
{
    using namespace statset;

    unsigned    id = InvalidIndex;

    _StatSync.Lock();    
    {
    _Stat.resize( _Stat.size()+1 );

    Stat&   stat = _Stat.back();

    strncpy( stat.full_name, full_name, sizeof(stat.full_name) );
    strncpy( stat.short_name, short_name, sizeof(stat.short_name) );
    stat.parent_id     = parent_id;
    stat.value         = 0;
    stat.is_permanent  = true;

    id = _Stat.size()-1;
    }
    _StatSync.Unlock();


    return id;
}


//------------------------------------------------------------------------------

void     
StatSet::SetStat( unsigned id, unsigned value )
{
    using namespace statset;
    
    _StatSync.Lock();    

    if( id < _Stat.size() )
        _Stat[id].value = value;
    
    _StatSync.Unlock();    
}


//------------------------------------------------------------------------------

void     
StatSet::IncStat( unsigned id, unsigned delta )
{
    using namespace statset;
    
    _StatSync.Lock();    
    
    if( id < _Stat.size() )
        _Stat[id].value += delta;
    
    _StatSync.Unlock();    
}


//------------------------------------------------------------------------------

void     
StatSet::DecStat( unsigned id, unsigned delta )
{
    using namespace statset;
    
    _StatSync.Lock();
        
    if( id < _Stat.size() )
    {
        unsigned    val = _Stat[id].value;
        _Stat[id].value = (delta >= val)  ? val-delta  : 0;
    }

    _StatSync.Unlock();
}


//------------------------------------------------------------------------------

unsigned 
StatSet::StatValue( unsigned id )
{
    using namespace statset;
    
    _StatSync.Lock();
    unsigned    val = (id < _Stat.size())  ? _Stat[id].value  : 0;
    _StatSync.Unlock();

    return val;
}


//------------------------------------------------------------------------------

unsigned 
StatSet::StatID( const char* name )
{
    using namespace statset;

    unsigned id = InvalidIndex;

    _StatSync.Lock();
    for( StatArray::const_iterator s=_Stat.begin(),s_end=_Stat.end(); s!=s_end; ++s )
    {
        if( strcmp( name, s->full_name ) == 0 )
        {
            id = s - _Stat.begin();
            break;
        }
    }
    _StatSync.Unlock();

    return id;
}


//------------------------------------------------------------------------------

const char*
StatSet::StatFullName( unsigned id )
{
    using namespace statset;

    _StatSync.Lock();
    const char* name = (id < _Stat.size())  ? _Stat[id].full_name  : "<invalid id>";
    _StatSync.Unlock();

    return name;
}


//------------------------------------------------------------------------------

void     
StatSet::DumpStats()
{
    using namespace statset;

/*
    unsigned max_len = 0;

    for( StatArray::Iterator s=_Stat.begin(),s_end=_Stat.end(); s!=s_end; ++s )
    {
        if( s->full_name.length() > max_len )
            max_len = s->full_name.length();
    }
    
    for( StatArray::Iterator s=_Stat.begin(),s_end=_Stat.end(); s!=s_end; ++s )
    {
        char    name[64];

        strcpy( name, s->full_name.c_str() );
        memset( name+s->full_name.length(), ' ', max_len-s->full_name.length() );
        name[max_len] = '\0';

        Trace( "%s = %u\n", name, s->value );
    }
*/
}


