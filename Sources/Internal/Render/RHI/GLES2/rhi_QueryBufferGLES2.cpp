
    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_GLES2.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_gl.h"


namespace rhi
{
//==============================================================================

class
QueryBufferGLES2_t
{
public:
                        QueryBufferGLES2_t();
                        ~QueryBufferGLES2_t();

    std::vector<GLuint> query;
};

typedef Pool<QueryBufferGLES2_t,RESOURCE_QUERY_BUFFER>  QueryBufferGLES2Pool;
RHI_IMPL_POOL(QueryBufferGLES2_t,RESOURCE_QUERY_BUFFER);


//==============================================================================


QueryBufferGLES2_t::QueryBufferGLES2_t()
{
}


//------------------------------------------------------------------------------

QueryBufferGLES2_t::~QueryBufferGLES2_t()
{
}

static Handle
gles2_QueryBuffer_Create( uint32 maxObjectCount )
{
    Handle              handle = QueryBufferGLES2Pool::Alloc();
    QueryBufferGLES2_t* buf    = QueryBufferGLES2Pool::Get( handle );

    if( buf )
    {
        buf->query.resize( maxObjectCount );
        memset( &(buf->query[0]), 0, sizeof(buf->query[0])*buf->query.size() );
    }

    return handle;
}

static void
gles2_QueryBuffer_Delete( Handle handle )
{
    QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get( handle );

    if( buf )
    {
        for( std::vector<GLuint>::iterator q=buf->query.begin(),q_end=buf->query.end(); q!=q_end; ++q )
        {
            GLuint  id = *q;

            if( id )
            {
                #if defined(__DAVAENGINE_IPHONE__)
                glDeleteQueriesEXT( 1, &id );
                #else
                glDeleteQueries( 1, &id );
                #endif
            }
        }

        buf->query.clear();
    }

    QueryBufferGLES2Pool::Free( handle );
}

static bool
gles2_QueryBuffer_IsReady( Handle handle, uint32 objectIndex )
{
    bool                ready = false;
    QueryBufferGLES2_t* buf   = QueryBufferGLES2Pool::Get( handle );

    if( buf  &&  objectIndex < buf->query.size() )
    {
        GLuint  result = 0;

        #if defined(__DAVAENGINE_IPHONE__)
        glGetQueryObjectuivEXT( buf->query[objectIndex], GL_QUERY_RESULT_AVAILABLE_EXT, &result );
        #else
        glGetQueryObjectuiv( buf->query[objectIndex], GL_QUERY_RESULT_AVAILABLE, &result );
        #endif
        
        ready = result == GL_TRUE;
    }

    return ready;
}

static int
gles2_QueryBuffer_Value( Handle handle, uint32 objectIndex )
{
    int                 value = 0;
    QueryBufferGLES2_t* buf   = QueryBufferGLES2Pool::Get( handle );

    if( buf  &&  objectIndex < buf->query.size() )
    {
        GLuint  result = 0;

        #if defined(__DAVAENGINE_IPHONE__)
        glGetQueryObjectuivEXT( buf->query[objectIndex], GL_QUERY_RESULT_EXT, &result );
        #else
        glGetQueryObjectuiv( buf->query[objectIndex], GL_QUERY_RESULT, &result );
        #endif
        
        value = result;
    }

    return value;
}


namespace QueryBufferGLES2
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_QueryBuffer_Create   = &gles2_QueryBuffer_Create;
    dispatch->impl_QueryBuffer_Delete   = &gles2_QueryBuffer_Delete;
    dispatch->impl_QueryBuffer_IsReady  = &gles2_QueryBuffer_IsReady;
    dispatch->impl_QueryBuffer_Value    = &gles2_QueryBuffer_Value;
}

void
BeginQuery( Handle handle, uint32 objectIndex )
{
    QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get( handle );

    if( buf  &&  objectIndex < buf->query.size() )
    {
        GLuint q = buf->query[objectIndex];

        if( !q )
        {
            #if defined(__DAVAENGINE_IPHONE__)
            glGenQueriesEXT( 1, &q );
            #else
            glGenQueries( 1, &q );
            #endif
            
            buf->query[objectIndex] = q;
        }
        
        if( q )
        {
            #if defined(__DAVAENGINE_IPHONE__)
            glBeginQueryEXT( GL_ANY_SAMPLES_PASSED_EXT, q );
            #else
            glBeginQuery( GL_ANY_SAMPLES_PASSED, q );
            #endif
        }
    }
}


void
EndQuery( Handle handle, uint32 objectIndex )
{
    QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get( handle );

    if( buf  &&  objectIndex < buf->query.size() )
    {
        GLuint q = buf->query[objectIndex];

        if( q )
        {
            #if defined(__DAVAENGINE_IPHONE__)
            glEndQueryEXT( GL_ANY_SAMPLES_PASSED_EXT );
            #else
            glEndQuery( GL_ANY_SAMPLES_PASSED );
            #endif
        }
    }
}

}


//==============================================================================
} // namespace rhi

