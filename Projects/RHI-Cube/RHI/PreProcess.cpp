
    #include "PreProcess.h"

    #include "../rhi_Type.h"

    #include "../MCPP/mcpp_lib.h"

    #include <stdio.h>
    #include <stdarg.h>


static std::string*         _PreprocessedText = 0;


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
    }
        
    return count;
}


//------------------------------------------------------------------------------

void
PreProcessText( const char* text, std::string* result )
{
    char*   argv[] = 
    { 
        "<mcpp>",   // we just need first arg
        "-P",       // do not output #line directives 
        "-C",       // keep comments
        "<input>"
    };

    _PreprocessedText = result;
    mcpp__set_input( text, strlen(text) );

    mcpp_set_out_func( &_mcpp__fputc, &_mcpp__fputs, &_mcpp__fprintf );
    mcpp_lib_main( countof(argv), argv );
    _PreprocessedText = 0;
}

