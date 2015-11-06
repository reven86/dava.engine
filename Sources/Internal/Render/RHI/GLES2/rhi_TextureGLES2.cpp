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
    #include "../Common/rhi_FormatConversion.h"
    #include "rhi_GLES2.h"

    #include "Debug/DVAssert.h"
    #include "Debug/Profiler.h"
    #include "FileSystem/Logger.h"
using DAVA::Logger;

    #include "_gl.h"

    #include <string.h>

namespace rhi
{
//==============================================================================

class
TextureGLES2_t
: public ResourceImpl<TextureGLES2_t, Texture::Descriptor>
{
public:
    TextureGLES2_t();

    bool Create(const Texture::Descriptor& desc, bool force_immediate = false);
    void Destroy(bool force_immediate = false);

    unsigned uid;
    unsigned uid2;

    unsigned width;
    unsigned height;
    TextureFormat format;
    void* mappedData;
    uint32 mappedLevel;
    GLenum mappedFace;
    uint32 isCubeMap : 1;
    uint32 isRenderTarget : 1;
    uint32 isRenderBuffer : 1;
    uint32 isMapped : 1;

    struct
    fbo_t
    {
        Handle color;
        Handle depthStencil;

        GLuint frameBuffer;
    };
    std::vector<fbo_t> fbo;

    SamplerState::Descriptor::Sampler samplerState;
    uint32 forceSetSamplerState : 1;
};
RHI_IMPL_RESOURCE(TextureGLES2_t, Texture::Descriptor);

TextureGLES2_t::TextureGLES2_t()
    : uid(0)
    , uid2(0)
    , fbo(0)
    , width(0)
    , height(0)
    , isCubeMap(false)
    , isRenderTarget(false)
    , isRenderBuffer(false)
    , mappedData(nullptr)
    , isMapped(false)
    , forceSetSamplerState(true)
{
}

//------------------------------------------------------------------------------

bool TextureGLES2_t::Create(const Texture::Descriptor& desc, bool force_immediate)
{
    DVASSERT(desc.levelCount);

    bool success = false;
    GLuint uid[2] = { 0, 0 };
    bool is_depth = desc.format == TEXTURE_FORMAT_D16 || desc.format == TEXTURE_FORMAT_D24S8;
    //    bool        need_stencil = desc.format == TEXTURE_FORMAT_D24S8;

    if (is_depth)
    {
        GLCommand cmd1 = { GLCommand::GEN_RENDERBUFFERS, { 1, (uint64)(uid) } };

        ExecGL(&cmd1, 1);

        if (cmd1.status == GL_NO_ERROR)
        {
            if (_GLES2_IsGlDepth24Stencil8Supported)
            {
                GLCommand d24s8cmd[] =
                {
                  { GLCommand::BIND_RENDERBUFFER, { GL_RENDERBUFFER, uid[0] } },
                  { GLCommand::RENDERBUFFER_STORAGE, { GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, desc.width, desc.height } },
                  { GLCommand::BIND_RENDERBUFFER, { GL_RENDERBUFFER, 0 } }
                };
                ExecGL(d24s8cmd, countof(d24s8cmd), force_immediate);

                // Store depth/stencil buffer index as secondary stencil index for iOS/Android
                uid[1] = uid[0];
            }
#if defined(__DAVAENGINE_ANDROID__)
            else if (_GLES2_IsGlDepthNvNonLinearSupported)
            {
                GLCommand d16s8nvcmd[] =
                {
                  { GLCommand::BIND_RENDERBUFFER, { GL_RENDERBUFFER, uid[0] } },
                  { GLCommand::RENDERBUFFER_STORAGE, { GL_RENDERBUFFER, GL_DEPTH_COMPONENT16_NONLINEAR_NV, desc.width, desc.height } },
                  { GLCommand::BIND_RENDERBUFFER, { GL_RENDERBUFFER, 0 } },
                  { GLCommand::BIND_RENDERBUFFER, { GL_RENDERBUFFER, uid[1] } },
                  { GLCommand::RENDERBUFFER_STORAGE, { GL_RENDERBUFFER, GL_STENCIL_INDEX8, desc.width, desc.height } },
                  { GLCommand::BIND_RENDERBUFFER, { GL_RENDERBUFFER, 0 } }
                };
                ExecGL(d16s8nvcmd, countof(d16s8nvcmd), force_immediate);
            }
#endif
            else
            {
                GLCommand d16s8cmd[] =
                {
                  { GLCommand::BIND_RENDERBUFFER, { GL_RENDERBUFFER, uid[0] } },
                  { GLCommand::RENDERBUFFER_STORAGE, { GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, desc.width, desc.height } },
                  { GLCommand::BIND_RENDERBUFFER, { GL_RENDERBUFFER, 0 } },
                  { GLCommand::BIND_RENDERBUFFER, { GL_RENDERBUFFER, uid[1] } },
                  { GLCommand::RENDERBUFFER_STORAGE, { GL_RENDERBUFFER, GL_STENCIL_INDEX8, desc.width, desc.height } },
                  { GLCommand::BIND_RENDERBUFFER, { GL_RENDERBUFFER, 0 } }
                };
                ExecGL(d16s8cmd, countof(d16s8cmd), force_immediate);
            }

            /*
            if( !need_stencil )
            {
                cmd2[3].func = GLCommand::NOP;
                cmd2[4].func = GLCommand::NOP;
                cmd2[5].func = GLCommand::NOP;
            }
*/
        }
    }
    else
    {
        GLCommand cmd1 = { GLCommand::GEN_TEXTURES, { 1, (uint64)(uid) } };

        ExecGL(&cmd1, 1, force_immediate);

        if (cmd1.status == GL_NO_ERROR)
        {
            GLenum target = (desc.type == TEXTURE_TYPE_CUBE) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
            uint32 cmd2_cnt = 2;
            GLCommand cmd2[4 + countof(desc.initialData)] =
            {
              { GLCommand::SET_ACTIVE_TEXTURE, { GL_TEXTURE0 + 0 } } //,
              //                { GLCommand::BIND_TEXTURE, { target, uid[0] } }
            };

            if (desc.autoGenMipmaps && !desc.isRenderTarget)
            {
                cmd2[3].func = GLCommand::GENERATE_MIPMAP;
                ++cmd2_cnt;
            }

            // process initial-data, if any
            //            if( desc.type == TEXTURE_TYPE_CUBE )
            {
                uint32 array_sz = (desc.type == TEXTURE_TYPE_CUBE) ? 6 : 1;
                GLenum face[] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };
                GLint int_fmt;
                GLint fmt;
                GLenum type;
                bool compressed;

                GetGLTextureFormat(desc.format, &int_fmt, &fmt, &type, &compressed);

                DVASSERT(desc.levelCount <= countof(desc.initialData));
                for (unsigned s = 0; s != array_sz; ++s)
                {
                    cmd2[cmd2_cnt].func = GLCommand::BIND_TEXTURE;
                    cmd2[cmd2_cnt].arg[0] = (desc.type == TEXTURE_TYPE_CUBE) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
                    cmd2[cmd2_cnt].arg[1] = uint64(&(uid[0]));
                    ++cmd2_cnt;

                    target = (desc.type == TEXTURE_TYPE_CUBE) ? face[s] : GL_TEXTURE_2D;

                    for (unsigned m = 0; m != desc.levelCount; ++m)
                    {
                        GLCommand* cmd = cmd2 + cmd2_cnt;
                        void* data = desc.initialData[s * desc.levelCount + m];

                        if (data)
                        {
                            Size2i sz = TextureExtents(Size2i(desc.width, desc.height), m);
                            uint32 data_sz = TextureSize(desc.format, sz.dx, sz.dy);

                            cmd->func = GLCommand::TEX_IMAGE2D;
                            cmd->arg[0] = target;
                            cmd->arg[1] = m;
                            cmd->arg[2] = uint64(int_fmt);
                            cmd->arg[3] = uint64(sz.dx);
                            cmd->arg[4] = uint64(sz.dy);
                            cmd->arg[5] = 0;
                            cmd->arg[6] = uint64(fmt);
                            cmd->arg[7] = type;
                            cmd->arg[8] = uint64(data_sz);
                            cmd->arg[9] = (uint64)(data);
                            cmd->arg[10] = compressed;

                            if (desc.format == TEXTURE_FORMAT_R4G4B4A4)
                                _FlipRGBA4_ABGR4(desc.initialData[m], data_sz);
                            else if (desc.format == TEXTURE_FORMAT_R5G5B5A1)
                                _RGBA5551toABGR1555(desc.initialData[m], data_sz);

                            ++cmd2_cnt;
                        }
                        else
                        {
                            break;
                        }
                    }
                }

                cmd2[cmd2_cnt].func = GLCommand::RESTORE_TEXTURE0;
                ++cmd2_cnt;
            }
            //            else
            //            {
            //                DVASSERT(desc.initialData[0]==nullptr);
            //            }

            ExecGL(cmd2, cmd2_cnt, force_immediate);
        }
    }

