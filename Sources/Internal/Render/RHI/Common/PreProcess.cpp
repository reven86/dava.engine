
    #include "PreProcess.h"

    #include "../rhi_Type.h"

    #include "MCPP/mcpp_lib.h"

    #include <stdio.h>
    #include <stdarg.h>


static std::string*         _PreprocessedText = nullptr;


//------------------------------------------------------------------------------

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
            
        default :
        {
        }
    }

    return ch;
}


//------------------------------------------------------------------------------

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
            
        default :
        {}
    }
        
    return 0;
}


//------------------------------------------------------------------------------

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
            
        default :
        {}
    }
        
    return count;
}


//------------------------------------------------------------------------------

void
PreProcessText( const char* text, std::string* result )
{
    const char*   argv[] =
    { 
        "<mcpp>",   // we just need first arg
        "-P",       // do not output #line directives 
        "-C",       // keep comments
        MCPP_Text
    };

    _PreprocessedText = result;
    mcpp__set_input( text, strlen(text) );

    mcpp_set_out_func( &_mcpp__fputc, &_mcpp__fputs, &_mcpp__fprintf );
    mcpp_lib_main( countof(argv), (char**)argv );
    _PreprocessedText = nullptr;
}


//------------------------------------------------------------------------------

void
PreProcessText( const char* text, const char** arg, unsigned argCount, std::string* result )
{
    if( text )
    {
        const char* argv[128];
        int         argc    = 0;
        DVASSERT(argCount<countof(argv)-2);

        argv[argc++] = "<mcpp>";// we just need first arg
        argv[argc++] = "-P";    // do not output #line directives
        argv[argc++] = "-C";    // keep comments
        for( const char** a=arg,**a_end=arg+argCount; a!=a_end; ++a )
            argv[argc++] = *a;
        argv[argc++] = MCPP_Text;

        _PreprocessedText = result;
        mcpp__set_input( text, strlen(text) );

        mcpp_set_out_func( &_mcpp__fputc, &_mcpp__fputs, &_mcpp__fprintf );
        mcpp_lib_main( argc, (char**)argv );
        _PreprocessedText = nullptr;
    }
    else
    {
        *result = "";
    }
}
