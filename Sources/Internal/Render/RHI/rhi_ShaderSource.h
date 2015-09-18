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


#ifndef __RHI_SHADERSOURCE_H__
#define __RHI_SHADERSOURCE_H__

    #include "rhi_Type.h"
    #include "Base/BaseTypes.h"    
    #include "Base/FastName.h"    

namespace DAVA { class File; }


namespace rhi
{
    using DAVA::uint32;
    using DAVA::FastName;
struct 
ShaderProp 
{
    enum Type   { TYPE_FLOAT1, TYPE_FLOAT2, TYPE_FLOAT3, TYPE_FLOAT4, TYPE_FLOAT4X4 };
//    enum Scope  { SCOPE_UNIQUE, SCOPE_SHARED };
    enum Storage  { STORAGE_STATIC, STORAGE_DYNAMIC };

    FastName    uid;
    Type        type;
    uint32      arraySize;
//    Scope       scope;
    Storage     storage;
    FastName    tag;
    uint32      bufferindex;
    uint32      bufferReg;
    uint32      bufferRegCount;
    uint32      isBigArray:1;
    float       defaultValue[16];
};

typedef std::vector<ShaderProp> ShaderPropList;

struct
ShaderSampler
{
    TextureType type;
    FastName    uid;
};

typedef std::vector<ShaderSampler> ShaderSamplerList;


class
ShaderSource
{
public:
                            ShaderSource( const char* filename="" );
                            ~ShaderSource();

    bool                    Construct( ProgType progType, const char* srcText, const std::vector<std::string>& defines );
    bool                    Construct( ProgType progType, const char* srcText );
    bool                    Load( DAVA::File* in );
    bool                    Save( DAVA::File* out ) const;

    const char*             SourceCode() const;
    const ShaderPropList&   Properties() const;
    const ShaderSamplerList&Samplers() const;
    const VertexLayout&     ShaderVertexLayout() const;
    uint32                  ConstBufferCount() const;
    uint32                  ConstBufferSize( uint32 bufIndex ) const;
//    ShaderProp::Scope       ConstBufferScope( uint32 bufIndex ) const;
    ShaderProp::Storage     ConstBufferStorage( uint32 bufIndex ) const;
    BlendState              Blending() const;

    void                    Dump() const;


private:

    void                    _Reset();
    void                    _AppendLine( const char* line, uint32 lineLen );

    struct
    buf_t
    {
//        ShaderProp::Scope   scope;
        ShaderProp::Storage storage;
        FastName            tag;
        uint32              regCount;
        std::vector<int>    avlRegIndex;
    };

    std::string                 fileName;

    ProgType                    type;
    std::string                 code;
    uint32                      codeLineCount;
    VertexLayout                vdecl;
    std::vector<ShaderProp>     prop;
    std::vector<buf_t>          buf;
    std::vector<ShaderSampler>  sampler;
    BlendState                  blending;
};


class
ShaderSourceCache
{
public:

    static const ShaderSource*  Get( FastName uid );
    static void                 Update( FastName uid, const ShaderSource& source );

    static void                 Clear();    
    static void                 Save( const char* fileName );
    static void                 Load( const char* fileName );


private:

    struct
    entry_t
    {
        FastName        uid;
        ShaderSource*   src;
    };

    static std::vector<entry_t> Entry;
};


} // namespace rhi
#endif // __RHI_SHADERSOURCE_H__