    if (uid[0])
    {
        this->uid = uid[0];
        this->uid2 = uid[1];
        mappedData = nullptr;
        width = desc.width;
        height = desc.height;
        format = desc.format;
        isCubeMap = desc.type == TEXTURE_TYPE_CUBE;
        isMapped = false;
        isRenderTarget = false;
        isRenderBuffer = is_depth;
        forceSetSamplerState = true;

        if (desc.isRenderTarget)
        {
            isRenderTarget = true;
            forceSetSamplerState = false;

            GLint int_fmt, fmt;
            GLenum type;
            bool compressed;

            GetGLTextureFormat(format, &int_fmt, &fmt, &type, &compressed);
            DVASSERT(!compressed);

            GLCommand cmd3[] =
            {
              { GLCommand::BIND_TEXTURE, { GL_TEXTURE_2D, uint64(&(this->uid)) } },
              { GLCommand::TEX_IMAGE2D, { GL_TEXTURE_2D, 0, uint64(int_fmt), uint64(desc.width), uint64(desc.height), 0, uint64(fmt), type, 0, 0, 0 } },
              { GLCommand::TEX_PARAMETER_I, { GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST } },
              { GLCommand::TEX_PARAMETER_I, { GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST } },
              { GLCommand::RESTORE_TEXTURE0, {} }
            };

            ExecGL(cmd3, countof(cmd3), force_immediate);
        }
        else
        {
            isRenderTarget = false;
        }

        success = true;
    }

