//==============================================================================
//
//  externals:

    #include "dbg_StatSet.h"

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

static StatArray _Stat;


//==============================================================================
} // namespace statset


//==============================================================================
//
//  publics:

void     
StatSet::ResetAll()
{
    using namespace statset;
    
    for( StatArray::iterator s=_Stat.begin(),s_end=_Stat.end(); s!=s_end; ++s )
    {
        if( !s->is_permanent )
            s->value = 0;
    }
}


//------------------------------------------------------------------------------

unsigned 
StatSet::AddStat( const char* full_name, const char* short_name, unsigned parent_id )
{
    using namespace statset;

    _Stat.resize( _Stat.size()+1 );

    Stat&   stat = _Stat.back();

    strncpy( stat.full_name, full_name, sizeof(stat.full_name) );
    strncpy( stat.short_name, short_name, sizeof(stat.short_name) );
    stat.parent_id     = parent_id;
    stat.value         = 0;
    stat.is_permanent  = false;

    return _Stat.size()-1;
}


//------------------------------------------------------------------------------

unsigned 
StatSet::AddPermanentStat( const char* full_name, const char* short_name, unsigned parent_id )
{
    using namespace statset;

    _Stat.resize( _Stat.size()+1 );

    Stat&   stat = _Stat.back();

    strncpy( stat.full_name, full_name, sizeof(stat.full_name) );
    strncpy( stat.short_name, short_name, sizeof(stat.short_name) );
    stat.parent_id     = parent_id;
    stat.value         = 0;
    stat.is_permanent  = true;

    return _Stat.size()-1;
}


//------------------------------------------------------------------------------

void     
StatSet::SetStat( unsigned id, unsigned value )
{
    using namespace statset;

    if( id < _Stat.size() )
        _Stat[id].value = value;
}


//------------------------------------------------------------------------------

void     
StatSet::IncStat( unsigned id, unsigned delta )
{
    using namespace statset;
    
    if( id < _Stat.size() )
        _Stat[id].value += delta;
}


//------------------------------------------------------------------------------

void     
StatSet::DecStat( unsigned id, unsigned delta )
{
    using namespace statset;
    
    if( id < _Stat.size() )
    {
        unsigned    val = _Stat[id].value;
        _Stat[id].value = (delta >= val)  ? val-delta  : 0;
    }
}


//------------------------------------------------------------------------------

unsigned 
StatSet::StatValue( unsigned id )
{
    using namespace statset;

    return (id < _Stat.size())  ? _Stat[id].value  : 0;
}


//------------------------------------------------------------------------------

unsigned 
StatSet::StatID( const char* name )
{
    using namespace statset;

    unsigned id = InvalidIndex;

    for( StatArray::const_iterator s=_Stat.begin(),s_end=_Stat.end(); s!=s_end; ++s )
    {
        if( strcmp( name, s->full_name ) == 0 )
        {
            id = s - _Stat.begin();
            break;
        }
    }

    return id;
}


//------------------------------------------------------------------------------

const char*
StatSet::StatFullName( unsigned id )
{
    using namespace statset;

    return (id < _Stat.size())  ? _Stat[id].full_name  : "<invalid id>";
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


