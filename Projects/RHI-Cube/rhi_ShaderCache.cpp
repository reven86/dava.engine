
    #include "rhi_ShaderCache.h"
    

namespace rhi
{
static ShaderBuilder    _ShaderBuilder  = 0;

struct
ProgInfo
{
    DAVA::FastName      uid;
    std::vector<uint8>  bin;
};

static std::vector<ProgInfo> _ProgInfo;

namespace ShaderCache
{
//------------------------------------------------------------------------------

bool
Initialize( ShaderBuilder builder )
{
    _ShaderBuilder = builder;
    return true;
}


//------------------------------------------------------------------------------

void
Unitialize()
{
}


//------------------------------------------------------------------------------

void
Clear()
{
}


//------------------------------------------------------------------------------

void
Load( const char* binFileName )
{
}


//------------------------------------------------------------------------------

bool
GetProg( const DAVA::FastName& uid, std::vector<uint8>* bin )
{
    bool    success = false;

    for( unsigned i=0; i!=_ProgInfo.size(); ++i )
    {
        if( _ProgInfo[i].uid == uid )
        {
            bin->clear();
            bin->insert( bin->begin(), _ProgInfo[i].bin.begin(), _ProgInfo[i].bin.end() );
            success = true;
            break;
        }
    }

    return success;
}


//------------------------------------------------------------------------------

void
UpdateProg( ProgType progType, const DAVA::FastName& uid, const char* srcText )
{
    bool    do_add = true;

    for( unsigned i=0; i!=_ProgInfo.size(); ++i )
    {
        if( _ProgInfo[i].uid == uid )
        {
            const uint8*    src     = (const uint8*)srcText;
            unsigned        src_sz  = strlen( srcText );

            _ProgInfo[i].bin.clear();
            _ProgInfo[i].bin.insert( _ProgInfo[i].bin.begin(), src, src+src_sz );
            _ProgInfo[i].bin.push_back( 0 );

            do_add = false;
            break;
        }
    }

    if( do_add )
    {
        const uint8*    src     = (const uint8*)srcText;
        unsigned        src_sz  = strlen( srcText );
        
        _ProgInfo.push_back( ProgInfo() );
        
        _ProgInfo.back().uid = uid;
        
        _ProgInfo.back().bin.clear();
        _ProgInfo.back().bin.insert( _ProgInfo.back().bin.begin(), src, src+src_sz );
        _ProgInfo.back().bin.push_back( 0 );
    }
}



} // namespace ShaderCache
} // namespace rhi