    return success;
}

//------------------------------------------------------------------------------

void TextureGLES2_t::Destroy(bool force_immediate)
{
    GLCommand cmd[16];
    unsigned cmd_cnt = 1;

    if (isRenderTarget)
    {
        cmd[0].func = GLCommand::DELETE_TEXTURES;
        cmd[0].arg[0] = 1;
        cmd[0].arg[1] = uint64(&(uid));

        DVASSERT(fbo.size() <= countof(cmd) - 1);
        for (unsigned i = 0; i != fbo.size(); ++i)
        {
            cmd[1 + i].func = GLCommand::DELETE_FRAMEBUFFERS;
            cmd[1 + i].arg[0] = 1;
            cmd[1 + i].arg[1] = uint64(&(fbo[i].frameBuffer));
        }

        cmd_cnt += fbo.size();
    }
    else if (isRenderBuffer)
    {
        cmd[0].func = GLCommand::DELETE_RENDERBUFFERS;
        cmd[0].arg[0] = 1;
        cmd[0].arg[1] = uint64(&(uid));

        if (uid2)
        {
            cmd[1].func = GLCommand::DELETE_RENDERBUFFERS;
            cmd[1].arg[0] = 1;
            cmd[1].arg[1] = uint64(&(uid2));
        }
    }
    else
    {
        cmd[0].func = GLCommand::DELETE_TEXTURES;
        cmd[0].arg[0] = 1;
        cmd[0].arg[1] = uint64(&(uid));
    }

    ExecGL(cmd, cmd_cnt, force_immediate);

    fbo.clear();

    DVASSERT(!isMapped);

    if (mappedData)
    {
        ::free(mappedData);

        mappedData = nullptr;
        width = 0;
        height = 0;
    }
}

typedef ResourcePool<TextureGLES2_t, RESOURCE_TEXTURE, Texture::Descriptor, true> TextureGLES2Pool;
RHI_IMPL_POOL(TextureGLES2_t, RESOURCE_TEXTURE, Texture::Descriptor, true);

//------------------------------------------------------------------------------

static void
gles2_Texture_Delete(Handle tex)
{
    if (tex != InvalidHandle)
    {
        TextureGLES2_t* self = TextureGLES2Pool::Get(tex);

        self->MarkRestored();
        self->Destroy();
        TextureGLES2Pool::Free(tex);
    }
}

