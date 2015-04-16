

    #include "_mcpp.h"

    #include "FileSystem/DynamicMemoryFile.h"
    #include "Base/BaseTypes.h"
    using DAVA::uint8;

    #include <stdio.h>

static DAVA::DynamicMemoryFile* _Input          = 0;
static int                      _InputEOF       = 0;
static FILE* const              _DefaultInput   = (FILE*)(0xDEADBABE);
const char*                     MCPP_Text       = "<input>";


//------------------------------------------------------------------------------

void
mcpp__set_input( const void* data, unsigned data_sz )
{
    _Input    = DAVA::DynamicMemoryFile::Create( (const uint8*)data, data_sz, DAVA::File::READ );
    _InputEOF = 0;
}


//------------------------------------------------------------------------------

FILE* 
mcpp__fopen( const char* filename, const char* mode )
{
    if( !strcmp( filename, MCPP_Text ) )
        return _DefaultInput;
    else
        return NULL;
};


//------------------------------------------------------------------------------

int
mcpp__fclose( FILE* file )
{
    if( !file )
        return -1;

    DVASSERT(file == _DefaultInput);
    return 0;
}


//------------------------------------------------------------------------------

int
mcpp__ferror( FILE* file )
{
    DVASSERT(file == _DefaultInput);
    return _InputEOF;
}


//------------------------------------------------------------------------------

char*
mcpp__fgets( char* buf, int max_size, FILE* file )
{
    DVASSERT(file == _DefaultInput);
    
    if( _Input->IsEof() )
    {
        buf[0]      = 0;
        _InputEOF   = 1;
    }
    else
    {
        _Input->ReadLine( (void*)buf, max_size );

        // workaround to prevent MCPP from ignoring line
        if( !buf[0] )
        {
            buf[0] = ' ';
            buf[1] = 0;
        }
    }

    return buf;
}


//------------------------------------------------------------------------------

int
mcpp__stat( const char* path, stat_t* buffer )
{
    int ret = -1;
    
    if( strcmp( path, MCPP_Text ) != 0 )
    {
        memset( buffer, 0, sizeof(stat_t) );
        buffer->st_mode = S_IFREG | S_IREAD;
        ret = 0;
    }

    return ret;
}

