#if !defined __RHI_PROGGLES2_HPP__
#define __RHI_PROGGLES2_HPP__

//    #include "rhi_ConstBuffer.hpp"
    #include "../rhi_Base.h"


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



    class
    ConstBuf
    {
    public:

                            ConstBuf()
                              : location(-1),
                                count(0),
                                data(0),
                                isDirty(false)
                            {}
                            ~ConstBuf()
                            {
                                ConstBuf::Destroy();
                            }
    
        bool                Construct( unsigned loc, unsigned count );
        void                Destroy();

        unsigned            ConstCount() const;
        bool                SetConst( unsigned const_i, unsigned count, const float* cdata );
        void                SetToRHI() const;
    
    
    private:        
        
        unsigned            location;

        unsigned            count;
        float*              data;
        mutable unsigned    isDirty:1;
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