//------------------------------------------------------------------------------

static Handle
gles2_Texture_Create(const Texture::Descriptor& desc)
{
    Handle handle = TextureGLES2Pool::Alloc();
    TextureGLES2_t* tex = TextureGLES2Pool::Get(handle);

    if (tex->Create(desc))
    {
        Texture::Descriptor creationDesc(desc);
        Memset(creationDesc.initialData, 0, sizeof(creationDesc.initialData));
        tex->UpdateCreationDesc(creationDesc);
    }
    else
    {
        TextureGLES2Pool::Free(handle);
        handle = InvalidHandle;
    }

    return handle;
}

//------------------------------------------------------------------------------

static void*
gles2_Texture_Map(Handle tex, unsigned level, TextureFace face)
{
    TextureGLES2_t* self = TextureGLES2Pool::Get(tex);
    void* data = nullptr;
    uint32 data_sz = TextureSize(self->format, self->width, self->height, level);

    DVASSERT(!self->isRenderBuffer);
    DVASSERT(!self->isMapped);

    self->mappedData = ::realloc(self->mappedData, data_sz);

    if (self->mappedData)
    {
        if (self->isRenderTarget)
        {
            DVASSERT(level == 0);
            DVASSERT(self->fbo.size())
            GLenum target = (self->isCubeMap) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
            GLCommand cmd[] =
            {
              { GLCommand::BIND_FRAMEBUFFER, { GL_FRAMEBUFFER, self->fbo[0].frameBuffer } },
              { GLCommand::READ_PIXELS, { 0, 0, self->width, self->height, GL_RGBA, GL_UNSIGNED_BYTE, uint64(self->mappedData) } },
              { GLCommand::BIND_FRAMEBUFFER, { GL_FRAMEBUFFER, _GLES2_Binded_FrameBuffer } },
            };

            ExecGL(cmd, countof(cmd));

            data = self->mappedData;
            self->mappedLevel = 0;
            self->isMapped = true;
        }
        else
        {
            data = self->mappedData;
            self->mappedLevel = level;
            self->isMapped = true;

            switch (face)
            {
            case TEXTURE_FACE_POSITIVE_X:
                self->mappedFace = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
                break;
            case TEXTURE_FACE_NEGATIVE_X:
                self->mappedFace = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
                break;
            case TEXTURE_FACE_POSITIVE_Y:
                self->mappedFace = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
                break;
            case TEXTURE_FACE_NEGATIVE_Y:
                self->mappedFace = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
                break;
            case TEXTURE_FACE_POSITIVE_Z:
                self->mappedFace = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
                break;
            case TEXTURE_FACE_NEGATIVE_Z:
                self->mappedFace = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
                break;
            }
        }
    }

    if (self->format == TEXTURE_FORMAT_R4G4B4A4)
    {
        _FlipRGBA4_ABGR4(self->mappedData, data_sz);
    }
    else if (self->format == TEXTURE_FORMAT_R5G5B5A1)
    {
        _ABGR1555toRGBA5551(self->mappedData, data_sz);
    }

    return data;
}

//------------------------------------------------------------------------------

