    
    #include "rhi_ShaderSource.h"
    
    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    #include "FileSystem/DynamicMemoryFile.h"
    using DAVA::DynamicMemoryFile;

    #include "RegExp.h"
    #include "PreProcess.h"



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
    std::string         src;    PreProcessText( srcText, &src );    
    DynamicMemoryFile*  in      = DynamicMemoryFile::Create( (const uint8*)src.c_str(), src.length()+1, DAVA::File::READ );

    if( in )
    {
        RegExp  prop_re;
        RegExp  sampler_re;
        RegExp  texture_re;
        
        prop_re.compile( "\\s*DECLARE_PROP__(DYNAMIC|STATIC)_(GLOBAL|LOCAL)\\s*\\(\\s*(float4x4|float4|float1)\\s*\\,\\s*([a-zA-Z_]+[a-zA-Z_0-9]+)\\s*\\,\\s*\\\"(.*)\\\"\\s*\\)" );
        sampler_re.compile( "\\s*DECL_SAMPLER2D\\s*\\(\\s*(.*)\\s*\\)" );
        texture_re.compile( "\\s?FP_TEXTURE2D\\s?\\(\\s?([a-zA-Z0-9_]+)\\s?\\," );
    

        _Reset();

        while( !in->IsEof() )
        {
            char    line[1024];
            uint32  lineLen = in->ReadLine( line, sizeof(line) );

            if( prop_re.test( line ) )
            {
                prop.resize( prop.size()+1 );

                ShaderProp& p           = prop.back();
                char        storage[32];prop_re.get_pattern( 1, countof(storage), storage );
                char        scope[32];  prop_re.get_pattern( 2, countof(scope), scope );
                char        ts[32];     prop_re.get_pattern( 3, countof(ts), ts );
                char        uid[32];    prop_re.get_pattern( 4, countof(uid), uid );
                char        script[256];prop_re.get_pattern( 5, countof(script), script );

                p.uid     = FastName(uid);
                p.type    = (stricmp( ts, "float4x4" ) == 0)  
                            ? ShaderProp::TYPE_FLOAT4X4  
                            : ((stricmp( ts, "float1" ) == 0) ? ShaderProp::TYPE_FLOAT1 : ShaderProp::TYPE_FLOAT4);
                p.scope   = (stricmp( scope, "local" ) == 0)
                            ? ShaderProp::SCOPE_LOCAL
                            : ShaderProp::SCOPE_GLOBAL;
                p.storage = (stricmp( storage, "dynamic" ) == 0)
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
            else if( sampler_re.test( line ) )
            {
                char                 sname[32]; sampler_re.get_pattern( 1, countof(sname), sname );
                const RegExp::Match* match      = sampler_re.pattern( 1 );
                int                  sn         = strlen( sname );

                DVASSERT(sampler.size()<10);
                char ch = line[match->begin+1];
                int  sl = sprintf( line+match->begin, "%u", sampler.size() );
                DVASSERT(sn>=sl);
                line[match->begin+1]=ch;
                if( sn > sl )
                    memset( line+match->begin+sl, ' ', sn-sl );

                sampler.resize( sampler.size()+1 );
                sampler.back().uid  = FastName(sname);

                code.append( line, strlen(line) );
                code.push_back( '\n' );
            }
            else if( texture_re.test( line ) )
            {
                const RegExp::Match* match      = texture_re.pattern( 1 );
                char                 sname[32]; texture_re.get_pattern( 1, countof(sname), sname );
                FastName             uid        ( sname );

                for( unsigned s=0; s!=sampler.size(); ++s )
                {
                    if( sampler[s].uid == uid )
                    {
                        int sl = sprintf( line+match->begin, "%u", s );
                        int sn = strlen( sname );
                        DVASSERT(sn>=sl);
                        line[match->begin+sl] = ',';
                        if( sn > sl )
                            memset( line+match->begin+sl, ' ', sn-sl );
                        
                        break;
                    }
                }
                code.append( line, strlen(line) );
                code.push_back( '\n' );
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

