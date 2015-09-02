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

typedef ResourcePool<QueryBufferGLES2_t,RESOURCE_QUERY_BUFFER,QueryBuffer::Descriptor,false>  QueryBufferGLES2Pool;
RHI_IMPL_POOL(QueryBufferGLES2_t,RESOURCE_QUERY_BUFFER,QueryBuffer::Descriptor,false);


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
gles2_QueryBuffer_Reset( Handle handle )
{
    QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get( handle );

    if( buf )
    {
    }
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
		#elif defined(__DAVAENGINE_ANDROID__)
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
	#elif defined(__DAVAENGINE_ANDROID__)
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
		#elif defined(__DAVAENGINE_ANDROID__)
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
    dispatch->impl_QueryBuffer_Reset    = &gles2_QueryBuffer_Reset;
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
	    #elif defined(__DAVAENGINE_ANDROID__)
            #else
            glGenQueries( 1, &q );
            #endif
            
            buf->query[objectIndex] = q;
        }
        
        if( q )
        {
            #if defined(__DAVAENGINE_IPHONE__)
            glBeginQueryEXT( GL_ANY_SAMPLES_PASSED_EXT, q );
	        #elif defined(__DAVAENGINE_ANDROID__)
            #elif defined(__DAVAENGINE_MACOS__)
            glBeginQuery( GL_SAMPLES_PASSED, q );
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
			#elif defined(__DAVAENGINE_ANDROID__)
            #elif defined(__DAVAENGINE_MACOS__)
            glEndQuery( GL_SAMPLES_PASSED );
            #else
            glEndQuery( GL_ANY_SAMPLES_PASSED );
            #endif
        }
    }
}

}


//==============================================================================
} // namespace rhi