static void
gles2_Texture_Unmap(Handle tex)
{
    TextureGLES2_t* self = TextureGLES2Pool::Get(tex);
    Size2i sz = TextureExtents(Size2i(self->width, self->height), self->mappedLevel);
    GLint int_fmt;
    GLint fmt;
    GLenum type;
    bool compressed;

    uint32 textureDataSize = TextureSize(self->format, sz.dx, sz.dy);
    GetGLTextureFormat(self->format, &int_fmt, &fmt, &type, &compressed);

    DVASSERT(self->isMapped);
    if (self->format == TEXTURE_FORMAT_R4G4B4A4)
    {
        _FlipRGBA4_ABGR4(self->mappedData, textureDataSize);
    }
    else if (self->format == TEXTURE_FORMAT_R5G5B5A1)
    {
        _RGBA5551toABGR1555(self->mappedData, textureDataSize);
    }

    GLenum ttarget = (self->isCubeMap) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    GLenum target = GL_TEXTURE_2D;

    if (self->isCubeMap)
    {
        switch (self->mappedFace)
        {
        case TEXTURE_FACE_POSITIVE_X:
            target = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
            break;
        case TEXTURE_FACE_NEGATIVE_X:
            target = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
            break;
        case TEXTURE_FACE_POSITIVE_Y:
            target = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
            break;
        case TEXTURE_FACE_NEGATIVE_Y:
            target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
            break;
        case TEXTURE_FACE_POSITIVE_Z:
            target = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
            break;
        case TEXTURE_FACE_NEGATIVE_Z:
            target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
            break;
        }
    }

    GLCommand cmd[] =
    {
      { GLCommand::SET_ACTIVE_TEXTURE, { GL_TEXTURE0 + 0 } },
      { GLCommand::BIND_TEXTURE, { ttarget, uint64(&(self->uid)) } },
      { GLCommand::TEX_IMAGE2D, { target, self->mappedLevel, uint64(int_fmt), uint64(sz.dx), uint64(sz.dy), 0, uint64(fmt), type, uint64(textureDataSize), (uint64)(self->mappedData), compressed } },
      { GLCommand::RESTORE_TEXTURE0, {} }
    };

    ExecGL(cmd, countof(cmd));

    self->isMapped = false;
    ::free(self->mappedData);
    self->mappedData = nullptr;
}

//------------------------------------------------------------------------------

void gles2_Texture_Update(Handle tex, const void* data, uint32 level, TextureFace face)
{
    TextureGLES2_t* self = TextureGLES2Pool::Get(tex);
    Size2i sz = TextureExtents(Size2i(self->width, self->height), level);
    GLint int_fmt;
    GLint fmt;
    GLenum type;
    bool compressed;
    uint32 textureDataSize = TextureSize(self->format, sz.dx, sz.dy);

    DVASSERT(!self->isRenderBuffer);
    DVASSERT(!self->isMapped);

    GetGLTextureFormat(self->format, &int_fmt, &fmt, &type, &compressed);

    if (self->format == TEXTURE_FORMAT_R4G4B4A4 || self->format == TEXTURE_FORMAT_R5G5B5A1)
    {
        gles2_Texture_Map(tex, level, face);
        memcpy(self->mappedData, data, textureDataSize);
        gles2_Texture_Unmap(tex);
    }
    else
    {
        GLenum ttarget = (self->isCubeMap) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
        GLenum target = GL_TEXTURE_2D;

        if (self->isCubeMap)
        {
            switch (face)
            {
            case TEXTURE_FACE_POSITIVE_X:
                target = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
                break;
            case TEXTURE_FACE_NEGATIVE_X:
                target = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
                break;
            case TEXTURE_FACE_POSITIVE_Y:
                target = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
                break;
            case TEXTURE_FACE_NEGATIVE_Y:
                target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
                break;
            case TEXTURE_FACE_POSITIVE_Z:
                target = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
                break;
            case TEXTURE_FACE_NEGATIVE_Z:
                target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
                break;
            }
        }

        GLCommand cmd[] =
        {
          { GLCommand::SET_ACTIVE_TEXTURE, { GL_TEXTURE0 + 0 } },
          { GLCommand::BIND_TEXTURE, { ttarget, uint64(&(self->uid)) } },
          { GLCommand::TEX_IMAGE2D, { target, uint64(level), uint64(int_fmt), uint64(sz.dx), uint64(sz.dy), 0, uint64(fmt), type, uint64(textureDataSize), (uint64)(data), compressed } },
          { GLCommand::RESTORE_TEXTURE0, {} }
        };

        ExecGL(cmd, countof(cmd));
    }
}

//------------------------------------------------------------------------------

static bool
gles2_Texture_NeedRestore(Handle tex)
{
    TextureGLES2_t* self = TextureGLES2Pool::Get(tex);

    return self->NeedRestore();
}

//==============================================================================

struct
SamplerStateGLES2_t
{
    SamplerState::Descriptor::Sampler fragmentSampler[MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT];
    uint32 fragmentSamplerCount;
    SamplerState::Descriptor::Sampler vertexSampler[MAX_VERTEX_TEXTURE_SAMPLER_COUNT];
    uint32 vertexSamplerCount;
};

typedef ResourcePool<SamplerStateGLES2_t, RESOURCE_SAMPLER_STATE, SamplerState::Descriptor, false> SamplerStateGLES2Pool;
RHI_IMPL_POOL(SamplerStateGLES2_t, RESOURCE_SAMPLER_STATE, SamplerState::Descriptor, false);
static const SamplerStateGLES2_t* _CurSamplerState = nullptr;

