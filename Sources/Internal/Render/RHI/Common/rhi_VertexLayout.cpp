#include "../rhi_Type.h"

    #include "Debug/DVAssert.h"
    #include "Base/BaseTypes.h"
using DAVA::uint32;
using DAVA::uint16;
using DAVA::uint8;
using DAVA::int8;
    #include "Logger/Logger.h"
using DAVA::Logger;
    #include "FileSystem/File.h"

namespace rhi
{
//==============================================================================

VertexLayout::VertexLayout()
    : _elem_count(0)
    , _stream_count(0)
{
}

//------------------------------------------------------------------------------

VertexLayout::~VertexLayout()
{
}

//------------------------------------------------------------------------------

void VertexLayout::Clear()
{
    _elem_count = 0;
    _stream_count = 0;
}

//------------------------------------------------------------------------------

void VertexLayout::AddStream(VertexDataFrequency freq)
{
    DVASSERT(_stream_count < MaxStreamCount);

    _stream[_stream_count].elem_count = 0;
    _stream[_stream_count].first_elem = _elem_count;
    _stream[_stream_count].freq = freq;
    _stream[_stream_count].__pad = 0;

    ++_stream_count;
}

//------------------------------------------------------------------------------

void VertexLayout::AddElement(VertexSemantics usage, unsigned usage_i, VertexDataType type, unsigned dimension)
{
    DVASSERT(_elem_count < MaxElemCount);
    Element* e = _elem + _elem_count;

    if (_stream_count == 0)
        AddStream(VDF_PER_VERTEX);

    e->usage = usage;
    e->usage_index = usage_i;
    e->data_type = type;
    e->data_count = dimension;

    ++_elem_count;
    ++_stream[_stream_count - 1].elem_count;
}

//------------------------------------------------------------------------------
/*
void
VertexLayout::insert_elem( unsigned pos, VertexSemantics usage, unsigned usage_i, VertexDataType type, unsigned dimension )
{
    Element*    e = _elem.insert( pos );

    e->usage        = usage;
    e->usage_index  = usage_i;
    e->data_type    = type;
    e->data_count   = dimension;
}
*/

//------------------------------------------------------------------------------

unsigned
VertexLayout::Stride(unsigned stream_i) const
{
    unsigned sz = 0;

    for (unsigned e = _stream[stream_i].first_elem; e != _stream[stream_i].first_elem + _stream[stream_i].elem_count; ++e)
        sz += ElementSize(e);

    return sz;
}

//------------------------------------------------------------------------------

unsigned
VertexLayout::StreamCount() const
{
    return _stream_count;
}

//------------------------------------------------------------------------------

VertexDataFrequency
VertexLayout::StreamFrequency(unsigned stream_i) const
{
    DVASSERT(stream_i < _stream_count);
    return static_cast<VertexDataFrequency>(_stream[stream_i].freq);
}

//------------------------------------------------------------------------------

unsigned
VertexLayout::ElementCount() const
{
    return _elem_count;
}

//------------------------------------------------------------------------------

unsigned
VertexLayout::ElementStreamIndex(unsigned elem_i) const
{
    unsigned s = 0;

    for (; s != _stream_count; ++s)
    {
        if (elem_i >= _stream[s].first_elem && elem_i < _stream[s].first_elem + _stream[s].elem_count)
            break;
    }

    return s;
}

//------------------------------------------------------------------------------

VertexSemantics
VertexLayout::ElementSemantics(unsigned elem_i) const
{
    return static_cast<VertexSemantics>(_elem[elem_i].usage);
}

//------------------------------------------------------------------------------

unsigned
VertexLayout::ElementSemanticsIndex(unsigned elem_i) const
{
    return static_cast<VertexSemantics>(_elem[elem_i].usage_index);
}

//------------------------------------------------------------------------------

VertexDataType
VertexLayout::ElementDataType(unsigned elem_i) const
{
    return static_cast<VertexDataType>(_elem[elem_i].data_type);
}

//------------------------------------------------------------------------------

unsigned
VertexLayout::ElementDataCount(unsigned elem_i) const
{
    return _elem[elem_i].data_count;
}

//------------------------------------------------------------------------------

unsigned
VertexLayout::ElementOffset(unsigned elem_i) const
{
    unsigned off = 0;
    unsigned s = ElementStreamIndex(elem_i);

    for (unsigned e = _stream[s].first_elem; e < elem_i; ++e)
        off += ElementSize(e);

    return off;
}

//------------------------------------------------------------------------------

unsigned
VertexLayout::ElementSize(unsigned elem_i) const
{
    const Element& e = _elem[elem_i];

    switch (e.data_type)
    {
    case VDT_FLOAT:
        return sizeof(float) * e.data_count;
    case VDT_HALF:
        return sizeof(uint16) * e.data_count;
    case VDT_INT8N:
        return sizeof(int8) * e.data_count;
    case VDT_UINT8N:
        return sizeof(uint8) * e.data_count;
    case VDT_UINT8:
        return sizeof(uint8) * e.data_count;
    case VDT_INT16N:
        return sizeof(uint16) * e.data_count;
    }

    return 0;
}

//------------------------------------------------------------------------------

void VertexLayout::Dump() const
{
    for (unsigned s = 0; s != _stream_count; ++s)
    {
        Logger::Info("stream[%u]  stride= %u", s, Stride(s));
        for (unsigned a = _stream[s].first_elem, a_end = _stream[s].first_elem + _stream[s].elem_count; a != a_end; ++a)
        {
            Logger::Info(
            "  [%u] +%02u  %s%u  %s x%u",
            a, ElementOffset(a),
            VertexSemanticsName(VertexSemantics(_elem[a].usage)), _elem[a].usage_index,
            VertexDataTypeName(VertexDataType(_elem[a].data_type)), _elem[a].data_count);
        }
    }
}

//------------------------------------------------------------------------------

bool VertexLayout::operator==(const VertexLayout& vl) const
{
    return (this->_elem_count == vl._elem_count && this->_stream_count == vl._stream_count) ? (memcmp(_elem, vl._elem, _elem_count * sizeof(Element)) == 0 && memcmp(_stream, vl._stream, _stream_count * sizeof(Stream)) == 0) : false;
}

//------------------------------------------------------------------------------

VertexLayout&
VertexLayout::operator=(const VertexLayout& src)
{
    this->_elem_count = src._elem_count;

    for (unsigned e = 0; e != _elem_count; ++e)
        this->_elem[e] = src._elem[e];

    this->_stream_count = src._stream_count;

    for (unsigned s = 0; s != _stream_count; ++s)
        this->_stream[s] = src._stream[s];

    return *this;
}

//==============================================================================

struct
VertexLayoutInfo
{
    uint32 uid;
    VertexLayout layout;
};

static std::vector<VertexLayoutInfo> UniqueVertexLayout;
static uint32 LastUID = 0;

//------------------------------------------------------------------------------

const VertexLayout*
VertexLayout::Get(uint32 uid)
{
    const VertexLayout* layout = nullptr;

    for (std::vector<VertexLayoutInfo>::iterator i = UniqueVertexLayout.begin(), i_end = UniqueVertexLayout.end(); i != i_end; ++i)
    {
        if (i->uid == uid)
        {
            layout = &(i->layout); // CRAP: returning pointer to data inside std::vector
            break;
        }
    }

    return layout;
}

//------------------------------------------------------------------------------

uint32
VertexLayout::UniqueId(const VertexLayout& layout)
{
    uint32 uid = InvalidUID;

    for (std::vector<VertexLayoutInfo>::iterator i = UniqueVertexLayout.begin(), i_end = UniqueVertexLayout.end(); i != i_end; ++i)
    {
        if (i->layout == layout)
        {
            uid = i->uid;
            break;
        }
    }

    if (uid == InvalidUID)
    {
        VertexLayoutInfo info;

        info.uid = ++LastUID;
        info.layout = layout;

        UniqueVertexLayout.push_back(info);
        uid = info.uid;
    }

    return uid;
}

//------------------------------------------------------------------------------

bool VertexLayout::IsCompatible(const VertexLayout& vbLayout, const VertexLayout& shaderLayout)
{
    bool usable = true;

    for (unsigned s = 0; s != shaderLayout.ElementCount(); ++s)
    {
        DVASSERT(shaderLayout.ElementSemantics(s) != VS_PAD);

        bool hasAttr = false;

        for (unsigned v = 0; v != vbLayout.ElementCount(); ++v)
        {
            if (vbLayout.ElementSemantics(v) == shaderLayout.ElementSemantics(s) && vbLayout.ElementSemanticsIndex(v) == shaderLayout.ElementSemanticsIndex(s))
            {
                hasAttr = true;
                break;
            }
        }

        if (!hasAttr)
        {
            usable = false;
            break;
        }
    }

    return usable;
}

//------------------------------------------------------------------------------

bool VertexLayout::MakeCompatible(const VertexLayout& vbLayout, const VertexLayout& shaderLayout, VertexLayout* compatibleLayout)
{
    bool success = false;

    if (IsCompatible(vbLayout, shaderLayout))
    {
        uint32 pad_i = 0;

        compatibleLayout->Clear();

        unsigned last_stream_i = 0;
        for (unsigned v = 0; v != vbLayout.ElementCount(); ++v)
        {
            bool do_pad = true;

            for (unsigned s = 0; s != shaderLayout.ElementCount(); ++s)
            {
                if (vbLayout.ElementSemantics(v) == shaderLayout.ElementSemantics(s) && vbLayout.ElementSemanticsIndex(v) == shaderLayout.ElementSemanticsIndex(s))
                {
                    do_pad = false;
                    break;
                }
            }

            if (last_stream_i != vbLayout.ElementStreamIndex(v))
            {
                last_stream_i = vbLayout.ElementStreamIndex(v);
                compatibleLayout->AddStream(vbLayout.StreamFrequency(last_stream_i));
            }

            if (do_pad)
            {
                compatibleLayout->AddElement(VS_PAD, pad_i, VDT_UINT8, vbLayout.ElementSize(v));
                ++pad_i;
            }
            else
            {
                compatibleLayout->AddElement(vbLayout.ElementSemantics(v), vbLayout.ElementSemanticsIndex(v), vbLayout.ElementDataType(v), vbLayout.ElementDataCount(v));
            }
        }

        success = true;
    }

    return success;
}

//------------------------------------------------------------------------------

bool VertexLayout::Save(DAVA::File* out) const
{
#define WRITE_CHECK(exp) if (!(exp)) { return false; }

    WRITE_CHECK(out->Write(&_elem_count) == sizeof(_elem_count));
    WRITE_CHECK(out->Write(_elem, _elem_count * sizeof(Element)) == _elem_count * sizeof(Element));

    WRITE_CHECK(out->Write(&_stream_count) == sizeof(_stream_count));
    WRITE_CHECK(out->Write(_stream, _stream_count * sizeof(Stream)) == _stream_count * sizeof(Stream));
    
#undef WRITE_CHECK

    return true;
}

//------------------------------------------------------------------------------

bool VertexLayout::Load(DAVA::File* in)
{
#define READ_CHECK(exp) if (!(exp)) { return false; }

    READ_CHECK(in->Read(&_elem_count) == sizeof(_elem_count));
    READ_CHECK(in->Read(&_elem, _elem_count * sizeof(Element)) == _elem_count * sizeof(Element));

    READ_CHECK(in->Read(&_stream_count) == sizeof(_stream_count));
    READ_CHECK(in->Read(&_stream, _stream_count * sizeof(Stream)) == _stream_count * sizeof(Stream));
    
#undef READ_CHECK

    return true;
}

//==============================================================================
} // namespace rhi
