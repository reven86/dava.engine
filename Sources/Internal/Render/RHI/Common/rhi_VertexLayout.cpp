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

    #include "../rhi_Type.h"

    #include "Debug/DVAssert.h"
    #include "Base/BaseTypes.h"
    using DAVA::uint32;
    using DAVA::uint16;
    using DAVA::uint8;
    using DAVA::int8;
    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    #include "FileSystem/File.h"


namespace rhi
{
//==============================================================================

VertexLayout::VertexLayout()
  : _elem_count(0)
{
}


//------------------------------------------------------------------------------

VertexLayout::~VertexLayout()
{
}


//------------------------------------------------------------------------------

void
VertexLayout::Clear()
{
    _elem_count = 0;
}


//------------------------------------------------------------------------------

void
VertexLayout::AddElement( VertexSemantics usage, unsigned usage_i, VertexDataType type, unsigned dimension )
{
    DVASSERT(_elem_count < MaxElemCount);
    Element*    e = _elem + _elem_count;

    e->usage        = usage;
    e->usage_index  = usage_i;
    e->data_type    = type;
    e->data_count   = dimension;

    ++_elem_count;
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
VertexLayout::Stride() const
{
    unsigned    sz = 0;

    for( unsigned e=0; e!=_elem_count; ++e )
        sz += ElementSize( e );

    return sz;
}


//------------------------------------------------------------------------------

unsigned
VertexLayout::ElementCount() const
{
    return _elem_count;
}


//------------------------------------------------------------------------------

VertexSemantics
VertexLayout::ElementSemantics( unsigned elem_i ) const
{
    return (VertexSemantics)(_elem[elem_i].usage);
}


//------------------------------------------------------------------------------

unsigned
VertexLayout::ElementSemanticsIndex( unsigned elem_i ) const
{
    return (VertexSemantics)(_elem[elem_i].usage_index);
}


//------------------------------------------------------------------------------

VertexDataType
VertexLayout::ElementDataType( unsigned elem_i ) const
{
    return (VertexDataType)(_elem[elem_i].data_type);
}


//------------------------------------------------------------------------------

unsigned
VertexLayout::ElementDataCount( unsigned elem_i ) const
{
    return _elem[elem_i].data_count;
}


//------------------------------------------------------------------------------

unsigned
VertexLayout::ElementOffset( unsigned elem_i ) const
{
    unsigned    off = 0;

    for( unsigned e=0; e<elem_i; ++e )
        off += ElementSize( e );

    return off;
}


//------------------------------------------------------------------------------

unsigned
VertexLayout::ElementSize( unsigned elem_i ) const
{
    const Element&  e = _elem[elem_i];

    switch( e.data_type )
    {
        case VDT_FLOAT   : return sizeof(float) * e.data_count;
        case VDT_HALF    : return sizeof(uint16) * e.data_count;
        case VDT_INT8N   : return sizeof(int8) * e.data_count;
        case VDT_UINT8N  : return sizeof(uint8) * e.data_count;
        case VDT_UINT8   : return sizeof(uint8) * e.data_count;
        case VDT_INT16N  : return sizeof(uint16) * e.data_count;
    }

    return 0;
}


//------------------------------------------------------------------------------

void
VertexLayout::Dump() const
{
    for( unsigned e=0; e!=_elem_count; ++e )
    {
        Logger::Info
        ( 
            "[%u] +%02u  %s%u  %s x%u", 
            e, ElementOffset(e),
            VertexSemanticsName( VertexSemantics(_elem[e].usage) ), _elem[e].usage_index,
            VertexDataTypeName( VertexDataType(_elem[e].data_type)), _elem[e].data_count 
        );

    }
    Logger::Info( "stride = %u\n", Stride() );
}


//------------------------------------------------------------------------------

bool
VertexLayout::operator==( const VertexLayout& vl ) const
{
    return (this->_elem_count == vl._elem_count)
           ? !memcmp( _elem, vl._elem, _elem_count*sizeof(Element) )
           : false;
}


//------------------------------------------------------------------------------

VertexLayout&
VertexLayout::operator=( const VertexLayout& src )
{
    this->_elem_count = src._elem_count;
    
    for( unsigned e=0; e!=_elem_count; ++e )
        this->_elem[e] = src._elem[e];

    return *this;
}


//==============================================================================

struct
VertexLayoutInfo
{
    uint32          uid;
    VertexLayout    layout;
};

static std::vector<VertexLayoutInfo>    UniqueVertexLayout;
static uint32                           LastUID             = 0;


//------------------------------------------------------------------------------

const VertexLayout*  
VertexLayout::Get( uint32 uid )
{
    const VertexLayout* layout = nullptr;

    for( std::vector<VertexLayoutInfo>::iterator i=UniqueVertexLayout.begin(),i_end=UniqueVertexLayout.end(); i!=i_end; ++i )
    {
        if( i->uid == uid )
        {
            layout = &(i->layout); // CRAP: returning pointer to data inside std::vector
            break;
        }
    }

    return layout;
}


//------------------------------------------------------------------------------

uint32
VertexLayout::UniqueId( const VertexLayout& layout )
{
    uint32  uid = InvalidUID;

    for( std::vector<VertexLayoutInfo>::iterator i=UniqueVertexLayout.begin(),i_end=UniqueVertexLayout.end(); i!=i_end; ++i )
    {
        if( i->layout == layout )
        {
            uid = i->uid;
            break;
        }
    }

    if( uid == InvalidUID )
    {
        VertexLayoutInfo    info;

        info.uid    = ++LastUID;
        info.layout = layout;

        UniqueVertexLayout.push_back( info );
        uid = info.uid;
    }

    return uid;
}


//------------------------------------------------------------------------------

bool
VertexLayout::IsCompatible( const VertexLayout& vbLayout, const VertexLayout& shaderLayout )
{
    bool    usable = true;

    for( unsigned s=0; s!=shaderLayout.ElementCount(); ++s )
    {
        DVASSERT(shaderLayout.ElementSemantics(s) != VS_PAD);

        bool    hasAttr = false;

        for( unsigned v=0; v!=vbLayout.ElementCount(); ++v )
        {
            if(     vbLayout.ElementSemantics(v) == shaderLayout.ElementSemantics(s) 
                &&  vbLayout.ElementSemanticsIndex(v) == shaderLayout.ElementSemanticsIndex(s)
              )
            {
                hasAttr = true;
                break;
            }
        }

        if( !hasAttr )
        {
            usable = false;
            break;
        }
    }

    return usable;
}


//------------------------------------------------------------------------------

bool
VertexLayout::MakeCompatible( const VertexLayout& vbLayout, const VertexLayout& shaderLayout, VertexLayout* compatibleLayout )
{
    bool    success = false;

    if( IsCompatible( vbLayout, shaderLayout ) )
    {
        uint32  pad_i = 0;

        compatibleLayout->Clear();

        for( unsigned v=0; v!=vbLayout.ElementCount(); ++v )
        {
            bool    do_pad = true;

            for( unsigned s=0; s!=shaderLayout.ElementCount(); ++s )
            {
                if(     vbLayout.ElementSemantics(v) == shaderLayout.ElementSemantics(s) 
                    &&  vbLayout.ElementSemanticsIndex(v) == shaderLayout.ElementSemanticsIndex(s)
                  )
                {
                    do_pad = false;
                    break;
                }
            }
            
            if( do_pad )
            {
                compatibleLayout->AddElement( VS_PAD, pad_i, VDT_UINT8, vbLayout.ElementSize(v) );
                ++pad_i;
            }
            else
            {
                compatibleLayout->AddElement( vbLayout.ElementSemantics(v), vbLayout.ElementSemanticsIndex(v), vbLayout.ElementDataType(v), vbLayout.ElementDataCount(v) );
            }
        }

        success = true;
    }

    return success;
}


//------------------------------------------------------------------------------

void
VertexLayout::Save( DAVA::File* out ) const
{
    out->Write( &_elem_count );
    out->Write( _elem, _elem_count*sizeof(Element) );
}


//------------------------------------------------------------------------------

void
VertexLayout::Load( DAVA::File* in )
{
    in->Read( &_elem_count );
    in->Read( &_elem, _elem_count*sizeof(Element) );
}

//==============================================================================
} // namespace rhi