//------------------------------------------------------------------------------

static Handle
gles2_SamplerState_Create(const SamplerState::Descriptor& desc)
{
    Handle handle = SamplerStateGLES2Pool::Alloc();
    SamplerStateGLES2_t* state = SamplerStateGLES2Pool::Get(handle);

    state->fragmentSamplerCount = desc.fragmentSamplerCount;
    for (unsigned i = 0; i != desc.fragmentSamplerCount; ++i)
    {
        state->fragmentSampler[i] = desc.fragmentSampler[i];
    }

    state->vertexSamplerCount = desc.vertexSamplerCount;
    for (unsigned i = 0; i != desc.vertexSamplerCount; ++i)
    {
        state->vertexSampler[i] = desc.vertexSampler[i];
    }

    // force no-filtering on vertex-textures
    for (uint32 s = 0; s != MAX_VERTEX_TEXTURE_SAMPLER_COUNT; ++s)
    {
        state->vertexSampler[s].minFilter = TEXFILTER_NEAREST;
        state->vertexSampler[s].magFilter = TEXFILTER_NEAREST;
        state->vertexSampler[s].mipFilter = TEXMIPFILTER_NONE;
    }

    return handle;
}

//------------------------------------------------------------------------------

static void
gles2_SamplerState_Delete(Handle hstate)
{
    SamplerStateGLES2_t* state = SamplerStateGLES2Pool::Get(hstate);

    if (_CurSamplerState == state)
        _CurSamplerState = nullptr;

    SamplerStateGLES2Pool::Free(hstate);
}

//==============================================================================

namespace SamplerStateGLES2
{
void Init(uint32 maxCount)
{
    SamplerStateGLES2Pool::Reserve(maxCount);
}
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_SamplerState_Create = &gles2_SamplerState_Create;
    dispatch->impl_SamplerState_Delete = &gles2_SamplerState_Delete;
}

void SetToRHI(Handle hstate)
{
    SamplerStateGLES2_t* state = SamplerStateGLES2Pool::Get(hstate);

    _CurSamplerState = state;
}
}

//==============================================================================

static GLenum
_TextureFilter(TextureFilter filter)
{
    GLenum f = GL_LINEAR;

    switch (filter)
    {
    case TEXFILTER_NEAREST:
        f = GL_NEAREST;
        break;
    case TEXFILTER_LINEAR:
        f = GL_LINEAR;
        break;
    }

    return f;
}

//------------------------------------------------------------------------------

static GLenum
_TextureMipFilter(TextureMipFilter filter)
{
    GLenum f = GL_LINEAR_MIPMAP_LINEAR;

    switch (filter)
    {
    case TEXMIPFILTER_NONE:
        f = GL_NEAREST_MIPMAP_NEAREST;
        break;
    case TEXMIPFILTER_NEAREST:
        f = GL_LINEAR_MIPMAP_NEAREST;
        break;
    case TEXMIPFILTER_LINEAR:
        f = GL_LINEAR_MIPMAP_LINEAR;
        break;
    }

    return f;
}

//------------------------------------------------------------------------------

static GLenum
_AddrMode(TextureAddrMode mode)
{
    GLenum m = GL_REPEAT;

    switch (mode)
    {
    case TEXADDR_WRAP:
        m = GL_REPEAT;
        break;
    case TEXADDR_CLAMP:
        m = GL_CLAMP_TO_EDGE;
        break;
    case TEXADDR_MIRROR:
        m = GL_MIRRORED_REPEAT;
        break;
    }

    return m;
}

//==============================================================================

namespace TextureGLES2
{
void Init(uint32 maxCount)
{
    TextureGLES2Pool::Reserve(maxCount);
}

void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_Texture_Create = &gles2_Texture_Create;
    dispatch->impl_Texture_Delete = &gles2_Texture_Delete;
    dispatch->impl_Texture_Map = &gles2_Texture_Map;
    dispatch->impl_Texture_Unmap = &gles2_Texture_Unmap;
    dispatch->impl_Texture_Update = &gles2_Texture_Update;
    dispatch->impl_Texture_NeedRestore = &gles2_Texture_NeedRestore;
}

