    
    #include "../rhi_ShaderSource.h"
    
    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    #include "FileSystem/DynamicMemoryFile.h"
    using DAVA::DynamicMemoryFile;

    #include "PreProcess.h"

    #include <regex>


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
    std::vector<std::string>    def;

    return ShaderSource::Construct( progType, srcText, def );
}


//------------------------------------------------------------------------------

bool
ShaderSource::Construct( ProgType progType, const char* srcText, const std::vector<std::string>& defines )
{
    bool                        success = false;
    std::vector<std::string>    def;
    const char*                 argv[128];
    int                         argc    = 0;
    std::string                 src;


    // pre-process source text with #defines, if any

    DVASSERT(defines.size()%2 == 0);
    def.resize( defines.size()/2 );
    for( unsigned i=0; i!=def.size(); ++i )
    {
        char    d[256];

        sprintf( d, "-D %s=%s", defines[i*2+0].c_str(), defines[i*2+1].c_str() );
        def[i] = d;
    }
    for( unsigned i=0; i!=def.size(); ++i )
        argv[argc++] = def[i].c_str();
    PreProcessText( srcText, argv, argc, &src );

    
    // parse properties/samplers
    
    DynamicMemoryFile*  in = DynamicMemoryFile::Create( (const uint8*)src.c_str(), src.length()+1, DAVA::File::READ );

    if( in )
    {
        std::regex  prop_re(".*property\\s*(float|float4|float4x4)\\s*([a-zA-Z_]+[a-zA-Z_0-9]*)\\s*\\:\\s*(.*)\\s+\\:(.*);.*");
        std::regex  sampler_re(".*DECL_SAMPLER2D\\s*\\(\\s*(.*)\\s*\\).*");
        std::regex  texture_re(".*FP_TEXTURE2D\\s*\\(\\s*([a-zA-Z0-9_]+)\\s*\\,.*");
        std::regex  blend_re(".*BLEND_MODE\\s*\\(\\s*(.*)\\s*\\).*");

        _Reset();

        while( !in->IsEof() )
        {
            char        line[4*1024];
            uint32      lineLen = in->ReadLine( line, sizeof(line) );
            std::cmatch match;


            if( std::regex_match( line, match, prop_re ) )
            {
                prop.resize( prop.size()+1 );

                ShaderProp& p      = prop.back();
                std::string type   = match[1].str();
                std::string uid    = match[2].str();
                std::string tags   = match[3].str();
                std::string script = match[4].str();

                p.uid   = FastName(uid);
                p.scope = ShaderProp::SCOPE_SHARED;

                if     ( stricmp( type.c_str(), "float" ) == 0 )    p.type = ShaderProp::TYPE_FLOAT1;
                else if( stricmp( type.c_str(), "float4" ) == 0 )   p.type = ShaderProp::TYPE_FLOAT4;
                else if( stricmp( type.c_str(), "float4x4" ) == 0 ) p.type = ShaderProp::TYPE_FLOAT4X4;
                
                {
                char        scope[128]  = "";
                char        tag[128]    = "";
                const char* ss          = strchr( tags.c_str(), ',' );

                if( ss )
                {
                    int n = ss - tags.c_str();

                    strncpy( scope, tags.c_str(), n );
                    scope[n] = '\0';
                    strcpy( tag, tags.c_str()+n+1 );
                }
                else
                {
                    strcpy( scope, tags.c_str() );
                }

//                sscanf( tags.c_str(), "%s,%s", scope, tag );
                if     ( stricmp( scope, "shared" ) == 0 )   p.scope = ShaderProp::SCOPE_SHARED;
                else if( stricmp( scope, "unique" ) == 0 )   p.scope = ShaderProp::SCOPE_UNIQUE;
                p.tag = FastName(tag);
                }

                memset( p.defaultValue, 0, sizeof(p.defaultValue) );
                if( script.length() )
                {
                }

                buf_t*  cbuf = 0;

                for( std::vector<buf_t>::iterator b=buf.begin(),b_end=buf.end(); b!=b_end; ++b )
                {
                    if( b->scope == p.scope  &&  b->tag == p.tag )
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
                    cbuf->tag       = p.tag;
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
            else if( std::regex_match( line, match, sampler_re ) )
            {
                std::string sname   = match[1].str();
                int         mbegin  = strstr( line, sname.c_str() ) - line;
                int         sn      = sname.length();

                DVASSERT(sampler.size()<10);
                char ch = line[mbegin+1];
                int  sl = sprintf( line+mbegin, "%u", (unsigned)(sampler.size()) );
                DVASSERT(sn>=sl);
                line[mbegin+1]=ch;
                if( sn > sl )
                    memset( line+mbegin+sl, ' ', sn-sl );
                sampler.resize( sampler.size()+1 );
                sampler.back().uid  = FastName(sname);

                code.append( line, strlen(line) );
                code.push_back( '\n' );
            }
            else if( std::regex_match( line, match, texture_re ) )
            {
                std::string sname   = match[1].str();
                int         mbegin  = strstr( line, sname.c_str() ) - line;
                FastName    suid    ( sname );
                
                for( unsigned s=0; s!=sampler.size(); ++s )
                {
                    if( sampler[s].uid == suid )
                    {
                        int sl = sprintf( line+mbegin, "%u", s );
                        int sn = sname.length();
                        DVASSERT(sn>=sl);
                        line[mbegin+sl] = ',';
                        if( sn > sl )
                            memset( line+mbegin+sl, ' ', sn-sl );
                        
                        break;
                    }
                }

                code.append( line, strlen(line) );
                code.push_back( '\n' );
            }
            else if( std::regex_match( line, match, blend_re ) )
            {
                std::string mode   = match[1].str();
                
                if( !stricmp( mode.c_str(), "alpha" ) )
                {
                    blending.rtBlend[0].blendEnabled = true;
                    blending.rtBlend[0].colorSrc = BLENDOP_SRC_ALPHA;
                    blending.rtBlend[0].colorDst = BLENDOP_INV_SRC_ALPHA;
                    blending.rtBlend[0].alphaSrc = BLENDOP_SRC_ALPHA;
                    blending.rtBlend[0].alphaDst = BLENDOP_INV_SRC_ALPHA;
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
            if( strstr( line, "VPROG_IN_COLOR" ) )
                vdecl.AddElement( VS_COLOR, 0, VDT_UINT8N, 4 );
        } // for each line

        type =  progType;

        success = true;
    }


    if( success )
    {
        const char* prog_begin  = (progType == PROG_VERTEX)  ? "VPROG_BEGIN"  : "FPROG_BEGIN";
        const char* prog        = strstr( code.c_str(), prog_begin );

        if( prog )
        {
            char        buf_def[1024];
            int         buf_len         = 0;
            char        var_def[4*1024];
            int         var_len         = 0;
            char        pt      = (progType == PROG_VERTEX)?'V':'F';
            unsigned    reg     = 0;

            buf_len += Snprinf( buf_def+buf_len, sizeof(buf_def)-buf_len, "//--------\n" );
            for( unsigned i=0; i!=buf.size(); ++i )
            {
                buf_len += Snprinf( buf_def+buf_len, sizeof(buf_def)-buf_len, "DECL_%cPROG_BUFFER(%u,%u,%u)\n", pt, i, buf[i].regCount, reg );
                reg += buf[i].regCount;
            }
            buf_len += Snprinf( buf_def+buf_len, sizeof(buf_def)-buf_len, "\n\n" );

            var_len += Snprinf( var_def+var_len, sizeof(var_def)-var_len, "    //--------\n" );
            for( std::vector<ShaderProp>::const_iterator p=prop.begin(),p_end=prop.end(); p!=p_end; ++p )
            {
                switch( p->type )
                {
                    case ShaderProp::TYPE_FLOAT1 :
                    {
                        char xyzw[] = "xyzw";
                        var_len += Snprinf( var_def+var_len, sizeof(var_def)-var_len, "    float %s = %cP_Buffer%u[%u].%c;\n", p->uid.c_str(), pt, p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount] );
                    }   break;

                    case ShaderProp::TYPE_FLOAT4 :
                        var_len += Snprinf( var_def+var_len, sizeof(var_def)-var_len, "    float4 %s = %cP_Buffer%u[%u];\n", p->uid.c_str(), pt, p->bufferindex, p->bufferReg );
                        break;

                    case ShaderProp::TYPE_FLOAT4X4 :
                    {
                        var_len += Snprinf
                        ( 
                            var_def+var_len, sizeof(var_def)-var_len, 
                            "    float4x4 %s = float4x4( %cP_Buffer%u[%u], %cP_Buffer%u[%u], %cP_Buffer%u[%u], %cP_Buffer%u[%u] );\n", 
                            p->uid.c_str(), 
                            pt, p->bufferindex, p->bufferReg+0,
                            pt, p->bufferindex, p->bufferReg+1,
                            pt, p->bufferindex, p->bufferReg+2,
                            pt, p->bufferindex, p->bufferReg+3
                        );
                    }   break;
                };
            }
            var_len += Snprinf( var_def+var_len, sizeof(var_def)-var_len, "    //--------\n" );
            var_len += Snprinf( var_def+var_len, sizeof(var_def)-var_len, "\n\n" );
            
            unsigned var_pos = (prog-code.c_str()) + strlen("XPROG_BEGIN") + buf_len +2;

            code.insert( prog-code.c_str(), buf_def, buf_len );
            code.insert( var_pos, var_def, var_len );
        }
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

const ShaderSamplerList&
ShaderSource::Samplers() const
{
    return sampler;
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

BlendState
ShaderSource::Blending()
{
    return blending;
}


//------------------------------------------------------------------------------

void
ShaderSource::_Reset()
{
    vdecl.Clear();
    prop.clear();
    sampler.clear();
    buf.clear();
    code.clear();
    
    for( unsigned i=0; i!=countof(blending.rtBlend); ++i )
    {
        blending.rtBlend[i].blendEnabled    = false;
        blending.rtBlend[i].alphaToCoverage = false;
    }
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

    if( type == PROG_FRAGMENT )
    {
        Logger::Info( "samplers (%u) :", sampler.size() );
        for( unsigned s=0; s!=sampler.size(); ++s )
        {
            Logger::Info( "  sampler#%u  \"%s\"", s, sampler[s].uid.c_str() );
        }
    }
}


//==============================================================================
} // namespace rhi

