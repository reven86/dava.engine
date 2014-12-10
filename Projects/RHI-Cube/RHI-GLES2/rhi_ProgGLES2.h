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
                              : _location(-1),
                                _count(0),
                                _data(0),
                                _is_dirty(false)
                            {}
                            ~ConstBuf()
                            {
                                ConstBuf::destroy();
                            }
    
        bool                construct( unsigned loc, unsigned count );
        void                destroy();

        unsigned            const_count() const;
        bool                set_const( unsigned const_i, unsigned count, const float* data );
        void                set_to_rhi() const;
    
    
    private:        
        
        unsigned            _location;

        unsigned            _count;
        float*              _data;
        mutable unsigned    _is_dirty:1;
    };



private:

    enum
    {
        MaxConstBufCount    = 8
    };

    struct
    ConstBufInfo
    {
        unsigned    location;
        unsigned    count;
    };

    ConstBufInfo        cbuf[MaxConstBufCount];
    unsigned            texunitLoc[16];
    
    unsigned            shader;
    const ProgType      type;
    mutable unsigned    texunitInited:1;
};


//==============================================================================
} // namespace rhi
#endif // __RHI_PROGGLES2_HPP__

