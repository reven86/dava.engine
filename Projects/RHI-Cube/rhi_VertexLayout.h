#if !defined __RHI_VERTEXLAYOUT_HPP__
#define __RHI_VERTEXLAYOUT_HPP__

    #include "rhi_Type.h"



namespace rhi
{
//==============================================================================
//
//  publics:

class
VertexLayout
{
public:


                    VertexLayout();
                    ~VertexLayout();

    unsigned        Stride() const;
    unsigned        ElementCount() const;
    VertexSemantics ElementSemantics( unsigned elem_i ) const;
    VertexDataType  ElementDataType( unsigned elem_i ) const;
    unsigned        ElementDataCount( unsigned elem_i ) const;
    unsigned        ElementOffset( unsigned elem_i ) const;
    unsigned        ElementSize( unsigned elem_i ) const;

    bool            operator==( const VertexLayout& vl ) const;
    VertexLayout&   operator=( const VertexLayout& src );
    
    void            Clear();
    void            AddElement( VertexSemantics usage, unsigned usage_i, VertexDataType type, unsigned dimension );
    void            InsertElement( unsigned pos, VertexSemantics usage, unsigned usage_i, VertexDataType type, unsigned dimension );

    void            Dump() const;


private:

    enum
    {
        MaxElemCount    = 8
    };

    struct
    Element
    {
        uint32  usage:8;
        uint32  usage_index:8;
        uint32  data_type:8;
        uint32  data_count:8;
    };


    Element     _elem[MaxElemCount];
    uint32      _elem_count;
};


//==============================================================================
} // namespace rhi
#endif // __RHI_VERTEXLAYOUT_HPP__

