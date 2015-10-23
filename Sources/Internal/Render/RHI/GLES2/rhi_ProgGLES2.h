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
    void            GetProgParams( unsigned progUid );
    unsigned        SamplerCount() const;

    unsigned        ConstBufferCount() const;
    Handle InstanceConstBuffer(unsigned bufIndex) const;
    void            SetupTextureUnits( unsigned baseUnit=0 ) const;


    static void     InvalidateAllConstBufferInstances();


    class
    ConstBuf
    {
    public:
        
        struct Desc {};

        ConstBuf()
            : location(-1)
            , count(0)
            , data(nullptr)
            , inst(nullptr)
            , lastInst(nullptr)
                            {}
                            ~ConstBuf()
                            {
                                ConstBuf::Destroy();
                            }

        bool                Construct(uint32 prog, void** lastBoundData, unsigned loc, unsigned count);
        void                Destroy();

        unsigned            ConstCount() const;
        bool                SetConst( unsigned const_i, unsigned count, const float* cdata );
        bool                SetConst( unsigned const_i, unsigned const_sub_i, const float* cdata, unsigned data_count );
        
        const void*         Instance() const;
        void                SetToRHI(uint32 progUid, const void* instData) const;
        void                InvalidateInstance();

        static void         AdvanceFrame();    

    
    private:
        uint32 glProg;
        uint16 location;
        #if DV_USE_UNIFORMBUFFER_OBJECT
        unsigned            ubo;
        #endif

        uint16 count;
        float*              data;
        mutable float*      inst;
        mutable void**      lastInst;
        mutable uint32      frame;

        static uint32       CurFrame;
    };



private:

    struct
    ConstBufInfo
    {
        unsigned    location;
        unsigned    count;
    };

    ConstBufInfo        cbuf[MAX_CONST_BUFFER_COUNT];
    void* cbufLastBoundData[MAX_CONST_BUFFER_COUNT];
    unsigned            texunitLoc[16];
    
    unsigned            shader;
    uint32 prog;
    const ProgType      type;
    mutable unsigned    texunitInited:1;
    unsigned            texunitCount;
};


//==============================================================================
} // namespace rhi
#endif // __RHI_PROGGLES2_HPP__

