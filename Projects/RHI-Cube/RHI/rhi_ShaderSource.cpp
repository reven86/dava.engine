    
    #include "rhi_ShaderSource.h"
    
    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    #include "FileSystem/DynamicMemoryFile.h"
    using DAVA::DynamicMemoryFile;

    #include "RegExp.h"


namespace rhi
{
//==============================================================================

ShaderSource::ShaderSource()
{
}


//------------------------------------------------------------------------------

ShaderSource::~ShaderSource()
{
}


//------------------------------------------------------------------------------

bool
ShaderSource::Construct( ProgType progType, const char* srcText )
{
    bool                success = false;
    DynamicMemoryFile*  in      = DynamicMemoryFile::Create( (const uint8*)srcText, strlen(srcText)+1, DAVA::File::READ );

    if( in )
    {
        RegExp  re;
        RegExp  s_re;
        
        re.compile( "\\s*DECLARE_PROP__(DYNAMIC|STATIC)_(GLOBAL|LOCAL)\\s*\\(\\s*(float4x4|float4|float1)\\s*\\,\\s*([a-zA-Z_]+[a-zA-Z_0-9]+)\\s*\\,\\s*\\\"(.*)\\\"\\s*\\)" );
        s_re.compile( "\\s*DECL_SAMPLER2D\\s*\\(\\s*(.*)\\s*\\)" );
    

        _Reset();

        while( !in->IsEof() )
        {
            char    line[1024];
            uint32  lineLen = in->ReadLine( line, sizeof(line) );

            if( re.test( line ) )
            {
                prop.resize( prop.size()+1 );

                ShaderProp& p           = prop.back();
                char        storage[32];re.get_pattern( 1, countof(storage), storage );
                char        scope[32];  re.get_pattern( 2, countof(scope), scope );
                char        ts[32];     re.get_pattern( 3, countof(ts), ts );
                char        uid[32];    re.get_pattern( 4, countof(uid), uid );
                char        script[256];re.get_pattern( 5, countof(script), script );

                p.uid     = FastName(uid);
                p.type    = (_stricmp( ts, "float4x4" ) == 0)  
                            ? ShaderProp::TYPE_FLOAT4X4  
                            : ((_stricmp( ts, "float1" ) == 0) ? ShaderProp::TYPE_FLOAT1 : ShaderProp::TYPE_FLOAT4);
                p.scope   = (_stricmp( scope, "local" ) == 0)  
                            ? ShaderProp::SCOPE_LOCAL
                            : ShaderProp::SCOPE_GLOBAL;
                p.storage = (_stricmp( storage, "dynamic" ) == 0)  
                            ? ShaderProp::STORAGE_DYNAMIC
                            : ShaderProp::STORAGE_STATIC;
                memset( p.defaultValue, 0, sizeof(p.defaultValue) );

                if( !IsEmptyString( script ) )
                {
                }


                buf_t*  cbuf = 0;

                for( std::vector<buf_t>::iterator b=buf.begin(),b_end=buf.end(); b!=b_end; ++b )
                {
                    if( b->scope == p.scope  &&  b->storage == p.storage )
                    {
                        cbuf = &(buf[b-buf.begin()]);
                        break;
                    }
                }

                if( !cbuf )
                {
                    buf.resize( buf.size()+1 );

                    cbuf = &(buf.back());

                    cbuf->scope     = p.scope;
                    cbuf->storage   = p.storage;
                    cbuf->regCount  = 0;
                }

                p.bufferindex = cbuf - &(buf[0]);

                if( p.type == ShaderProp::TYPE_FLOAT1 )
                {
                    bool    do_add = true;

                    for( std::vector<ShaderProp>::const_iterator pp=prop.begin(),pp_end=prop.end(); pp!=pp_end; ++pp )
                    {
                        if(     pp->type == ShaderProp::TYPE_FLOAT1 
                            &&  pp->bufferRegCount < 3
                          )
                        {
                            p.bufferReg      = pp->bufferReg;
                            p.bufferRegCount = pp->bufferRegCount + 1;

                            do_add = false;
                            break;
                        }
                    }

                    if( do_add )
                    {
                        p.bufferReg       = cbuf->regCount;
                        p.bufferRegCount = 0;

                        ++cbuf->regCount;
                    }
                }
                else
                {
                    p.bufferReg        = cbuf->regCount;
                    p.bufferRegCount   = (p.type == ShaderProp::TYPE_FLOAT4)  ? 1  : 4;

                    cbuf->regCount     += p.bufferRegCount;
                }
            }
            else
            {
                code.append( line, lineLen );
                code.push_back( '\n' );
            }


            if( strstr( line, "VPROG_IN_BEGIN" ) )
                vdecl.Clear();
            if( strstr( line, "VPROG_IN_POSITION" ) )
                vdecl.AddElement( VS_POSITION, 0, VDT_FLOAT, 3 );
            if( strstr( line, "VPROG_IN_NORMAL" ) )
                vdecl.AddElement( VS_NORMAL, 0, VDT_FLOAT, 3 );
            if( strstr( line, "VPROG_IN_TEXCOORD" ) )
                vdecl.AddElement( VS_TEXCOORD, 0, VDT_FLOAT, 2 );
        } // for each line
    }

    return success;
}


//------------------------------------------------------------------------------

const char*
ShaderSource::SourceCode() const
{
    return code.c_str();
}


//------------------------------------------------------------------------------

const ShaderPropList&
ShaderSource::Properties() const
{
    return prop;
}


//------------------------------------------------------------------------------

const VertexLayout&
ShaderSource::ShaderVertexLayout() const
{
    return vdecl;
}


//------------------------------------------------------------------------------

uint32
ShaderSource::ConstBufferCount() const
{
    return buf.size();
}


//------------------------------------------------------------------------------

uint32
ShaderSource::ConstBufferSize( uint32 bufIndex ) const
{
    return buf[bufIndex].regCount;
}


//------------------------------------------------------------------------------

ShaderProp::Scope
ShaderSource::ConstBufferScope( uint32 bufIndex ) const
{
    return buf[bufIndex].scope;
}


//------------------------------------------------------------------------------

void
ShaderSource::_Reset()
{
    vdecl.Clear();
    prop.clear();
    buf.clear();
    code.clear();
}


//------------------------------------------------------------------------------

void
ShaderSource::Dump() const
{
    Logger::Info( "src-code:" );
    Logger::Info( code.c_str() );

    Logger::Info( "properties (%u) :", prop.size() );
    for( std::vector<ShaderProp>::const_iterator p=prop.begin(),p_end=prop.end(); p!=p_end; ++p )
    {
        Logger::Info( "  %-16s    buf#%u  -  %u, %u x float4", p->uid.c_str(), p->bufferindex, p->bufferReg, p->bufferRegCount );
    }

    Logger::Info( "buffers (%u) :", buf.size() );
    for( unsigned i=0; i!=buf.size(); ++i )
    {
        Logger::Info( "  buf#%u  reg.count = %u", i, buf[i].regCount );
    }

    if( type == PROG_VERTEX )
    {
        Logger::Info( "vertex-layout:" );
        vdecl.Dump();
    }
}


//==============================================================================
} // namespace rhi

