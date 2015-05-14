    
    #include "../rhi_ShaderSource.h"
    
    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    #include "FileSystem/DynamicMemoryFile.h"
    using DAVA::DynamicMemoryFile;
    #include "Utils/Utils.h"

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

static rhi::BlendOp
BlendOpFromText( const char* op )
{
    if     ( stricmp( op, "zero" ) == 0 )           return rhi::BLENDOP_ZERO;
    else if( stricmp( op, "one" ) == 0 )            return rhi::BLENDOP_ONE;
    else if( stricmp( op, "src_alpha" ) == 0 )      return rhi::BLENDOP_SRC_ALPHA;
    else if( stricmp( op, "inv_src_alpha" ) == 0)   return rhi::BLENDOP_INV_SRC_ALPHA;
    else if( stricmp( op, "src_color" ) == 0)       return rhi::BLENDOP_SRC_COLOR;
    else                                            return rhi::BLENDOP_ONE;
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
        std::regex  prop_re(".*property\\s*(float|float2|float3|float4|float4x4)\\s*([a-zA-Z_]+[a-zA-Z_0-9]*)\\s*\\:\\s*(.*)\\s+\\:(.*);.*");
        std::regex  proparr_re(".*property\\s*(float4|float4x4)\\s*([a-zA-Z_]+[a-zA-Z_0-9]*)\\s*\\[([0-9]+)\\]\\s*\\:\\s*(.*)\\s+\\:(.*);.*");
        std::regex  fsampler2d_re(".*DECL_FP_SAMPLER2D\\s*\\(\\s*(.*)\\s*\\).*");
        std::regex  vsampler2d_re(".*DECL_VP_SAMPLER2D\\s*\\(\\s*(.*)\\s*\\).*");
        std::regex  samplercube_re(".*DECL_FP_SAMPLERCUBE\\s*\\(\\s*(.*)\\s*\\).*");
        std::regex  ftexture2d_re(".*FP_TEXTURE2D\\s*\\(\\s*([a-zA-Z0-9_]+)\\s*\\,.*");
        std::regex  vtexture2d_re(".*VP_TEXTURE2D\\s*\\(\\s*([a-zA-Z0-9_]+)\\s*\\,.*");
        std::regex  texturecube_re(".*FP_TEXTURECUBE\\s*\\(\\s*([a-zA-Z0-9_]+)\\s*\\,.*");
        std::regex  blend_re(".*BLEND_MODE\\s*\\(\\s*(.*)\\s*\\).*");
        std::regex  blending2_re(".*blending\\s*\\:\\s*src=(zero|one|src_alpha|inv_src_alpha)\\s+dst=(zero|one|src_alpha|inv_src_alpha).*");
        std::regex  comment_re("\\s*//.*");

        _Reset();

        while( !in->IsEof() )
        {
            char        line[4*1024];
            uint32      lineLen     = in->ReadLine( line, sizeof(line) );
            std::cmatch match;
            bool        isComment   = std::regex_match( line, match, comment_re );
            bool        propDefined = false;
            bool        propArray   = false;

            if( !isComment  &&  std::regex_match( line, match, prop_re ) )
            {
                propDefined = true;
                propArray   = false;
            }
            else if( !isComment  &&  std::regex_match( line, match, proparr_re ) )
            {
                propDefined = true;
                propArray   = true;
            }


            if( propDefined )
            {
                prop.resize( prop.size()+1 );

                ShaderProp& p      = prop.back();
                std::string type;
                std::string uid;
                std::string tags;
                std::string script;
                std::string arrSz;

                if( propArray )
                {
                    type   = match[1].str();
                    uid    = match[2].str();
                    arrSz  = match[3].str();
                    tags   = match[4].str();
                    script = match[5].str();
                   
                    p.arraySize = atoi( arrSz.c_str() );
                }
                else
                {
                    type   = match[1].str();
                    uid    = match[2].str();
                    tags   = match[3].str();
                    script = match[4].str();
                    
                    p.arraySize = 1;
                }


                p.uid       = FastName(uid);
//                p.scope     = ShaderProp::SCOPE_SHARED;
                p.storage   = ShaderProp::STORAGE_DYNAMIC;
                p.type      = ShaderProp::TYPE_FLOAT4;

                if     ( stricmp( type.c_str(), "float" ) == 0 )    p.type = ShaderProp::TYPE_FLOAT1;
                else if( stricmp( type.c_str(), "float2" ) == 0 )   p.type = ShaderProp::TYPE_FLOAT2;
                else if( stricmp( type.c_str(), "float3" ) == 0 )   p.type = ShaderProp::TYPE_FLOAT3;
                else if( stricmp( type.c_str(), "float4" ) == 0 )   p.type = ShaderProp::TYPE_FLOAT4;
                else if( stricmp( type.c_str(), "float4x4" ) == 0 ) p.type = ShaderProp::TYPE_FLOAT4X4;
                
                {
                char        storage[128] = "";
                char        tag[128]     = "";
                const char* ss           = strchr( tags.c_str(), ',' );

                if( ss )
                {
                    int n = ss - tags.c_str();

                    strncpy( storage, tags.c_str(), n );
                    storage[n] = '\0';
                    strcpy( tag, tags.c_str()+n+1 );
                }
                else
                {
                    strcpy( storage, tags.c_str() );
                }

//                sscanf( tags.c_str(), "%s,%s", scope, tag );
                if     ( stricmp( storage, "static" ) == 0 )    p.storage = ShaderProp::STORAGE_STATIC;
                else if( stricmp( storage, "dynamic" ) == 0 )   p.storage = ShaderProp::STORAGE_DYNAMIC;
                p.tag = FastName(tag);
                }

                memset( p.defaultValue, 0, sizeof(p.defaultValue) );
                if( script.length() )
                {
                    const char* def_value = strstr( script.c_str(), "def_value" );
                    
                    if( def_value )
                    {
                        char    val[128];

                        if( sscanf( def_value, "def_value=%s", val ) == 1 )
                        {
                            DAVA::Vector<DAVA::String>  v;
                            
                            DAVA::Split( val, ",", v );
                            for( uint32 i=0; i!=v.size(); ++i )
                                p.defaultValue[i] = float(atof( v[i].c_str() ));
                        }
                    }
                }

                buf_t*  cbuf = 0;

                for( std::vector<buf_t>::iterator b=buf.begin(),b_end=buf.end(); b!=b_end; ++b )
                {
                    if( b->storage == p.storage  &&  b->tag == p.tag )
                    {
                        cbuf = &(buf[b-buf.begin()]);
                        break;
                    }
                }

                if( !cbuf )
                {
                    buf.resize( buf.size()+1 );

                    cbuf = &(buf.back());

                    cbuf->storage   = p.storage;
                    cbuf->tag       = p.tag;
                    cbuf->regCount  = 0;
                }

                p.bufferindex = cbuf - &(buf[0]);
                
                if( p.type == ShaderProp::TYPE_FLOAT1  ||  p.type == ShaderProp::TYPE_FLOAT2  ||  p.type == ShaderProp::TYPE_FLOAT3 )
                {
                    bool    do_add = true;
                    uint32  sz     = 0;
                    
                    switch( p.type )
                    {
                        case ShaderProp::TYPE_FLOAT1 : sz = 1; break;
                        case ShaderProp::TYPE_FLOAT2 : sz = 2; break;
                        case ShaderProp::TYPE_FLOAT3 : sz = 3; break;
                    }

                    for( unsigned r=0; r!=cbuf->avlRegIndex.size(); ++r )                    
                    {
                        if( cbuf->avlRegIndex[r] + sz <= 4 )
                        {
                            p.bufferReg      = r;
                            p.bufferRegCount = cbuf->avlRegIndex[r];

                            cbuf->avlRegIndex[r] += sz;
                            
                            do_add = false;
                            break;
                        }
                    }

                    if( do_add )
                    {
                        p.bufferReg       = cbuf->regCount;
                        p.bufferRegCount = 0;

                        ++cbuf->regCount;

                        cbuf->avlRegIndex.push_back( sz );
                    }
                }
                else if( p.type == ShaderProp::TYPE_FLOAT4  ||  p.type == ShaderProp::TYPE_FLOAT4X4 )
                {
                    p.bufferReg        = cbuf->regCount;
                    p.bufferRegCount   = p.arraySize * ((p.type == ShaderProp::TYPE_FLOAT4)  ? 1  : 4);

                    cbuf->regCount     += p.bufferRegCount;

                    for( int i=0; i!=p.bufferRegCount; ++i )
                        cbuf->avlRegIndex.push_back( 4 );
                }
            }
            else if( !isComment  &&  std::regex_match( line, match, fsampler2d_re ) )
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
                sampler.back().type = TEXTURE_TYPE_2D;

                _AppendLine( line, strlen(line) );
            }
            else if( !isComment  &&  std::regex_match( line, match, samplercube_re ) )
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
                sampler.back().type = TEXTURE_TYPE_CUBE;

                _AppendLine( line, strlen(line) );
            }
            else if( !isComment  &&  std::regex_match( line, match, ftexture2d_re ) )
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
                
                _AppendLine( line, strlen(line) );
            }
            else if( !isComment  &&  std::regex_match( line, match, vsampler2d_re ) )
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
                sampler.back().type = TEXTURE_TYPE_2D;

                _AppendLine( line, strlen(line) );
            }
            else if( !isComment  &&  std::regex_match( line, match, vtexture2d_re ) )
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
                
                _AppendLine( line, strlen(line) );
            }
            else if( !isComment  &&  std::regex_match( line, match, texturecube_re ) )
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

                _AppendLine( line, strlen(line) );
            }
            else if( !isComment  &&  std::regex_match( line, match, blend_re ) )
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
            else if( !isComment  &&  std::regex_match( line, match, blending2_re ) )
            {
                std::string src   = match[1].str();
                std::string dst   = match[2].str();
                
                blending.rtBlend[0].blendEnabled = true;
                blending.rtBlend[0].colorSrc     = blending.rtBlend[0].alphaSrc = BlendOpFromText( src.c_str() );
                blending.rtBlend[0].colorDst     = blending.rtBlend[0].alphaDst = BlendOpFromText( dst.c_str() );
            }
            else
            {
                _AppendLine( line, strlen(line) );
            }


            if( strstr( line, "VPROG_IN_BEGIN" ) )
                vdecl.Clear();
            if( strstr( line, "VPROG_IN_POSITION" ) )
                vdecl.AddElement( VS_POSITION, 0, VDT_FLOAT, 3 );
            if( strstr( line, "VPROG_IN_NORMAL" ) )
                vdecl.AddElement( VS_NORMAL, 0, VDT_FLOAT, 3 );
            
            if( strstr( line, "VPROG_IN_TEXCOORD" ) )
            {
                uint32  usage_i  = 0;
                uint32  data_cnt = 2;

                std::regex  texcoord_re(".*VPROG_IN_TEXCOORD\\s*([0-7])\\s*\\(([0-7])\\s*\\).*");

                if( std::regex_match( line, match, texcoord_re ) )
                {
                    std::string u = match[1].str();
                    std::string c = match[2].str();
                    
                    usage_i  = atoi( u.c_str() );                
                    data_cnt = atoi( c.c_str() );                
                }

                vdecl.AddElement( VS_TEXCOORD, usage_i, VDT_FLOAT, data_cnt );
            }

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

            buf_len += Snprintf( buf_def+buf_len, sizeof(buf_def)-buf_len, "//--------\n" );
            for( unsigned i=0; i!=buf.size(); ++i )
            {
                buf_len += Snprintf(buf_def + buf_len, sizeof(buf_def) - buf_len, "DECL_%cPROG_BUFFER(%u,%u,%u)\n", pt, i, buf[i].regCount, reg);
                reg += buf[i].regCount;
            }
            buf_len += Snprintf(buf_def + buf_len, sizeof(buf_def) - buf_len, "\n\n");

            var_len += Snprintf(var_def + var_len, sizeof(var_def) - var_len, "    //--------\n");
            for( std::vector<ShaderProp>::const_iterator p=prop.begin(),p_end=prop.end(); p!=p_end; ++p )
            {
                switch( p->type )
                {
                    case ShaderProp::TYPE_FLOAT1 :
                    {
                        const char* xyzw = "xyzw";
                        var_len += Snprintf( var_def+var_len, sizeof(var_def)-var_len, "    float %s = %cP_Buffer%u[%u].%c;\n", p->uid.c_str(), pt, p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount] );
                    }   break;
                    
                    case ShaderProp::TYPE_FLOAT2 :
                    {
                        const char* xyzw = "xyzw";
                        var_len += Snprintf
                        ( 
                            var_def+var_len, sizeof(var_def)-var_len, 
                            "    float2 %s = float2( %cP_Buffer%u[%u].%c, %cP_Buffer%u[%u].%c );\n", 
                            p->uid.c_str(), 
                            pt, p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount+0],
                            pt, p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount+1]
                        );
                    }   break;
                    
                    case ShaderProp::TYPE_FLOAT3 :
                    {
                        const char* xyzw = "xyzw";
                        var_len += Snprintf
                        ( 
                            var_def+var_len, sizeof(var_def)-var_len, 
                            "    float3 %s = float3( %cP_Buffer%u[%u].%c, %cP_Buffer%u[%u].%c, %cP_Buffer%u[%u].%c );\n", 
                            p->uid.c_str(), 
                            pt, p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount+0],
                            pt, p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount+1],
                            pt, p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount+2]
                        );
                    }   break;

                    case ShaderProp::TYPE_FLOAT4 :
                        var_len += Snprintf(var_def + var_len, sizeof(var_def) - var_len, "    float4 %s = %cP_Buffer%u[%u];\n", p->uid.c_str(), pt, p->bufferindex, p->bufferReg);
                        break;

                    case ShaderProp::TYPE_FLOAT4X4 :
                    {
                        var_len += Snprintf
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
            var_len += Snprintf(var_def + var_len, sizeof(var_def) - var_len, "    //--------\n");
            var_len += Snprintf(var_def + var_len, sizeof(var_def) - var_len, "\n\n");
            
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
/*
ShaderProp::Scope
ShaderSource::ConstBufferScope( uint32 bufIndex ) const
{
    return buf[bufIndex].scope;
}
*/

//------------------------------------------------------------------------------

ShaderProp::Storage
ShaderSource::ConstBufferStorage( uint32 bufIndex ) const
{
    return buf[bufIndex].storage;
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
    codeLineCount = 0;
    
    for( unsigned i=0; i!=countof(blending.rtBlend); ++i )
    {
        blending.rtBlend[i].blendEnabled    = false;
        blending.rtBlend[i].alphaToCoverage = false;
    }
}


//------------------------------------------------------------------------------

void
ShaderSource::_AppendLine( const char* line, uint32 lineLen )
{
code.append( line, lineLen );
code.push_back( '\n' );
return;

    char    text[4*1024];
    int     len = Snprintf(text, sizeof(text) - 1, "/*%04u*/ ", codeLineCount + 1);

    strncpy( text+len, line, lineLen );
    text[len+lineLen] = '\0';

    code.append( text, strlen(text) );
    code.push_back( '\n' );

    ++codeLineCount;        
}


//------------------------------------------------------------------------------

void
ShaderSource::Dump() const
{
    Logger::Info( "src-code:" );

    char        src[64*1024];
    char*       src_line[1024];
    unsigned    line_cnt        = 0;
    
    if( strlen(code.c_str()) < sizeof(src) )
    {
        strcpy( src, code.c_str() );
        memset( src_line, 0, sizeof(src_line) );

        src_line[line_cnt++] = src;
        for( char* s=src; *s; ++s )
        {
            if( *s == '\n'  ||  *s == '\r' )
            {
                while( *s  &&  (*s == '\n'  ||  *s == '\r') )
                {
                    *s = 0;
                    ++s;
                }

                if( !(*s) )
                    break;
            
                src_line[line_cnt] = s;
                ++line_cnt;
            }
        }
    
        for( unsigned i=0; i!=line_cnt; ++i )
        {
            Logger::Info( "%4u |  %s", 1+i, src_line[i] );
        }
    }
    else
    {
        Logger::Info( code.c_str() );
    }

    Logger::Info( "properties (%u) :", prop.size() );
    for( std::vector<ShaderProp>::const_iterator p=prop.begin(),p_end=prop.end(); p!=p_end; ++p )
    {
        if( p->type == ShaderProp::TYPE_FLOAT4  ||  p->type == ShaderProp::TYPE_FLOAT4X4 )
        {
            if( p->arraySize == 1 )
            {
                Logger::Info( "  %-16s    buf#%u  -  %u, %u x float4", p->uid.c_str(), p->bufferindex, p->bufferReg, p->bufferRegCount );
            }
            else
            {
                char    name[128];

                Snprintf( name, sizeof(name)-1, "%s[%u]", p->uid.c_str(), p->arraySize );
                Logger::Info( "  %-16s    buf#%u  -  %u, %u x float4", name, p->bufferindex, p->bufferReg, p->bufferRegCount );
            }
        }
        else
        {
            const char* xyzw = "xyzw";

            switch( p->type )
            {
                case ShaderProp::TYPE_FLOAT1 :
                    Logger::Info( "  %-16s    buf#%u  -  %u, %c", p->uid.c_str(), p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount] );
                    break;
                
                case ShaderProp::TYPE_FLOAT2 :
                    Logger::Info( "  %-16s    buf#%u  -  %u, %c%c", p->uid.c_str(), p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount+0], xyzw[p->bufferRegCount+1] );
                    break;
                
                case ShaderProp::TYPE_FLOAT3 :
                    Logger::Info( "  %-16s    buf#%u  -  %u, %c%c%c", p->uid.c_str(), p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount+0], xyzw[p->bufferRegCount+1], xyzw[p->bufferRegCount+2] );
                    break;
            }
        }
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

    Logger::Info( "samplers (%u) :", sampler.size() );
    for( unsigned s=0; s!=sampler.size(); ++s )
    {
        Logger::Info( "  sampler#%u  \"%s\"", s, sampler[s].uid.c_str() );
    }
}


//==============================================================================
} // namespace rhi

