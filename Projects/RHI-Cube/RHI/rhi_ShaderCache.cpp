
    #include "../MCPP/mcpp_lib.h"

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

static std::string*         _PreprocessedText = 0;

static int 
_mcpp__fputc( int ch, OUTDEST dst )
{
    switch( dst )
    {
        case MCPP_OUT :
        {
            if( _PreprocessedText )
                _PreprocessedText->push_back( (char)ch );
        }   break;

        case MCPP_ERR :
        {
        }   break;

        case MCPP_DBG :
        {
        }   break;
    }

    return ch;
}


static int
_mcpp__fputs( const char* str, OUTDEST dst )
{
    switch( dst )
    {
        case MCPP_OUT :
        {
            if( _PreprocessedText )
                *_PreprocessedText += str;
        }   break;

        case MCPP_ERR :
        {
        }   break;

        case MCPP_DBG :
        {
        }   break;
    }
        
    return 0;
}


static int 
_mcpp__fprintf( OUTDEST dst, const char* format, ... )
{
    va_list     arglist;
    char        buf[2048];
    int         count     = 0;

    va_start( arglist, format );
    count = vsnprintf( buf, countof(buf), format, arglist );
    va_end( arglist );

    switch( dst )
    {
        case MCPP_OUT :
        {
            if( _PreprocessedText )
                *_PreprocessedText += buf;
        }   break;

        case MCPP_ERR :
        {
        }   break;

        case MCPP_DBG :
        {
        }   break;
    }
        
    return count;
}



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

static void
PreProcessSource( Api targetApi, const char* srcText, std::string* preprocessedText )
{
    _PreprocessedText = preprocessedText;
    mcpp__set_input( srcText, strlen(srcText) );

    char*   argv[] = 
    { 
        "<mcpp>",   // we just need first arg
        "-P",       // do not output #line directives 
        "-C",       // keep comments
        "<input>"
    };

    mcpp_set_out_func( &_mcpp__fputc, &_mcpp__fputs, &_mcpp__fprintf );
    mcpp_lib_main( countof(argv), argv );
    _PreprocessedText = 0;
}


//------------------------------------------------------------------------------

void
UpdateProg( Api targetApi, ProgType progType, const DAVA::FastName& uid, const char* srcText )
{
    std::string         txt;
    std::vector<uint8>* bin = 0;

    PreProcessSource( targetApi, srcText, &txt );

    for( unsigned i=0; i!=_ProgInfo.size(); ++i )
    {
        if( _ProgInfo[i].uid == uid )
        {
            bin = &(_ProgInfo[i].bin);
            break;
        }
    }

    if( !bin )
    {
        _ProgInfo.push_back( ProgInfo() );
        
        _ProgInfo.back().uid = uid;        
        bin = &(_ProgInfo.back().bin);
    }

    bin->clear();
    bin->insert( bin->begin(), (const uint8*)(&(txt[0])), (const uint8*)(&(txt[txt.length()-1])+1) );
    bin->push_back( 0 );
}



} // namespace ShaderCache
} // namespace rhi