
    #include "../rhi_Base.h"
    #include "../RHI/rhi_ShaderCache.h"
    #include "../RHI/rhi_Pool.h"
    #include "rhi_RingBufferMetal.h"

    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_metal.h"


namespace rhi
{


class
PipelineStateMetal_t
{
public:
                        PipelineStateMetal_t()
                        {}

    class
    ConstBuf
    {
    public:
        
        enum ProgType { PROG_VERTEX, PROG_FRAGMENT };

                            ConstBuf()
                              : index(InvalidIndex),
                                count(0),
                                data(0),
                                inst_offset(0),
                                inst_valid(false)
                            {}
                            ~ConstBuf()
                            {
                                ConstBuf::Destroy();
                            }
    
        bool                Construct( ProgType type, unsigned index, unsigned count );
        void                Destroy();

        unsigned            ConstCount() const;
        bool                SetConst( unsigned const_i, unsigned count, const float* cdata );
        
        void                SetToRHI( unsigned bufIndex, id<MTLRenderCommandEncoder> ce ) const;
    
    
    private:        

        ProgType            type;
        unsigned            index;
        unsigned            count;
        float*              data;
        mutable float*      inst;
        mutable unsigned    inst_offset;
        mutable uint32      inst_valid:1;
    };

    struct
    VertexProg
    {
                        VertexProg()
                        {}
        void            GetBufferInfo( MTLRenderPipelineReflection* info );
        Handle          InstanceConstBuffer( unsigned buf_i );

        struct
        BufInfo
        {
            unsigned    index;
            unsigned    count;
            unsigned    used:1;
        };
        BufInfo         cbuf[MAX_CONST_BUFFER_COUNT];
    };

    struct
    FragmentProg
    {
                        FragmentProg()
                        {}
        void            GetBufferInfo( MTLRenderPipelineReflection* info );
        Handle          InstanceConstBuffer( unsigned buf_i );

        struct
        BufInfo
        {
            unsigned    index;
            unsigned    count;
            unsigned    used:1;
        };
        BufInfo         cbuf[MAX_CONST_BUFFER_COUNT];
    };

    VertexProg      vprog;
    FragmentProg    fprog;

    id<MTLRenderPipelineState>  state;
};

typedef Pool<PipelineStateMetal_t>              PipelineStateMetalPool;
typedef Pool<PipelineStateMetal_t::ConstBuf>    ConstBufMetalPool;

RHI_IMPL_POOL(PipelineStateMetal_t);
RHI_IMPL_POOL(PipelineStateMetal_t::ConstBuf);


static RingBufferMetal                          DefaultConstRingBuffer;


void
PipelineStateMetal_t::FragmentProg::GetBufferInfo( MTLRenderPipelineReflection* info )
{
    for( unsigned i=0; i!=countof(cbuf); ++i )
    {
        cbuf[i].index = i;
        cbuf[i].count = 0;
        cbuf[i].used  = false;
    
        for( MTLArgument* arg in info.fragmentArguments )
        {
            if(     arg.active  
                &&  arg.type == MTLArgumentTypeBuffer 
                &&  arg.index == i
              )
            {
                MTLStructType*  str = arg.bufferStructType;
                
                for( MTLStructMember* member in str.members )
                {
                    if( member.dataType == MTLDataTypeArray )
                    {
                        MTLArrayType*   arr = member.arrayType;
                        
                        cbuf[i].index = i;
                        cbuf[i].count = arr.arrayLength;
                        cbuf[i].used  = true;
                        break;
                    }
                }
                
                break;
            }
        }
    }
}

Handle 
PipelineStateMetal_t::FragmentProg::InstanceConstBuffer( unsigned bufIndex )
{
    Handle  handle = InvalidHandle;

    DVASSERT(bufIndex < countof(cbuf));
//    DVASSERT(cbuf[bufIndex].location != InvalidIndex);
    
    if( bufIndex < countof(cbuf)  &&  cbuf[bufIndex].index != InvalidIndex )
    {
        handle = ConstBufMetalPool::Alloc();

        ConstBuf*   cb = ConstBufMetalPool::Get( handle );

        if( !cb->Construct( ConstBuf::PROG_FRAGMENT, cbuf[bufIndex].index, cbuf[bufIndex].count ) )
        {
            ConstBufMetalPool::Free( handle );
            handle = InvalidHandle;
        }
    }

    return handle;
}



void
PipelineStateMetal_t::VertexProg::GetBufferInfo( MTLRenderPipelineReflection* info )
{
    for( unsigned i=0; i!=countof(cbuf); ++i )
    {
        cbuf[i].index = i;
        cbuf[i].count = 0;
        cbuf[i].used  = false;

        for( MTLArgument* arg in info.vertexArguments )
        {
//            const char* name = arg.name.UTF8String;
            if(     arg.active  
                &&  arg.type == MTLArgumentTypeBuffer 
                &&  arg.index == 1+i // CRAP: vprog-buf#0 assumed to be vdata
              )
            {
                MTLStructType*  str = arg.bufferStructType;
                
                for( MTLStructMember* member in str.members )
                {
                    if( member.dataType == MTLDataTypeArray )
                    {
                        MTLArrayType*   arr = member.arrayType;
                        
                        cbuf[i].index = i;
                        cbuf[i].count = arr.arrayLength;
                        cbuf[i].used  = true;
                        break;
                    }
                }
                break;
            }
        }
    }
}

Handle 
PipelineStateMetal_t::VertexProg::InstanceConstBuffer( unsigned bufIndex )
{	
    Handle  handle = InvalidHandle;

    DVASSERT(bufIndex < countof(cbuf));
//    DVASSERT(cbuf[bufIndex].location != InvalidIndex);
    
    if( bufIndex < countof(cbuf)  &&  cbuf[bufIndex].index != InvalidIndex )
    {
        handle = ConstBufMetalPool::Alloc();

        ConstBuf*   cb = ConstBufMetalPool::Get( handle );

        if( !cb->Construct( ConstBuf::PROG_VERTEX, cbuf[bufIndex].index, cbuf[bufIndex].count ) )
        {
            ConstBufMetalPool::Free( handle );
            handle = InvalidHandle;
        }
    }

    return handle;
}

    
//------------------------------------------------------------------------------

bool
PipelineStateMetal_t::ConstBuf::Construct( PipelineStateMetal_t::ConstBuf::ProgType ptype, unsigned buf_i, unsigned cnt )
{
    type        = ptype;
    index       = buf_i;
    count       = cnt;
    data        = (float*)::malloc( cnt*4*sizeof(float) );
    inst        = 0;
    inst_offset = 0;
    inst_valid  = false;

    return true;
}


//------------------------------------------------------------------------------

void
PipelineStateMetal_t::ConstBuf::Destroy()
{
    if( data )
    {
        ::free( data );
        data = 0;
    }
    
    index       = InvalidIndex;
    count       = 0;
    inst        = 0;
    inst_offset = 0;
    inst_valid  = false;
}


//------------------------------------------------------------------------------

unsigned
PipelineStateMetal_t::ConstBuf::ConstCount() const
{
    return count;
}


//------------------------------------------------------------------------------

bool
PipelineStateMetal_t::ConstBuf::SetConst( unsigned const_i, unsigned const_count, const float* cdata )
{
    bool    success = false;

    if( const_i + const_count <= count )
    {
        memcpy( data + const_i*4, cdata, const_count*4*sizeof(float) );
        inst_valid = false;
        success    = true;
    }

    return success;
}


//------------------------------------------------------------------------------

void
PipelineStateMetal_t::ConstBuf::SetToRHI( unsigned bufIndex, id<MTLRenderCommandEncoder> ce ) const
{
    id<MTLBuffer>   buf = DefaultConstRingBuffer.BufferUID();
    
    if( !inst_valid )
    {
        inst = DefaultConstRingBuffer.Alloc( count*4*sizeof(float), &inst_offset );
        memcpy( inst, data, count*4*sizeof(float) );
        inst_valid = true;
    }

    if( type == PROG_VERTEX )
        [ce setVertexBuffer:buf offset:inst_offset atIndex:1+bufIndex]; // CRAP: vprog-buf#0 assumed to be vdata
    else
        [ce setFragmentBuffer:buf offset:inst_offset atIndex:bufIndex];
}

    


namespace PipelineState
{

Handle
Create( const PipelineState::Descriptor& desc )
{
    Handle                      handle      = PipelineStateMetalPool::Alloc();;
    PipelineStateMetal_t*       ps          = PipelineStateMetalPool::Get( handle );
    static std::vector<uint8>   vprog_bin;
    static std::vector<uint8>   fprog_bin;

    rhi::ShaderCache::GetProg( desc.vprogUid, &vprog_bin );
    rhi::ShaderCache::GetProg( desc.fprogUid, &fprog_bin );


    // compile vprog

    NSString*           vp_src  = [NSString stringWithUTF8String:(const char*)(&vprog_bin[0])];
    MTLCompileOptions*  vp_opt  = [MTLCompileOptions alloc];
    NSError*            vp_err  = nil;
    id<MTLLibrary>      vp_lib  = [_Metal_Device newLibraryWithSource:vp_src options:vp_opt error:&vp_err];
    id<MTLFunction>     vp_func = [vp_lib newFunctionWithName:@"vp_main"];

    if( vp_err == nil )
    {
        if( vp_func == nil )
        {
            Logger::Error( "FAILED to get vprog \"%s\" function", desc.fprogUid.c_str() );
        }
    }
    else
    {
        Logger::Error( "FAILED to compile vprog \"%s\" :\n%s", desc.vprogUid.c_str(), vp_err.localizedDescription.UTF8String );
    }

    
    // compile fprog

    NSString*           fp_src  = [NSString stringWithUTF8String:(const char*)(&fprog_bin[0])];
    MTLCompileOptions*  fp_opt  = [MTLCompileOptions alloc];
    NSError*            fp_err  = nil;
    id<MTLLibrary>      fp_lib  = [_Metal_Device newLibraryWithSource:fp_src options:fp_opt error:&fp_err];
    id<MTLFunction>     fp_func = nil;
    
    if( fp_err == nil )
    {
        fp_func = [fp_lib newFunctionWithName:@"fp_main"];
        
        if( fp_func == nil )
        {
            Logger::Error( "FAILED to get fprog \"%s\" function", desc.fprogUid.c_str() );
        }
    }
    else
    {
        Logger::Error( "FAILED to compile fprog \"%s\" :\n%s", desc.fprogUid.c_str(), fp_err.localizedDescription.UTF8String );
    }


    // create render-state
    
    if( vp_func != nil  &&  fp_func != nil )
    {
        MTLRenderPipelineDescriptor*    desc    = [MTLRenderPipelineDescriptor new];
        MTLRenderPipelineReflection*    ps_info = nil;
        NSError*                        rs_err  = nil;
        
        desc.depthAttachmentPixelFormat         = MTLPixelFormatDepth32Float;
        desc.colorAttachments[0].pixelFormat    = MTLPixelFormatBGRA8Unorm;
        desc.sampleCount                        = 1;
        desc.vertexFunction                     = vp_func;
        desc.fragmentFunction                   = fp_func;

        ps->state = [_Metal_Device newRenderPipelineStateWithDescriptor:desc options:MTLPipelineOptionBufferTypeInfo reflection:&ps_info error:&rs_err];
        
        if( rs_err == nil )
        {
            ps->vprog.GetBufferInfo( ps_info );
            ps->fprog.GetBufferInfo( ps_info );
        }
        else
        {
            Logger::Error( "FAILED create pipeline-state:\n%s", rs_err.localizedDescription.UTF8String );
        }
    }
    else
    {
        handle = InvalidHandle;
    }

    return handle;
} 










Handle
CreateVProgConstBuffer( Handle ps, unsigned bufIndex )
{
    PipelineStateMetal_t* psm = PipelineStateMetalPool::Get( ps );
    
    return psm->vprog.InstanceConstBuffer( bufIndex );
}


Handle
CreateFProgConstBuffer( Handle ps, unsigned bufIndex )
{
    PipelineStateMetal_t* psm = PipelineStateMetalPool::Get( ps );
    
    return psm->fprog.InstanceConstBuffer( bufIndex );
}


} // // namespace PipelineState


namespace ConstBuffer
{

bool
SetConst( Handle cb, uint32 constIndex, uint32 constCount, const float* data )
{
    PipelineStateMetal_t::ConstBuf* buf = ConstBufMetalPool::Get( cb );

    return buf->SetConst( constIndex, constCount, data );
}

}


namespace PipelineStateMetal
{

void
SetToRHI( Handle ps, id<MTLRenderCommandEncoder> ce )
{
    PipelineStateMetal_t* psm = PipelineStateMetalPool::Get( ps );

    DVASSERT(psm);

    [ce setRenderPipelineState:psm->state];
}

} // namespace PipelineStateMetal


namespace ConstBufferMetal
{

void
InitializeRingBuffer( uint32 size )
{
    DefaultConstRingBuffer.Initialize( size );
}


void
SetToRHI( Handle buf, unsigned bufIndex, id<MTLRenderCommandEncoder> ce )
{
    PipelineStateMetal_t::ConstBuf* cbuf = ConstBufMetalPool::Get( buf );

    cbuf->SetToRHI( bufIndex, ce );
}

}




} // namespace rhi
