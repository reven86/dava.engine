

    #include "_mcpp.h"

    #include "FileSystem/DynamicMemoryFile.h"
    #include "Base/BaseTypes.h"
    using DAVA::uint8;

    #include <stdio.h>

static DAVA::DynamicMemoryFile* _Input          = 0;
static FILE* const              _DefaultInput   = (FILE*)(0xDEADBABE);


//------------------------------------------------------------------------------

void
mcpp__set_input( const void* data, unsigned data_sz )
{
    _Input = DAVA::DynamicMemoryFile::Create( (const uint8*)data, data_sz, DAVA::File::READ );
}


//------------------------------------------------------------------------------

FILE* 
mcpp__fopen( const char* filename, const char* mode )
{
    if( !strcmp( filename, "<input>" ) )
        return _DefaultInput;
    else
        return NULL;
//    return fopen( filename, mode );
};


//------------------------------------------------------------------------------

int
mcpp__fclose( FILE* file )
{
    DVASSERT(file == _DefaultInput);
    return 0;
//    return fclose( file );
}


//------------------------------------------------------------------------------

int
mcpp__ferror( FILE* file )
{
    DVASSERT(file == _DefaultInput);
    return (_Input->IsEof()) ? 1 : 0;
//    return ferror( file );
}


//------------------------------------------------------------------------------

char*
mcpp__fgets( char* buf, int max_size, FILE* file )
{
    DVASSERT(file == _DefaultInput);
    _Input->ReadLine( (void*)buf, max_size );

    // workaround to prevent MCPP from stopping processing
    if( !buf[0] )
    {
        buf[0] = ' ';
        buf[1] = 0;
    }

    return buf;
//    return fgets( buf, max_size, file );
}