void SetToRHI(Handle tex, unsigned unit_i, uint32 base_i)
{
    TextureGLES2_t* self = TextureGLES2Pool::Get(tex);
    bool fragment = base_i != InvalidIndex;
    uint32 sampler_i = (base_i == InvalidIndex) ? unit_i : base_i + unit_i;
    GLenum target = (self->isCubeMap) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;

    const SamplerState::Descriptor::Sampler* sampler = (fragment) ? _CurSamplerState->fragmentSampler + unit_i : _CurSamplerState->vertexSampler + unit_i;

    if (_GLES2_LastActiveTexture != GL_TEXTURE0 + sampler_i)
    {
        GL_CALL(glActiveTexture(GL_TEXTURE0 + sampler_i));
        _GLES2_LastActiveTexture = GL_TEXTURE0 + sampler_i;
    }

    GL_CALL(glBindTexture(target, self->uid));
    //{SCOPED_NAMED_TIMING("gl-BindTexture");}

    if (sampler_i == 0)
    {
        _GLES2_LastSetTex0 = self->uid;
        _GLES2_LastSetTex0Target = target;
    }

    if (_CurSamplerState && (self->forceSetSamplerState || memcmp(&(self->samplerState), sampler, sizeof(rhi::SamplerState::Descriptor::Sampler))) && !self->isRenderBuffer)
    {
        if (sampler->mipFilter != TEXMIPFILTER_NONE)
        {
            GL_CALL(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, _TextureMipFilter(TextureMipFilter(sampler->mipFilter))));
        }
        else
        {
            GL_CALL(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, _TextureFilter(TextureFilter(sampler->minFilter))));
        }

        GL_CALL(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, _TextureFilter(TextureFilter(sampler->magFilter))));

        GL_CALL(glTexParameteri(target, GL_TEXTURE_WRAP_S, _AddrMode(TextureAddrMode(sampler->addrU))));
        GL_CALL(glTexParameteri(target, GL_TEXTURE_WRAP_T, _AddrMode(TextureAddrMode(sampler->addrV))));

        self->samplerState = *sampler;
        self->forceSetSamplerState = false;
    }
}

void SetAsRenderTarget(Handle tex, Handle depth)
{
    TextureGLES2_t* self = TextureGLES2Pool::Get(tex);
    GLuint fb = 0;
    DVASSERT(self->isRenderTarget || self->isRenderBuffer);
    for (unsigned i = 0; i != self->fbo.size(); ++i)
    {
        if (self->fbo[i].color == tex && self->fbo[i].depthStencil == depth)
        {
            fb = self->fbo[i].frameBuffer;
            break;
        }
    }

    if (!fb)
    {
        glGenFramebuffers(1, &fb);

        if (fb)
        {
            TextureGLES2_t* ds = (depth != InvalidHandle && depth != DefaultDepthBuffer) ? TextureGLES2Pool::Get(depth) : nullptr;

            glBindFramebuffer(GL_FRAMEBUFFER, fb);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self->uid, 0);
            if (ds)
            {
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ds->uid);
                if (ds->uid2)
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ds->uid2);
#else
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ds->uid);
#endif
            }
#if defined __DAVAENGINE_IPHONE__ || defined __DAVAENGINE_ANDROID__
#else
            GLenum b[1] = { GL_COLOR_ATTACHMENT0 };

            glDrawBuffers(1, b);
#endif

            int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            if (status == GL_FRAMEBUFFER_COMPLETE)
            {
                TextureGLES2_t::fbo_t fbo = { tex, depth, fb };

                self->fbo.push_back(fbo);
            }
            else
            {
                Logger::Error("glCheckFramebufferStatus= %08X", status);
                DVASSERT(status == GL_FRAMEBUFFER_COMPLETE);
            }
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    _GLES2_Binded_FrameBuffer = fb;
}

Size2i
Size(Handle tex)
{
    TextureGLES2_t* self = TextureGLES2Pool::Get(tex);

    return Size2i(self->width, self->height);
}

void ReCreateAll()
{
    TextureGLES2Pool::ReCreateAll();
}

unsigned
NeedRestoreCount()
{
    return TextureGLES2_t::NeedRestoreCount();
}

} // namespace TextureGLES2

//==============================================================================
} // namespace rhi
