#if !defined __RHI_PROGGLES2_HPP__
#define __RHI_PROGGLES2_HPP__

    #include "../Common/rhi_Private.h"


namespace rhi
{
//==============================================================================

class
ProgGLES2
{
public:


                    ProgGLES2( ProgType t );
    virtual         ~ProgGLES2();

    bool            Construct( const char* src_data );
    void            Destroy();

    unsigned        ShaderUid() const;
    unsigned        TextureLocation( unsigned texunitIndex ) const;
    const char*     UniformBufferName() const;
    void            GetProgParams( unsigned progUid );

    unsigned        ConstBufferCount() const;
    Handle          InstanceConstBuffer( unsigned bufIndex );
    void            SetToRHI() const;


    static void     InvalidateAllConstBufferInstances();


    class
    ConstBuf
    {
    public:

                            ConstBuf()
                              : location(-1),
                                count(0),
                                data(nullptr),
                                inst(nullptr)
/*
                                data(0),
                                isDirty(false),
                                isInstanced(false)
*/                            
                            {}
                            ~ConstBuf()
                            {
                                ConstBuf::Destroy();
                            }
    
        bool                Construct( unsigned loc, unsigned count );
        void                Destroy();

        unsigned            ConstCount() const;
        bool                SetConst( unsigned const_i, unsigned count, const float* cdata );
        bool                SetConst( unsigned const_i, unsigned const_sub_i, const float* cdata, unsigned data_count );
        
        const void*         Instance() const;
        void                SetToRHI( const void* instData ) const;
        void                InvalidateInstance();
    
    
    private:        
        
        unsigned            location;
        #if DV_USE_UNIFORMBUFFER_OBJECT
        unsigned            ubo;
        #endif

        unsigned            count;
        float*              data;
        mutable float*      inst;

/*
        unsigned            count;
        mutable float*      data;
        mutable uint32      isDirty:1;
        mutable uint32      isInstanced:1;
*/
    };



private:

    struct
    ConstBufInfo
    {
        unsigned    location;
        unsigned    count;
    };

    ConstBufInfo        cbuf[MAX_CONST_BUFFER_COUNT];
    unsigned            texunitLoc[16];
    
    unsigned            shader;
    const ProgType      type;
    mutable unsigned    texunitInited:1;
};


//==============================================================================
} // namespace rhi
#endif // __RHI_PROGGLES2_HPP__

