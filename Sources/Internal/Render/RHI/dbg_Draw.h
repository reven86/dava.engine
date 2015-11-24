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


#if !defined __DBG_DRAW_H__
#define __DBG_DRAW_H__
//==============================================================================
//
//  externals:

    #include "rhi_Public.h"

//==============================================================================
//
//  forward decls:

namespace DAVA
{
//==============================================================================
//
//  publics:

class
DbgDraw
{
public:
    DbgDraw();
    ~DbgDraw();

    static void EnsureInited();
    static void Uninitialize();

    static void SetScreenSize(uint32 w, uint32 h);
    static void FlushBatched(rhi::HPacketList batchBuf, const Matrix4& view, const Matrix4& projection);
    static void SetNormalTextSize();
    static void SetSmallTextSize();

    static unsigned Text2D(int x, int y, uint32 color, const char* format, ...);
    static unsigned MultilineText2D(int x, int y, uint32 color, const char* format, ...);

    static void Line2D(int x1, int y1, int x2, int y2, uint32 color);
    static void Rect2D(int left, int top, int right, int bottom, uint32 color);
    static void FilledRect2D(int left, int top, int right, int bottom, uint32 color);

public:
    enum
    {
        MaxTextLength = 256,
        NormalCharW = 7,
        NormalCharH = 13,
        SmallCharW = 5,
        SmallCharH = 11
    };

private:
    static DbgDraw* Instance();

    struct Vertex_PC;
    struct Vertex_PTC;

    void _init();
    void _uninit();
    Vertex_PC* _alloc_pc_vertices(unsigned count);

    struct
    Vertex_PC
    {
        float x, y, z;
        uint32 color;

        enum
        {
            Format = 1
        };
    };

    struct
    Vertex_PTC
    {
        float x, y, z;
        float u, v;
        uint32 color;

        enum
        {
            Format = 2
        };
    };

    class
    BufferBase
    {
    public:
        void set_small_text_size();
        void set_normal_text_size();

    protected:
        enum DrawType
        {
            draw3D = 1,
            draw2D = 2
        };

        BufferBase()
            : _small_text(false)
        {
        }
        virtual ~BufferBase()
        {
        }

        uint32 _small_text : 1;
    };

    template <typename Vertex, rhi::PrimitiveType Prim>
    class
    Buffer
    : public BufferBase
    {
    public:
        Buffer(const char* const name = "");
        ~Buffer();

        bool construct(unsigned max_vertex_count);
        void destroy();

        Vertex* alloc_vertices(unsigned count);
        void flush_3d(rhi::Handle cmd_buf, const Matrix4& view, const Matrix4& projection);
        void flush_2d(rhi::Handle cmd_buf);
        void flush_batched_2d(rhi::HPacketList packetList);

    private:
        enum
        {
            VBCount = 2
        };

        void _grow();

        unsigned _prim_count(unsigned v_cnt) const;

        rhi::HVertexBuffer _vb[VBCount];
        unsigned _cur_vb_i;

        unsigned _vb_size;
        Vertex* _cur_v;
        Vertex* _end_v;
        unsigned _v_cnt;
        const char* const _name;
        uint32 _need_grow : 1;
        uint32 _grow_ttw : 3;
    };

    Buffer<Vertex_PC, rhi::PRIMITIVE_LINELIST> _line_buf1;
    Buffer<Vertex_PC, rhi::PRIMITIVE_LINELIST> _line_buf2;
    Buffer<Vertex_PC, rhi::PRIMITIVE_TRIANGLELIST> _tri3d_buf;
    Buffer<Vertex_PC, rhi::PRIMITIVE_TRIANGLELIST> _tri2d_buf;
    Buffer<Vertex_PTC, rhi::PRIMITIVE_TRIANGLELIST> _normal_text2d_buf;
    Buffer<Vertex_PTC, rhi::PRIMITIVE_TRIANGLELIST> _small_text2d_buf;
    Buffer<Vertex_PC, rhi::PRIMITIVE_LINELIST> _line2d_buf;

    uint32 _inited : 1;
    uint32 _no_depth_test : 1;
    uint32 _small_text : 1;

    // wargning : is not used
    // uint32          _draw_immediately:1;
    uint32 _wnd_w;
    uint32 _wnd_h;

    enum
    {
        FontTextureSize = 128
    };

    bool _permanent_text_small;

    rhi::HPipelineState _ptc_pipeline_state;
    rhi::HConstBuffer _ptc_const;
    rhi::HPipelineState _pc_pipeline_state;
    rhi::HDepthStencilState _ptc_depth_state;
    rhi::HSamplerState _ptc_sampler_state;
    rhi::HConstBuffer _pc_const;
    rhi::HTexture _tex_small_font;
    rhi::HTexture _tex_normal_font;
    rhi::HTextureSet _texset_small_font;
    rhi::HTextureSet _texset_normal_font;
};
}

//==============================================================================
#endif // __DBG_DRAW_H__
