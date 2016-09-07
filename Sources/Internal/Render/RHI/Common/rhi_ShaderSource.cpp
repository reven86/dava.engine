#include "../rhi_ShaderSource.h"

//#define PROFILER_ENABLED 1
//#include "Debug/Profiler.h"

    #include "Logger/Logger.h"
using DAVA::Logger;
    #include "FileSystem/DynamicMemoryFile.h"
    #include "FileSystem/FileSystem.h"
using DAVA::DynamicMemoryFile;
    #include "Utils/Utils.h"
    #include "Debug/Profiler.h"
    #include "Concurrency/Mutex.h"
    #include "Concurrency/LockGuard.h"
using DAVA::Mutex;
using DAVA::LockGuard;

    #include "PreProcess.h"
    
    #include "Parser/sl_Parser.h"
    #include "Parser/sl_Tree.h"
    #include "Parser/sl_GeneratorHLSL.h"
    #include "Parser/sl_GeneratorGLES.h"
    #include "Parser/sl_GeneratorMSL.h"

    #define RHI__USE_STD_REGEX 0
    #define RHI__OPTIMIZED_REGEX 1


    #if RHI__USE_STD_REGEX
        #include <regex>
    #else
        #include "RegExp.h"
    #endif

namespace rhi
{
//==============================================================================

ShaderSource::ShaderSource(const char* filename)
    : fileName(filename)
{
}

//------------------------------------------------------------------------------

ShaderSource::~ShaderSource()
{
}

//------------------------------------------------------------------------------

static rhi::BlendOp
BlendOpFromText(const char* op)
{
    if (stricmp(op, "zero") == 0)
        return rhi::BLENDOP_ZERO;
    else if (stricmp(op, "one") == 0)
        return rhi::BLENDOP_ONE;
    else if (stricmp(op, "src_alpha") == 0)
        return rhi::BLENDOP_SRC_ALPHA;
    else if (stricmp(op, "inv_src_alpha") == 0)
        return rhi::BLENDOP_INV_SRC_ALPHA;
    else if (stricmp(op, "src_color") == 0)
        return rhi::BLENDOP_SRC_COLOR;
    else if (stricmp(op, "dst_color") == 0)
        return rhi::BLENDOP_DST_COLOR;
    else
        return rhi::BLENDOP_ONE;
}

//------------------------------------------------------------------------------

bool ShaderSource::Construct(ProgType progType, const char* srcText)
{
    std::vector<std::string> def;

    return ShaderSource::Construct(progType, srcText, def);
}

//------------------------------------------------------------------------------

bool ShaderSource::Construct(ProgType progType, const char* srcText, const std::vector<std::string>& defines)
{
    //SCOPED_NAMED_TIMING("ShaderSource::Construct");
    bool success = false;
    std::vector<std::string> def;
    const char* argv[128];
    int argc = 0;
    std::string src;

    // pre-process source text with #defines, if any

    DVASSERT(defines.size() % 2 == 0);
    def.reserve(defines.size() / 2);
    for (size_t i = 0, n = defines.size() / 2; i != n; ++i)
    {
        const char* s1 = defines[i * 2 + 0].c_str();
        const char* s2 = defines[i * 2 + 1].c_str();
        def.push_back(DAVA::Format("-D %s=%s", s1, s2));
    }
    for (unsigned i = 0; i != def.size(); ++i)
        argv[argc++] = def[i].c_str();
    SetPreprocessCurFile(fileName.c_str());
    PreProcessText(srcText, argv, argc, &src);

#if 0
{
    Logger::Info("src-code:");

    char ss[64 * 1024];
    unsigned line_cnt = 0;

    if (strlen(src.c_str()) < sizeof(ss))
    {
        strcpy(ss, src.c_str());

        const char* line = ss;
        for (char* s = ss; *s; ++s)
        {
            if( *s=='\r')
                *s=' ';

            if (*s == '\n')
            {
                *s = 0;
                Logger::Info("%4u |  %s", 1 + line_cnt, line);
                line = s+1;
                ++line_cnt;
            }
        }
    }
    else
    {
        Logger::Info(src.c_str());
    }
}
#endif

    static sl::Allocator alloc;
    //    sl::HLSLTree tree(&alloc);
    sl::HLSLParser parser(&alloc, "<shader>", src.c_str(), strlen(src.c_str()));
    ast = new sl::HLSLTree(&alloc);

    if (parser.Parse(ast))
    {
        _ProcessMetaData(ast);
        type = progType;
        success = true;
    }
    else
    {
        delete ast;
        ast = nullptr;
        sl::Log_Error("Parse error\n");
        DVASSERT(ast);
    }

    // parse properties/samplers
    /*
    DAVA::ScopedPtr<DynamicMemoryFile> in(DynamicMemoryFile::Create(reinterpret_cast<const uint8*>(src.c_str()), uint32(src.length() + 1), DAVA::File::READ));

    if (in)
    {

        _Reset();
        STOP_NAMED_TIMING("shadersrc.setup");

        START_NAMED_TIMING("shadersrc.parse");
        while (!in->IsEof())
        {
            char line[4 * 1024];
            uint32 lineLen = in->ReadLine(line, sizeof(line));
            #if RHI__USE_STD_REGEX
            std::cmatch match;
            bool isComment = std::regex_match(line, match, comment_re);
            #else
            bool isComment = comment_re.test(line);
            #endif
            bool propDefined = false;
            bool propArray = false;

            #if RHI__USE_STD_REGEX
            if (!isComment && std::regex_match(line, match, prop_re))
            {
                propDefined = true;
                propArray = false;
            }
            else if (!isComment && std::regex_match(line, match, proparr_re))
            {
                propDefined = true;
                propArray = true;
            }
            #else
            if (!isComment && prop_re.test(line))
            {
                propDefined = true;
                propArray = false;
            }
            else if (!isComment && proparr_re.test(line))
            {
                propDefined = true;
                propArray = true;
            }
            #endif

            if (propDefined)
            {
                prop.resize(prop.size() + 1);

                ShaderProp& p = prop.back();
                std::string type;
                std::string uid;
                std::string tags;
                std::string script;
                std::string arrSz;

                
                #if RHI__USE_STD_REGEX
                if (propArray)
                {
                    type = match[1].str();
                    uid = match[2].str();
                    arrSz = match[3].str();
                    tags = match[4].str();
                    script = match[5].str();

                    p.arraySize = atoi(arrSz.c_str());
                    p.isBigArray = (strstr(script.c_str(), "bigarray")) ? true : false;
                }
                else
                {
                    type = match[1].str();
                    uid = match[2].str();
                    tags = match[3].str();
                    script = match[4].str();

                    p.arraySize = 1;
                    p.isBigArray = false;
                }
                #else
                if (propArray)
                {
                    proparr_re.get_pattern(1, &type);
                    proparr_re.get_pattern(2, &uid);
                    proparr_re.get_pattern(3, &arrSz);
                    proparr_re.get_pattern(4, &tags);
                    proparr_re.get_pattern(5, &script);

                    p.arraySize = atoi(arrSz.c_str());
                    p.isBigArray = (strstr(script.c_str(), "bigarray")) ? true : false;
                }
                else
                {
                    prop_re.get_pattern(1, &type);
                    prop_re.get_pattern(2, &uid);
                    prop_re.get_pattern(3, &tags);
                    prop_re.get_pattern(4, &script);

                    p.arraySize = 1;
                    p.isBigArray = false;
                }
                #endif

                p.uid = FastName(uid);
                //                p.scope     = ShaderProp::SCOPE_SHARED;
                p.storage = ShaderProp::STORAGE_DYNAMIC;
                p.type = ShaderProp::TYPE_FLOAT4;

                if (stricmp(type.c_str(), "float") == 0)
                {
                    p.type = ShaderProp::TYPE_FLOAT1;
                    p.precision = ShaderProp::PRECISION_NORMAL;
                }
                else if (stricmp(type.c_str(), "float2") == 0)
                {
                    p.type = ShaderProp::TYPE_FLOAT2;
                    p.precision = ShaderProp::PRECISION_NORMAL;
                }
                else if (stricmp(type.c_str(), "float3") == 0)
                {
                    p.type = ShaderProp::TYPE_FLOAT3;
                    p.precision = ShaderProp::PRECISION_NORMAL;
                }
                else if (stricmp(type.c_str(), "float4") == 0)
                {
                    p.type = ShaderProp::TYPE_FLOAT4;
                    p.precision = ShaderProp::PRECISION_NORMAL;
                }
                else if (stricmp(type.c_str(), "float4x4") == 0)
                {
                    p.type = ShaderProp::TYPE_FLOAT4X4;
                    p.precision = ShaderProp::PRECISION_NORMAL;
                }
                else if (stricmp(type.c_str(), "half") == 0)
                {
                    p.type = ShaderProp::TYPE_FLOAT1;
                    p.precision = ShaderProp::PRECISION_HALF;
                }
                else if (stricmp(type.c_str(), "half2") == 0)
                {
                    p.type = ShaderProp::TYPE_FLOAT2;
                    p.precision = ShaderProp::PRECISION_HALF;
                }
                else if (stricmp(type.c_str(), "half3") == 0)
                {
                    p.type = ShaderProp::TYPE_FLOAT3;
                    p.precision = ShaderProp::PRECISION_HALF;
                }
                else if (stricmp(type.c_str(), "half4") == 0)
                {
                    p.type = ShaderProp::TYPE_FLOAT4;
                    p.precision = ShaderProp::PRECISION_HALF;
                }
                else if (stricmp(type.c_str(), "min10float") == 0)
                {
                    p.type = ShaderProp::TYPE_FLOAT1;
                    p.precision = ShaderProp::PRECISION_LOW;
                }
                else if (stricmp(type.c_str(), "min10float2") == 0)
                {
                    p.type = ShaderProp::TYPE_FLOAT2;
                    p.precision = ShaderProp::PRECISION_LOW;
                }
                else if (stricmp(type.c_str(), "min10float3") == 0)
                {
                    p.type = ShaderProp::TYPE_FLOAT3;
                    p.precision = ShaderProp::PRECISION_LOW;
                }
                else if (stricmp(type.c_str(), "min10float4") == 0)
                {
                    p.type = ShaderProp::TYPE_FLOAT4;
                    p.precision = ShaderProp::PRECISION_LOW;
                }
                else
                {
                    Logger::Error("unknown property type (%s)", type.c_str());
                    return false;
                }

                {
                    char storage[128] = "";
                    char tag[128] = "";
                    const char* ss = strchr(tags.c_str(), ',');

                    if (ss)
                    {
                        size_t n = ss - tags.c_str();

                        strncpy(storage, tags.c_str(), n);
                        storage[n] = '\0';
                        strcpy(tag, tags.c_str() + n + 1);
                    }
                    else
                    {
                        strcpy(storage, tags.c_str());
                    }

                    //                sscanf( tags.c_str(), "%s,%s", scope, tag );
                    if (stricmp(storage, "static") == 0)
                        p.storage = ShaderProp::STORAGE_STATIC;
                    else if (stricmp(storage, "dynamic") == 0)
                        p.storage = ShaderProp::STORAGE_DYNAMIC;
                    p.tag = FastName(tag);
                }

                memset(p.defaultValue, 0, sizeof(p.defaultValue));
                if (script.length())
                {
                    const char* def_value = strstr(script.c_str(), "def_value");

                    if (def_value)
                    {
                        char val[128];

                        if (sscanf(def_value, "def_value=%s", val) == 1)
                        {
                            DAVA::Vector<DAVA::String> v;

                            DAVA::Split(val, ",", v);
                            for (uint32 i = 0; i != v.size(); ++i)
                                p.defaultValue[i] = float(atof(v[i].c_str()));
                        }
                    }
                }

                buf_t* cbuf = 0;

                for (std::vector<buf_t>::iterator b = buf.begin(), b_end = buf.end(); b != b_end; ++b)
                {
                    if (b->storage == p.storage && b->tag == p.tag)
                    {
                        cbuf = &(buf[b - buf.begin()]);
                        break;
                    }
                }

                if (!cbuf)
                {
                    buf.resize(buf.size() + 1);

                    cbuf = &(buf.back());

                    cbuf->storage = p.storage;
                    cbuf->tag = p.tag;
                    cbuf->regCount = 0;
                }

                p.bufferindex = static_cast<uint32>(cbuf - &(buf[0]));

                if (p.type == ShaderProp::TYPE_FLOAT1 || p.type == ShaderProp::TYPE_FLOAT2 || p.type == ShaderProp::TYPE_FLOAT3)
                {
                    bool do_add = true;
                    uint32 sz = 0;

                    switch (p.type)
                    {
                    case ShaderProp::TYPE_FLOAT1:
                        sz = 1;
                        break;
                    case ShaderProp::TYPE_FLOAT2:
                        sz = 2;
                        break;
                    case ShaderProp::TYPE_FLOAT3:
                        sz = 3;
                        break;
                    default:
                        break;
                    }

                    for (unsigned r = 0; r != cbuf->avlRegIndex.size(); ++r)
                    {
                        if (cbuf->avlRegIndex[r] + sz <= 4)
                        {
                            p.bufferReg = r;
                            p.bufferRegCount = cbuf->avlRegIndex[r];

                            cbuf->avlRegIndex[r] += sz;

                            do_add = false;
                            break;
                        }
                    }

                    if (do_add)
                    {
                        p.bufferReg = cbuf->regCount;
                        p.bufferRegCount = 0;

                        ++cbuf->regCount;

                        cbuf->avlRegIndex.push_back(sz);
                    }
                }
                else if (p.type == ShaderProp::TYPE_FLOAT4 || p.type == ShaderProp::TYPE_FLOAT4X4)
                {
                    p.bufferReg = cbuf->regCount;
                    p.bufferRegCount = p.arraySize * ((p.type == ShaderProp::TYPE_FLOAT4) ? 1 : 4);

                    cbuf->regCount += p.bufferRegCount;

                    for (int i = 0; i != p.bufferRegCount; ++i)
                        cbuf->avlRegIndex.push_back(4);
                }
            }
            else if (!isComment  
                        #if RHI__USE_STD_REGEX
                     && std::regex_match(line, match, fsampler2d_re) 
                        #else
                     && fsampler2d_re.test(line)
                        #endif
                     )
            {
                #if RHI__USE_STD_REGEX
                std::string sname = match[1].str();
                #else
                std::string sname;
                fsampler2d_re.get_pattern(1, &sname);
                #endif
                size_t mbegin = strstr(line, sname.c_str()) - line;
                size_t sn = sname.length();

                DVASSERT(sampler.size() < 10);
                char ch = line[mbegin + 1];
                int sl = sprintf(line + mbegin, "%u", uint32(sampler.size()));
                DVASSERT(sl >= 0 && sn >= static_cast<size_t>(sl));
                line[mbegin + 1] = ch;
                if (sn > static_cast<size_t>(sl))
                    memset(line + mbegin + sl, ' ', sn - sl);
                sampler.resize(sampler.size() + 1);
                sampler.back().uid = FastName(sname);
                sampler.back().type = TEXTURE_TYPE_2D;

                _AppendLine(line, strlen(line));
            }
            else if (!isComment  
                        #if RHI__USE_STD_REGEX
                     && std::regex_match(line, match, samplercube_re) 
                        #else
                     && samplercube_re.test(line)
                        #endif
                     )
            {
                #if RHI__USE_STD_REGEX
                std::string sname = match[1].str();
                #else
                std::string sname;
                samplercube_re.get_pattern(1, &sname);
                #endif
                size_t mbegin = strstr(line, sname.c_str()) - line;
                size_t sn = sname.length();

                DVASSERT(sampler.size() < 10);
                char ch = line[mbegin + 1];
                int sl = sprintf(line + mbegin, "%u", uint32(sampler.size()));
                DVASSERT(sl >= 0 && sn >= static_cast<size_t>(sl));
                line[mbegin + 1] = ch;
                if (sn > static_cast<size_t>(sl))
                    memset(line + mbegin + sl, ' ', sn - sl);
                sampler.resize(sampler.size() + 1);
                sampler.back().uid = FastName(sname);
                sampler.back().type = TEXTURE_TYPE_CUBE;

                _AppendLine(line, strlen(line));
            }
            else if (!isComment  
                        #if RHI__USE_STD_REGEX
                     && std::regex_match(line, match, ftexture2d_re) 
                        #else
                     && ftexture2d_re.test(line)
                        #endif
                     )
            {
                #if RHI__USE_STD_REGEX
                std::string sname = match[1].str();
                size_t mbegin = match.position(1);
                #else
                std::string sname;
                ftexture2d_re.get_pattern(1, &sname);
                size_t mbegin = ftexture2d_re.pattern(1)->begin;
                #endif
                FastName suid(sname);

                for (unsigned s = 0; s != sampler.size(); ++s)
                {
                    if (sampler[s].uid == suid)
                    {
                        int sl = sprintf(line + mbegin, "%u", s);
                        size_t sn = sname.length();
                        DVASSERT(sl >= 0 && sn >= static_cast<size_t>(sl));
                        line[mbegin + sl] = ',';
                        if (sn > static_cast<size_t>(sl))
                            memset(line + mbegin + sl, ' ', sn - sl);

                        break;
                    }
                }

                _AppendLine(line, strlen(line));
            }
            else if (!isComment  
                        #if RHI__USE_STD_REGEX
                     && std::regex_match(line, match, vsampler2d_re) 
                        #else
                     && vsampler2d_re.test(line)
                        #endif
                     )
            {
                #if RHI__USE_STD_REGEX
                std::string sname = match[1].str();
                #else
                std::string sname;
                vsampler2d_re.get_pattern(1, &sname);
                #endif
                size_t mbegin = strstr(line, sname.c_str()) - line;
                size_t sn = sname.length();

                DVASSERT(sampler.size() < 10);
                char ch = line[mbegin + 1];
                int sl = sprintf(line + mbegin, "%u", uint32(sampler.size()));
                DVASSERT(sl >= 0 && sn >= static_cast<size_t>(sl));
                line[mbegin + 1] = ch;
                if (sn > static_cast<size_t>(sl))
                    memset(line + mbegin + sl, ' ', sn - sl);
                sampler.resize(sampler.size() + 1);
                sampler.back().uid = FastName(sname);
                sampler.back().type = TEXTURE_TYPE_2D;

                _AppendLine(line, strlen(line));
            }
            else if (!isComment  
                        #if RHI__USE_STD_REGEX
                     && std::regex_match(line, match, vtexture2d_re) 
                        #else
                     && vtexture2d_re.test(line)
                        #endif
                     )
            {
                #if RHI__USE_STD_REGEX
                std::string sname = match[1].str();
                size_t mbegin = match.position(1);
                #else
                std::string sname;
                vtexture2d_re.get_pattern(1, &sname);
                size_t mbegin = vtexture2d_re.pattern(1)->begin;
                #endif
                FastName suid(sname);

                for (unsigned s = 0; s != sampler.size(); ++s)
                {
                    if (sampler[s].uid == suid)
                    {
                        int sl = sprintf(line + mbegin, "%u", s);
                        size_t sn = sname.length();
                        DVASSERT(sl >= 0 && sn >= static_cast<size_t>(sl));
                        line[mbegin + sl] = ',';
                        if (sn > static_cast<size_t>(sl))
                            memset(line + mbegin + sl, ' ', sn - sl);

                        break;
                    }
                }

                _AppendLine(line, strlen(line));
            }
            else if (!isComment  
                        #if RHI__USE_STD_REGEX
                     && std::regex_match(line, match, texturecube_re) 
                        #else
                     && texturecube_re.test(line)
                        #endif
                     )
            {
                #if RHI__USE_STD_REGEX
                std::string sname = match[1].str();
                size_t mbegin = match.position(1);
                #else
                std::string sname;
                texturecube_re.get_pattern(1, &sname);
                size_t mbegin = texturecube_re.pattern(1)->begin;
                #endif
                FastName suid(sname);

                for (unsigned s = 0; s != sampler.size(); ++s)
                {
                    if (sampler[s].uid == suid)
                    {
                        int sl = sprintf(line + mbegin, "%u", s);
                        size_t sn = sname.length();
                        DVASSERT(sl >= 0 && sn >= static_cast<size_t>(sl));
                        line[mbegin + sl] = ',';
                        if (sn > static_cast<size_t>(sl))
                            memset(line + mbegin + sl, ' ', sn - sl);

                        break;
                    }
                }

                _AppendLine(line, strlen(line));
            }
            else if (!isComment  
                        #if RHI__USE_STD_REGEX
                     && std::regex_match(line, match, blend_re) 
                        #else
                     && blend_re.test(line)
                        #endif
                     )
            {
                #if RHI__USE_STD_REGEX
                std::string mode = match[1].str();
                #else
                std::string mode;
                blend_re.get_pattern(1, &mode);
                #endif

                if (!stricmp(mode.c_str(), "alpha"))
                {
                    blending.rtBlend[0].blendEnabled = true;
                    blending.rtBlend[0].colorSrc = BLENDOP_SRC_ALPHA;
                    blending.rtBlend[0].colorDst = BLENDOP_INV_SRC_ALPHA;
                    blending.rtBlend[0].alphaSrc = BLENDOP_SRC_ALPHA;
                    blending.rtBlend[0].alphaDst = BLENDOP_INV_SRC_ALPHA;
                }
            }
            else if (!isComment  
                        #if RHI__USE_STD_REGEX
                     && std::regex_match(line, match, blending2_re) 
                        #else
                     && blending2_re.test(line)
                        #endif
                     )
            {
                #if RHI__USE_STD_REGEX
                std::string src = match[1].str();
                std::string dst = match[2].str();
                #else
                std::string src;
                blending2_re.get_pattern(1, &src);
                std::string dst;
                blending2_re.get_pattern(2, &dst);
                #endif

                blending.rtBlend[0].blendEnabled = true;
                blending.rtBlend[0].colorSrc = blending.rtBlend[0].alphaSrc = BlendOpFromText(src.c_str());
                blending.rtBlend[0].colorDst = blending.rtBlend[0].alphaDst = BlendOpFromText(dst.c_str());
            }
            else if (!isComment  
                        #if RHI__USE_STD_REGEX
                     && std::regex_match(line, match, colormask_re) 
                        #else
                     && colormask_re.test(line)
                        #endif
                     )
            {
                #if RHI__USE_STD_REGEX
                std::string mask = match[1].str();
                #else
                std::string mask;
                colormask_re.get_pattern(1, &mask);
                #endif

                if (stricmp(mask.c_str(), "all") == 0)
                    blending.rtBlend[0].writeMask = COLORMASK_ALL;
                else if (stricmp(mask.c_str(), "none") == 0)
                    blending.rtBlend[0].writeMask = COLORMASK_NONE;
                else if (stricmp(mask.c_str(), "rgb") == 0)
                    blending.rtBlend[0].writeMask = COLORMASK_R | COLORMASK_G | COLORMASK_B;
                else if (stricmp(mask.c_str(), "a") == 0)
                    blending.rtBlend[0].writeMask = COLORMASK_A;
            }
            else
            {
                _AppendLine(line, strlen(line));
            }

            if (strstr(line, "VPROG_IN_BEGIN"))
                vdecl.Clear();
            if (strstr(line, "VPROG_IN_STREAM_VERTEX"))
                vdecl.AddStream(VDF_PER_VERTEX);
            if (strstr(line, "VPROG_IN_STREAM_INSTANCE"))
                vdecl.AddStream(VDF_PER_INSTANCE);
            if (strstr(line, "VPROG_IN_POSITION"))
                vdecl.AddElement(VS_POSITION, 0, VDT_FLOAT, 3);
            if (strstr(line, "VPROG_IN_NORMAL"))
                vdecl.AddElement(VS_NORMAL, 0, VDT_FLOAT, 3);

            if (strstr(line, "VPROG_IN_TEXCOORD"))
            {
                uint32 usage_i = 0;
                uint32 data_cnt = 2;

                #if RHI__USE_STD_REGEX
                std::regex texcoord_re(".*VPROG_IN_TEXCOORD\\s*([0-7])\\s*\\(([0-7])\\s*\\).*");
                #else
                RegExp texcoord_re;
#if !(RHI__OPTIMIZED_REGEX)
                texcoord_re.compile(".*VPROG_IN_TEXCOORD\\s*([0-7])\\s*\\(([0-7])\\s*\\).*");
#else
                texcoord_re.compile("VPROG_IN_TEXCOORD\\s*([0-7])\\s*\\(\\s*([0-7])\\s*\\)");
#endif
                #endif

                if ( 
                    #if RHI__USE_STD_REGEX
                std::regex_match(line, match, texcoord_re) 
                    #else
                texcoord_re.test(line)
                    #endif
                )
                {
                    #if RHI__USE_STD_REGEX
                    std::string u = match[1].str();
                    std::string c = match[2].str();
                    #else
                    std::string u;
                    texcoord_re.get_pattern(1, &u);
                    std::string c;
                    texcoord_re.get_pattern(2, &c);
                    #endif

                    usage_i = atoi(u.c_str());
                    data_cnt = atoi(c.c_str());
                }

                vdecl.AddElement(VS_TEXCOORD, usage_i, VDT_FLOAT, data_cnt);
            }

            if (strstr(line, "VPROG_IN_COLOR"))
                vdecl.AddElement(VS_COLOR, 0, VDT_UINT8N, 4);

            if (strstr(line, "VPROG_IN_TANGENT"))
                vdecl.AddElement(VS_TANGENT, 0, VDT_FLOAT, 3);

            if (strstr(line, "VPROG_IN_BINORMAL"))
                vdecl.AddElement(VS_BINORMAL, 0, VDT_FLOAT, 3);

            if (strstr(line, "VPROG_IN_BLENDINDEX"))
            {
                uint32 data_cnt = 1;
                #if RHI__USE_STD_REGEX
                std::regex index_re(".*VPROG_IN_BLENDINDEX\\s*\\(([0-7])\\s*\\).*");
                #else
                RegExp index_re;
#if !(RHI__OPTIMIZED_REGEX)
                index_re.compile(".*VPROG_IN_BLENDINDEX\\s*\\(([0-7])\\s*\\).*");
#else
                index_re.compile("VPROG_IN_BLENDINDEX\\s*\\(\\s*([0-7])\\s*\\)");
#endif
                #endif

                if ( 
                    #if RHI__USE_STD_REGEX
                std::regex_match(line, match, index_re) 
                    #else
                index_re.test(line)
                    #endif
                )
                {
                    #if RHI__USE_STD_REGEX
                    std::string c = match[1].str();
                    #else
                    std::string c;
                    index_re.get_pattern(1, &c);
                    #endif

                    data_cnt = atoi(c.c_str());
                }

                vdecl.AddElement(VS_BLENDINDEX, 0, VDT_FLOAT, data_cnt);
            }

        } // for each line
        STOP_NAMED_TIMING("shadersrc.parse");

        type = progType;

        success = true;
    }

    if (success)
    {
        START_NAMED_TIMING("shadersrc.gen");
        // check if any const-buffer has more than one bigarray-prop

        for (size_t b = 0, b_end = buf.size(); b != b_end; ++b)
        {
            unsigned bigarray_cnt = 0;

            for (std::vector<ShaderProp>::iterator p = prop.begin(), p_end = prop.end(); p != p_end; ++p)
            {
                if (p->isBigArray && p->bufferindex == b)
                {
                    if (++bigarray_cnt > 1)
                        break;
                }
            }

            if (bigarray_cnt > 1)
            {
                DVASSERT(bigarray_cnt <= 1);
                return false;
            }
        }

        // generate prop-var definitions

        const char* prog_begin = (progType == PROG_VERTEX) ? "VPROG_BEGIN" : "FPROG_BEGIN";
        const char* prog = strstr(code.c_str(), prog_begin);

        if (prog)
        {
            char buf_def[1024];
            int buf_len = 0;
            char var_def[8 * 1024];
            int var_len = 0;
            char pt = (progType == PROG_VERTEX) ? 'V' : 'F';
            unsigned reg = 0;

            buf_len += Snprintf(buf_def + buf_len, sizeof(buf_def) - buf_len, "//--------\n");
            for (unsigned i = 0; i != buf.size(); ++i)
            {
                buf_len += Snprintf(buf_def + buf_len, sizeof(buf_def) - buf_len, "DECL_%cPROG_BUFFER(%u,%u,%u)\n", pt, i, buf[i].regCount, reg);
                reg += buf[i].regCount;
            }
            buf_len += Snprintf(buf_def + buf_len, sizeof(buf_def) - buf_len, "\n\n");

            var_len += Snprintf(var_def + var_len, sizeof(var_def) - var_len, "    //--------\n");
            for (std::vector<ShaderProp>::const_iterator p = prop.begin(), p_end = prop.end(); p != p_end; ++p)
            {
                switch (p->type)
                {
                case ShaderProp::TYPE_FLOAT1:
                {
                    const char* xyzw = "xyzw";
                    //                        var_len += Snprintf( var_def+var_len, sizeof(var_def)-var_len, "    float %s = %cP_Buffer%u[%u].%c;\n", p->uid.c_str(), pt, p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount] );
                    var_len += Snprintf(
                    var_def + var_len, sizeof(var_def) - var_len,
                    "    #define %s  float4(%cP_Buffer%u[%u]).%c\n",
                    p->uid.c_str(), pt, p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount]);
                }
                break;

                case ShaderProp::TYPE_FLOAT2:
                {
                    const char* xyzw = "xyzw";
                    var_len += Snprintf(
                    var_def + var_len, sizeof(var_def) - var_len,
                    //                            "    float2 %s = float2( %cP_Buffer%u[%u].%c, %cP_Buffer%u[%u].%c );\n",      k
                    //                    "    float2 %s = float2( float4(%cP_Buffer%u[%u]).%c, float4(%cP_Buffer%u[%u]).%c );\n",
                    "    #define %s  float2( float4(%cP_Buffer%u[%u]).%c, float4(%cP_Buffer%u[%u]).%c )\n",
                    p->uid.c_str(),
                    pt, p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount + 0],
                    pt, p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount + 1]);
                }
                break;

                case ShaderProp::TYPE_FLOAT3:
                {
                    const char* xyzw = "xyzw";
                    var_len += Snprintf(
                    var_def + var_len, sizeof(var_def) - var_len,
                    //                            "    float3 %s = float3( %cP_Buffer%u[%u].%c, %cP_Buffer%u[%u].%c, %cP_Buffer%u[%u].%c );\n",
                    //                    "    float3 %s = float3( float4(%cP_Buffer%u[%u]).%c, float4(%cP_Buffer%u[%u]).%c, float4(%cP_Buffer%u[%u]).%c );\n",
                    "    #define %s  float3( float4(%cP_Buffer%u[%u]).%c, float4(%cP_Buffer%u[%u]).%c, float4(%cP_Buffer%u[%u]).%c )\n",
                    p->uid.c_str(),
                    pt, p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount + 0],
                    pt, p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount + 1],
                    pt, p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount + 2]);
                }
                break;

                case ShaderProp::TYPE_FLOAT4:
                {
                    if (p->arraySize == 1)
                    {
                        var_len += Snprintf(
                        var_def + var_len, sizeof(var_def) - var_len,
                        "    #define %s  float4(%cP_Buffer%u[%u])\n",
                        p->uid.c_str(), pt, p->bufferindex, p->bufferReg);
                    }
                    else
                    {
                        if (p->isBigArray)
                        {
                            var_len += Snprintf(var_def + var_len, sizeof(var_def) - var_len, "    #define %s %cP_Buffer%u\n", p->uid.c_str(), pt, p->bufferindex);
                        }
                        else
                        {
                            var_len += Snprintf(var_def + var_len, sizeof(var_def) - var_len, "    float4 %s[%u];\n", p->uid.c_str(), p->arraySize);
                            for (unsigned i = 0; i != p->arraySize; ++i)
                            {
                                var_len += Snprintf(var_def + var_len, sizeof(var_def) - var_len, "      %s[%u] = %cP_Buffer%u[%u];\n", p->uid.c_str(), i, pt, p->bufferindex, p->bufferReg + i);
                            }
                        }
                    }
                }
                break;

                case ShaderProp::TYPE_FLOAT4X4:
                {
                    var_len += Snprintf(
                    var_def + var_len, sizeof(var_def) - var_len,
                    "    #define %s float4x4( %cP_Buffer%u[%u], %cP_Buffer%u[%u], %cP_Buffer%u[%u], %cP_Buffer%u[%u] )\n",
                    p->uid.c_str(),
                    pt, p->bufferindex, p->bufferReg + 0,
                    pt, p->bufferindex, p->bufferReg + 1,
                    pt, p->bufferindex, p->bufferReg + 2,
                    pt, p->bufferindex, p->bufferReg + 3);
                }
                break;
                };
            }
            var_len += Snprintf(var_def + var_len, sizeof(var_def) - var_len, "    //--------\n");
            var_len += Snprintf(var_def + var_len, sizeof(var_def) - var_len, "\n\n");
            size_t var_pos = (prog - code.c_str()) + strlen("XPROG_BEGIN") + buf_len + 2;

            code.insert(prog - code.c_str(), buf_def, buf_len);
            code.insert(var_pos, var_def, var_len);
        }
        STOP_NAMED_TIMING("shadersrc.gen");
    }
*/
    return success;
}

//------------------------------------------------------------------------------

void
ShaderSource::_ProcessMetaData(sl::HLSLTree* ast)
{
    struct
    prop_t
    {
        sl::HLSLDeclaration* decl;
        sl::HLSLStatement* prev_statement;
    };

    std::vector<prop_t> prop_decl;
    char btype = 'x';

    if (ast->FindGlobalStruct("vertex_in"))
        btype = 'V';
    else if (ast->FindGlobalStruct("fragment_in"))
        btype = 'F';

    // find properties/samplers
    {
        sl::HLSLStatement* statement = ast->GetRoot()->statement;
        sl::HLSLStatement* pstatement = NULL;
        unsigned sampler_reg = 0;

        while (statement)
        {
            if (statement->nodeType == sl::HLSLNodeType_Declaration)
            {
                sl::HLSLDeclaration* decl = (sl::HLSLDeclaration*)statement;

                if (decl->type.flags & sl::HLSLTypeFlag_Property)
                {
                    property.resize(property.size() + 1);
                    rhi::ShaderProp& prop = property.back();

                    prop.uid = DAVA::FastName(decl->name);
                    prop.storage = rhi::ShaderProp::STORAGE_DYNAMIC;
                    prop.precision = rhi::ShaderProp::PRECISION_NORMAL;
                    prop.arraySize = 1;
                    prop.isBigArray = false;

                    switch (decl->type.baseType)
                    {
                    case sl::HLSLBaseType_Float:
                        prop.type = rhi::ShaderProp::TYPE_FLOAT1;
                        break;
                    case sl::HLSLBaseType_Float2:
                        prop.type = rhi::ShaderProp::TYPE_FLOAT2;
                        break;
                    case sl::HLSLBaseType_Float3:
                        prop.type = rhi::ShaderProp::TYPE_FLOAT3;
                        break;
                    case sl::HLSLBaseType_Float4:
                        prop.type = rhi::ShaderProp::TYPE_FLOAT4;
                        break;
                    case sl::HLSLBaseType_Float4x4:
                        prop.type = rhi::ShaderProp::TYPE_FLOAT4X4;
                        break;
                    }

                    for (sl::HLSLAttribute* a = decl->attributes; a; a = a->nextAttribute)
                    {
                        if (stricmp(a->attrText, "static") == 0 || stricmp(a->attrText, "statik") == 0)
                            prop.storage = rhi::ShaderProp::STORAGE_STATIC;
                        else if (stricmp(a->attrText, "dynamic") == 0)
                            prop.storage = rhi::ShaderProp::STORAGE_DYNAMIC;
                        else
                            prop.tag = FastName(a->attrText);
                    }

                    if (decl->assignment)
                    {
                        if (decl->assignment->nodeType == sl::HLSLNodeType_ConstructorExpression)
                        {
                            sl::HLSLConstructorExpression* ctor = (sl::HLSLConstructorExpression*)(decl->assignment);
                            float val[4] = { 0, 0, 0, 0 };
                            unsigned val_i = 0;

                            for (sl::HLSLExpression *arg = ctor->argument; arg; arg = arg->nextExpression, ++val_i)
                            {
                                if (arg->nodeType == sl::HLSLNodeType_LiteralExpression)
                                {
                                    sl::HLSLLiteralExpression* expr = (sl::HLSLLiteralExpression*)(arg);

                                    if (expr->type == sl::HLSLBaseType_Float)
                                        val[val_i] = expr->fValue;
                                    else if (expr->type == sl::HLSLBaseType_Int)
                                        val[val_i] = float(expr->iValue);
                                }
                            }
                        }
                    }

                    buf_t* cbuf = nullptr;

                    for (std::vector<buf_t>::iterator b = buf.begin(), b_end = buf.end(); b != b_end; ++b)
                    {
                        if (b->storage == prop.storage && b->tag == prop.tag)
                        {
                            cbuf = &(buf[b - buf.begin()]);
                            break;
                        }
                    }

                    if (!cbuf)
                    {
                        buf.resize(buf.size() + 1);

                        cbuf = &(buf.back());

                        cbuf->storage = prop.storage;
                        cbuf->tag = prop.tag;
                        cbuf->regCount = 0;
                    }

                    prop.bufferindex = static_cast<uint32>(cbuf - &(buf[0]));

                    if (prop.type == rhi::ShaderProp::TYPE_FLOAT1 || prop.type == rhi::ShaderProp::TYPE_FLOAT2 || prop.type == rhi::ShaderProp::TYPE_FLOAT3)
                    {
                        bool do_add = true;
                        uint32 sz = 0;

                        switch (prop.type)
                        {
                        case rhi::ShaderProp::TYPE_FLOAT1:
                            sz = 1;
                            break;
                        case rhi::ShaderProp::TYPE_FLOAT2:
                            sz = 2;
                            break;
                        case rhi::ShaderProp::TYPE_FLOAT3:
                            sz = 3;
                            break;
                        default:
                            break;
                        }

                        for (unsigned r = 0; r != cbuf->avlRegIndex.size(); ++r)
                        {
                            if (cbuf->avlRegIndex[r] + sz <= 4)
                            {
                                prop.bufferReg = r;
                                prop.bufferRegCount = cbuf->avlRegIndex[r];

                                cbuf->avlRegIndex[r] += sz;

                                do_add = false;
                                break;
                            }
                        }

                        if (do_add)
                        {
                            prop.bufferReg = cbuf->regCount;
                            prop.bufferRegCount = 0;

                            ++cbuf->regCount;

                            cbuf->avlRegIndex.push_back(sz);
                        }
                    }
                    else if (prop.type == rhi::ShaderProp::TYPE_FLOAT4 || prop.type == rhi::ShaderProp::TYPE_FLOAT4X4)
                    {
                        prop.bufferReg = cbuf->regCount;
                        prop.bufferRegCount = prop.arraySize * ((prop.type == rhi::ShaderProp::TYPE_FLOAT4) ? 1 : 4);

                        cbuf->regCount += prop.bufferRegCount;

                        for (int i = 0; i != prop.bufferRegCount; ++i)
                            cbuf->avlRegIndex.push_back(4);
                    }

                    prop_t pp;

                    pp.decl = decl;
                    pp.prev_statement = pstatement;

                    prop_decl.push_back(pp);
                }

                if (decl->type.baseType == sl::HLSLBaseType_Sampler2D
                    || decl->type.baseType == sl::HLSLBaseType_SamplerCube
                    )
                {
                    sampler.resize(sampler.size() + 1);
                    rhi::ShaderSampler& s = sampler.back();

                    char regName[128];

                    switch (decl->type.baseType)
                    {
                    case sl::HLSLBaseType_Sampler2D:
                        s.type = rhi::TEXTURE_TYPE_2D;
                        break;
                    case sl::HLSLBaseType_SamplerCube:
                        s.type = rhi::TEXTURE_TYPE_CUBE;
                        break;
                    }
                    s.uid = FastName(decl->name);
                    Snprintf(regName, sizeof(regName), "s%u", sampler_reg);
                    ++sampler_reg;
                    decl->registerName = ast->AddString(regName);
                }
            }

            pstatement = statement;
            statement = statement->nextStatement;
        }
    }

    // rename vertex-input variables to pre-defined names
    {
        sl::HLSLStruct* vinput = ast->FindGlobalStruct("vertex_in");
        if (vinput)
        {
            const char* vertex_in = ast->AddString("vertex_in");

            class Replacer : public sl::HLSLTreeVisitor
            {
            public:
                sl::HLSLStructField* field;
                const char* new_name;
                const char* vertex_in;
                virtual void VisitMemberAccess(sl::HLSLMemberAccess* node)
                {
                    if (node->field == field->name
                        && node->object->expressionType.baseType == sl::HLSLBaseType_UserDefined
                        && node->object->expressionType.typeName == vertex_in
                        )
                    {
                        node->field = new_name;
                    }

                    sl::HLSLTreeVisitor::VisitMemberAccess(node);
                }
                void Replace(sl::HLSLTree* ast, sl::HLSLStructField* field_, const char* new_name_)
                {
                    field = field_;
                    new_name = new_name_;
                    VisitRoot(ast->GetRoot());

                    field->name = new_name_;
                }
            };

            Replacer r;

            r.vertex_in = vertex_in;

            struct
            {
                const char* semantic;
                const char* attr_name;
            } attr[] =
            {
              { "POSITION", "position" },
              { "NORMAL", "normal" },
              { "TEXCOORD", "texcoord0" },
              { "TEXCOORD0", "texcoord0" },
              { "TEXCOORD1", "texcoord1" },
              { "TEXCOORD2", "texcoord2" },
              { "TEXCOORD3", "texcoord3" },
              { "TEXCOORD4", "texcoord4" },
              { "TEXCOORD5", "texcoord5" },
              { "TEXCOORD6", "texcoord6" },
              { "TEXCOORD7", "texcoord7" },
              { "COLOR", "color0" },
              { "COLOR0", "color0" },
              { "COLOR1", "color1" },
              { "TANGENT", "tangent" },
              { "BINORMAL", "binormal" },
              { "BLENDWEIGHT", "blendweight" },
              { "BLENDINDICES", "blendindex" }
            };

            for (sl::HLSLStructField* field = vinput->field; field; field = field->nextField)
            {
                if (field->semantic)
                {
                    for (unsigned a = 0; a != countof(attr); ++a)
                    {
                        if (stricmp(field->semantic, attr[a].semantic) == 0)
                        {
                            r.Replace(ast, field, ast->AddString(attr[a].attr_name));
                            break;
                        }
                    }
                }
            }
        }
    }

    // get vertex-layout
    {
        sl::HLSLStruct* input = ast->FindGlobalStruct("vertex_in");

        if (input)
        {
            struct
            {
                rhi::VertexSemantics usage;
                const char* semantic;
            }
            semantic[] =
            {
              { rhi::VS_POSITION, "POSITION" },
              { rhi::VS_NORMAL, "NORMAL" },
              { rhi::VS_COLOR, "COLOR" },
              { rhi::VS_TEXCOORD, "TEXCOORD" },
              { rhi::VS_TANGENT, "TANGENT" },
              { rhi::VS_BINORMAL, "BINORMAL" },
              { rhi::VS_BLENDWEIGHT, "BLENDWEIGHT" },
              { rhi::VS_BLENDINDEX, "BLENDINDICES" }
            };

            vertexLayout.Clear();

            rhi::VertexDataFrequency cur_freq = rhi::VDF_PER_VERTEX;

            for (sl::HLSLStructField* field = input->field; field; field = field->nextField)
            {
                rhi::VertexSemantics usage;
                unsigned usage_i = 0;
                rhi::VertexDataType data_type = rhi::VDT_FLOAT;
                unsigned data_count = 0;
                rhi::VertexDataFrequency freq = rhi::VDF_PER_VERTEX;

                switch (field->type.baseType)
                {
                case sl::HLSLBaseType_Float4:
                    data_type = rhi::VDT_FLOAT;
                    data_count = 4;
                    break;
                case sl::HLSLBaseType_Float3:
                    data_type = rhi::VDT_FLOAT;
                    data_count = 3;
                    break;
                case sl::HLSLBaseType_Float2:
                    data_type = rhi::VDT_FLOAT;
                    data_count = 2;
                    break;
                case sl::HLSLBaseType_Float:
                    data_type = rhi::VDT_FLOAT;
                    data_count = 1;
                    break;
                case sl::HLSLBaseType_Uint4:
                    data_type = rhi::VDT_UINT8;
                    data_count = 4;
                    break;
                }

                char sem[128];

                strcpy(sem, field->semantic);
                for (char* s = sem; *s; ++s)
                    *s = toupper(*s);

                for (unsigned i = 0; i != countof(semantic); ++i)
                {
                    const char* t = strstr(sem, semantic[i].semantic);

                    if (t == sem)
                    {
                        const char* tu = sem + strlen(semantic[i].semantic);

                        usage = semantic[i].usage;
                        usage_i = atoi(tu);

                        break;
                    }
                }

                if (usage == rhi::VS_COLOR)
                {
                    data_type = rhi::VDT_UINT8N;
                    data_count = 4;
                }

                if (field->attribute)
                {
                    if (stricmp(field->attribute->attrText, "vertex") == 0)
                        freq = rhi::VDF_PER_VERTEX;
                    else if (stricmp(field->attribute->attrText, "instance") == 0)
                        freq = rhi::VDF_PER_INSTANCE;
                }

                if (freq != cur_freq)
                    vertexLayout.AddStream(freq);
                cur_freq = freq;

                vertexLayout.AddElement(usage, usage_i, data_type, data_count);
            }

            //-            Logger::Info("vertex-layout:");
            //-            vertexLayout.Dump();
        }
    }

    if (prop_decl.size())
    {
        std::vector<sl::HLSLBuffer*> cbuf_decl;

        cbuf_decl.resize(buf.size());
        for (unsigned i = 0; i != buf.size(); ++i)
        {
            sl::HLSLBuffer* cbuf = ast->AddNode<sl::HLSLBuffer>(prop_decl[0].decl->fileName, prop_decl[0].decl->line);
            sl::HLSLDeclaration* decl = ast->AddNode<sl::HLSLDeclaration>(prop_decl[0].decl->fileName, prop_decl[0].decl->line);
            sl::HLSLLiteralExpression* sz = ast->AddNode<sl::HLSLLiteralExpression>(prop_decl[0].decl->fileName, prop_decl[0].decl->line);
            ;
            char buf_name[128];
            char buf_type_name[128];
            char buf_reg_name[128];

            Snprintf(buf_name, sizeof(buf_name), "%cP_Buffer%u", btype, i);
            Snprintf(buf_type_name, sizeof(buf_name), "%cP_Buffer%u_t", btype, i);
            Snprintf(buf_reg_name, sizeof(buf_name), "b%u", i);

            decl->name = ast->AddString(buf_name);
            decl->type.baseType = sl::HLSLBaseType_Float4;
            decl->type.array = true;
            decl->type.arraySize = sz;

            sz->type = sl::HLSLBaseType_Int;
            sz->iValue = buf[i].regCount;

            cbuf->field = decl;
            cbuf->name = ast->AddString(buf_type_name);
            cbuf->registerName = ast->AddString(buf_reg_name);
            cbuf->registerCount = buf[i].regCount;

            cbuf_decl[i] = cbuf;
        }

        for (unsigned i = 0; i != cbuf_decl.size() - 1; ++i)
            cbuf_decl[i]->nextStatement = cbuf_decl[i + 1];

        prop_decl[0].prev_statement->nextStatement = cbuf_decl[0];
        cbuf_decl[cbuf_decl.size() - 1]->nextStatement = prop_decl[0].decl;

        #define DO_FLOAT4_CAST 1

        DVASSERT(property.size() == prop_decl.size());
        for (unsigned i = 0; i != property.size(); ++i)
        {
            switch (property[i].type)
            {
            case rhi::ShaderProp::TYPE_FLOAT4:
            {
                sl::HLSLArrayAccess* arr_access = ast->AddNode<sl::HLSLArrayAccess>(prop_decl[i].decl->fileName, prop_decl[i].decl->line);
                sl::HLSLLiteralExpression* idx = ast->AddNode<sl::HLSLLiteralExpression>(prop_decl[0].decl->fileName, prop_decl[0].decl->line);
                sl::HLSLIdentifierExpression* arr = ast->AddNode<sl::HLSLIdentifierExpression>(prop_decl[0].decl->fileName, prop_decl[0].decl->line);
                char buf_name[128];

                Snprintf(buf_name, sizeof(buf_name), "%cP_Buffer%u", btype, property[i].bufferindex);
                arr->name = ast->AddString(buf_name);
                arr->global = true;

                idx->type = sl::HLSLBaseType_Int;
                idx->iValue = property[i].bufferReg;

                arr_access->array = arr;
                arr_access->index = idx;
                    
                    #if DO_FLOAT4_CAST
                sl::HLSLCastingExpression* cast_expr = ast->AddNode<sl::HLSLCastingExpression>(prop_decl[i].decl->fileName, prop_decl[i].decl->line);
                cast_expr->expression = arr_access;
                cast_expr->type.baseType = sl::HLSLBaseType_Float4;

                prop_decl[i].decl->assignment = cast_expr;
                prop_decl[i].decl->type.flags |= sl::HLSLTypeFlag_Static | sl::HLSLTypeFlag_Property;
                    #else
                prop_decl[i].decl->assignment = arr_access;
                prop_decl[i].decl->type.flags |= sl::HLSLTypeFlag_Static | sl::HLSLTypeFlag_Property;
                    #endif
            }
            break;

            case rhi::ShaderProp::TYPE_FLOAT3:
            case rhi::ShaderProp::TYPE_FLOAT2:
            case rhi::ShaderProp::TYPE_FLOAT1:
            {
                sl::HLSLMemberAccess* member_access = ast->AddNode<sl::HLSLMemberAccess>(prop_decl[i].decl->fileName, prop_decl[i].decl->line);
                char xyzw[] = { 'x', 'y', 'z', 'w', '\0' };
                unsigned elem_cnt = 0;
                sl::HLSLArrayAccess* arr_access = ast->AddNode<sl::HLSLArrayAccess>(prop_decl[i].decl->fileName, prop_decl[i].decl->line);
                sl::HLSLLiteralExpression* idx = ast->AddNode<sl::HLSLLiteralExpression>(prop_decl[0].decl->fileName, prop_decl[0].decl->line);
                sl::HLSLIdentifierExpression* arr = ast->AddNode<sl::HLSLIdentifierExpression>(prop_decl[0].decl->fileName, prop_decl[0].decl->line);
                char buf_name[128];

                switch (property[i].type)
                {
                case rhi::ShaderProp::TYPE_FLOAT1:
                    elem_cnt = 1;
                    break;
                case rhi::ShaderProp::TYPE_FLOAT2:
                    elem_cnt = 2;
                    break;
                case rhi::ShaderProp::TYPE_FLOAT3:
                    elem_cnt = 3;
                    break;
                }

                member_access->object = arr_access;
                xyzw[property[i].bufferRegCount + elem_cnt] = 0;
                member_access->field = ast->AddString(xyzw + property[i].bufferRegCount);

                Snprintf(buf_name, sizeof(buf_name), "%cP_Buffer%u", btype, property[i].bufferindex);
                arr->name = ast->AddString(buf_name);
                arr->global = true;

                idx->type = sl::HLSLBaseType_Int;
                idx->iValue = property[i].bufferReg;

                arr_access->array = arr;
                arr_access->index = idx;

                    #if DO_FLOAT4_CAST
                sl::HLSLCastingExpression* cast_expr = ast->AddNode<sl::HLSLCastingExpression>(prop_decl[i].decl->fileName, prop_decl[i].decl->line);
                cast_expr->expression = arr_access;
                cast_expr->type.baseType = sl::HLSLBaseType_Float4;
                member_access->object = cast_expr;

                prop_decl[i].decl->assignment = member_access;
                prop_decl[i].decl->type.flags |= sl::HLSLTypeFlag_Static | sl::HLSLTypeFlag_Property;
                    #else
                prop_decl[i].decl->assignment = member_access;
                prop_decl[i].decl->type.flags |= sl::HLSLTypeFlag_Static | sl::HLSLTypeFlag_Property;
                    #endif
            }
            break;

            case rhi::ShaderProp::TYPE_FLOAT4X4:
            {
                sl::HLSLConstructorExpression* ctor = ast->AddNode<sl::HLSLConstructorExpression>(prop_decl[i].decl->fileName, prop_decl[i].decl->line);
                sl::HLSLArrayAccess* arr_access[4];
                sl::HLSLCastingExpression* cast_expr[4];

                ctor->type.baseType = sl::HLSLBaseType_Float4x4;

                for (unsigned k = 0; k != 4; ++k)
                {
                    arr_access[k] = ast->AddNode<sl::HLSLArrayAccess>(prop_decl[i].decl->fileName, prop_decl[i].decl->line);

                    sl::HLSLLiteralExpression* idx = ast->AddNode<sl::HLSLLiteralExpression>(prop_decl[0].decl->fileName, prop_decl[0].decl->line);
                    sl::HLSLIdentifierExpression* arr = ast->AddNode<sl::HLSLIdentifierExpression>(prop_decl[0].decl->fileName, prop_decl[0].decl->line);
                    char buf_name[128];

                    Snprintf(buf_name, sizeof(buf_name), "%cP_Buffer%u", btype, property[i].bufferindex);
                    arr->name = ast->AddString(buf_name);

                    idx->type = sl::HLSLBaseType_Int;
                    idx->iValue = property[i].bufferReg + k;

                    arr_access[k]->array = arr;
                    arr_access[k]->index = idx;
                }

                    #if DO_FLOAT4_CAST
                for (unsigned k = 0; k != 4; ++k)
                {
                    cast_expr[k] = ast->AddNode<sl::HLSLCastingExpression>(prop_decl[i].decl->fileName, prop_decl[i].decl->line);
                    cast_expr[k]->expression = arr_access[k];
                    cast_expr[k]->type.baseType = sl::HLSLBaseType_Float4;
                }

                ctor->argument = cast_expr[0];
                for (unsigned k = 0; k != 4 - 1; ++k)
                    cast_expr[k]->nextExpression = cast_expr[k + 1];
                    #else
                ctor->argument = arr_access[0];
                for (unsigned k = 0; k != 4 - 1; ++k)
                    arr_access[k]->nextExpression = arr_access[k + 1];
                    #endif

                prop_decl[i].decl->assignment = ctor;
                prop_decl[i].decl->type.flags |= sl::HLSLTypeFlag_Static | sl::HLSLTypeFlag_Property;
            }
            break;
            }
        }
    }

    // get blending

    sl::HLSLBlend* blend = ast->GetRoot()->blend;

    if (blend)
    {
        blending.rtBlend[0].blendEnabled = !(blend->src_op == sl::BLENDOP_ONE && blend->dst_op == sl::BLENDOP_ZERO);

        switch (blend->src_op)
        {
        case sl::BLENDOP_ZERO:
            blending.rtBlend[0].colorSrc = rhi::BLENDOP_ZERO;
            ;
            break;
        case sl::BLENDOP_ONE:
            blending.rtBlend[0].colorSrc = rhi::BLENDOP_ONE;
            ;
            break;
        case sl::BLENDOP_SRC_ALPHA:
            blending.rtBlend[0].colorSrc = rhi::BLENDOP_SRC_ALPHA;
            ;
            break;
        case sl::BLENDOP_INV_SRC_ALPHA:
            blending.rtBlend[0].colorSrc = rhi::BLENDOP_INV_SRC_ALPHA;
            ;
            break;
        case sl::BLENDOP_SRC_COLOR:
            blending.rtBlend[0].colorSrc = rhi::BLENDOP_SRC_COLOR;
            ;
            break;
        case sl::BLENDOP_DST_COLOR:
            blending.rtBlend[0].colorSrc = rhi::BLENDOP_DST_COLOR;
            ;
            break;
        }
        switch (blend->dst_op)
        {
        case sl::BLENDOP_ZERO:
            blending.rtBlend[0].colorDst = rhi::BLENDOP_ZERO;
            ;
            break;
        case sl::BLENDOP_ONE:
            blending.rtBlend[0].colorDst = rhi::BLENDOP_ONE;
            ;
            break;
        case sl::BLENDOP_SRC_ALPHA:
            blending.rtBlend[0].colorDst = rhi::BLENDOP_SRC_ALPHA;
            ;
            break;
        case sl::BLENDOP_INV_SRC_ALPHA:
            blending.rtBlend[0].colorDst = rhi::BLENDOP_INV_SRC_ALPHA;
            ;
            break;
        case sl::BLENDOP_SRC_COLOR:
            blending.rtBlend[0].colorDst = rhi::BLENDOP_SRC_COLOR;
            ;
            break;
        case sl::BLENDOP_DST_COLOR:
            blending.rtBlend[0].colorDst = rhi::BLENDOP_DST_COLOR;
            ;
            break;
        }
    }

    // get color write-mask

    sl::HLSLColorMask* mask = ast->GetRoot()->color_mask;

    if (mask)
    {
        switch (mask->mask)
        {
        case sl::COLORMASK_NONE:
            blending.rtBlend[0].writeMask = rhi::COLORMASK_NONE;
            break;
        case sl::COLORMASK_ALL:
            blending.rtBlend[0].writeMask = rhi::COLORMASK_ALL;
            break;
        case sl::COLORMASK_RGB:
            blending.rtBlend[0].writeMask = rhi::COLORMASK_R | rhi::COLORMASK_G | rhi::COLORMASK_B;
            break;
        case sl::COLORMASK_A:
            blending.rtBlend[0].writeMask = rhi::COLORMASK_A;
            break;
        }
    }

#if 0
    Logger::Info("properties (%u) :", property.size());
    for (std::vector<rhi::ShaderProp>::const_iterator p = property.begin(), p_end = property.end(); p != p_end; ++p)
    {
        if (p->type == rhi::ShaderProp::TYPE_FLOAT4 || p->type == rhi::ShaderProp::TYPE_FLOAT4X4)
        {
            if (p->arraySize == 1)
            {
                Logger::Info("  %-16s    buf#%u  -  %u, %u x float4", p->uid.c_str(), p->bufferindex, p->bufferReg, p->bufferRegCount);
            }
            else
            {
                char name[128];

                Snprintf(name, sizeof(name) - 1, "%s[%u]", p->uid.c_str(), p->arraySize);
                Logger::Info("  %-16s    buf#%u  -  %u, %u x float4", name, p->bufferindex, p->bufferReg, p->bufferRegCount);
            }
        }
        else
        {
            const char* xyzw = "xyzw";

            switch (p->type)
            {
            case rhi::ShaderProp::TYPE_FLOAT1:
                Logger::Info("  %-16s    buf#%u  -  %u, %c", p->uid.c_str(), p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount]);
                break;

            case rhi::ShaderProp::TYPE_FLOAT2:
                Logger::Info("  %-16s    buf#%u  -  %u, %c%c", p->uid.c_str(), p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount + 0], xyzw[p->bufferRegCount + 1]);
                break;

            case rhi::ShaderProp::TYPE_FLOAT3:
                Logger::Info("  %-16s    buf#%u  -  %u, %c%c%c", p->uid.c_str(), p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount + 0], xyzw[p->bufferRegCount + 1], xyzw[p->bufferRegCount + 2]);
                break;

            default:
                break;
            }
        }
    }

    Logger::Info("\n--- ShaderSource");
    Logger::Info("buffers (%u) :", buf.size());
    for (unsigned i = 0; i != buf.size(); ++i)
    {
        Logger::Info("  buf#%u  reg.count = %u", i, buf[i].regCount);
    }

    Logger::Info("samplers (%u) :", sampler.size());
    for (unsigned s = 0; s != sampler.size(); ++s)
    {
        Logger::Info("  sampler#%u  \"%s\"", s, sampler[s].uid.c_str());
    }
    Logger::Info("\n\n");
#endif
};

//------------------------------------------------------------------------------

static inline bool
ReadUI1(DAVA::File* f, uint8* x)
{
    return (f->Read(x) == sizeof(uint8));
}

static inline bool
ReadUI4(DAVA::File* f, uint32* x)
{
    return (f->Read(x) == sizeof(uint32));
}

static inline bool
ReadS0(DAVA::File* f, std::string* str)
{
    char s0[128 * 1024];
    uint32 sz = 0;
    if (ReadUI4(f, &sz))
    {
        if (f->Read(s0, sz) == sz)
        {
            *str = s0;
            return true;
        }
    }
    return false;
}

bool ShaderSource::Load(DAVA::File* in)
{
#define READ_CHECK(exp) if (!(exp)) { return false; }
    return false;
    /*
    std::string s0;
    uint32 readUI4;
    uint8 readUI1;

    _Reset();

    READ_CHECK(ReadUI4(in, &readUI4));
    type = ProgType(readUI4);

    READ_CHECK(ReadS0(in, &code));

    READ_CHECK(vertexLayout.Load(in));

    READ_CHECK(ReadUI4(in, &readUI4));
    property.resize(readUI4);
    for (unsigned p = 0; p != property.size(); ++p)
    {
        READ_CHECK(ReadS0(in, &s0));
        property[p].uid = FastName(s0.c_str());

        READ_CHECK(ReadS0(in, &s0));
        property[p].tag = FastName(s0.c_str());

        READ_CHECK(ReadUI4(in, &readUI4));
        property[p].type = ShaderProp::Type(readUI4);

        READ_CHECK(ReadUI4(in, &readUI4));
        property[p].storage = ShaderProp::Storage(readUI4);

        READ_CHECK(ReadUI4(in, &readUI4));
        property[p].isBigArray = readUI4;

        READ_CHECK(ReadUI4(in, &property[p].arraySize));
        READ_CHECK(ReadUI4(in, &property[p].bufferindex));
        READ_CHECK(ReadUI4(in, &property[p].bufferReg));
        READ_CHECK(ReadUI4(in, &property[p].bufferRegCount));

        READ_CHECK(in->Read(property[p].defaultValue, 16 * sizeof(float)) == 16 * sizeof(float));
    }

    READ_CHECK(ReadUI4(in, &readUI4));
    buf.resize(readUI4);
    for (unsigned b = 0; b != buf.size(); ++b)
    {
        READ_CHECK(ReadUI4(in, &readUI4));
        buf[b].storage = ShaderProp::Storage(readUI4);

        READ_CHECK(ReadS0(in, &s0));
        buf[b].tag = FastName(s0.c_str());

        READ_CHECK(ReadUI4(in, &buf[b].regCount));
    }

    READ_CHECK(ReadUI4(in, &readUI4));
    sampler.resize(readUI4);
    for (unsigned s = 0; s != sampler.size(); ++s)
    {
        READ_CHECK(ReadUI4(in, &readUI4));
        sampler[s].type = TextureType(readUI4);

        READ_CHECK(ReadS0(in, &s0));
        sampler[s].uid = FastName(s0.c_str());
    }

    READ_CHECK(ReadUI1(in, &readUI1));
    blending.rtBlend[0].colorFunc = readUI1;
    READ_CHECK(ReadUI1(in, &readUI1));
    blending.rtBlend[0].colorSrc = readUI1;
    READ_CHECK(ReadUI1(in, &readUI1));
    blending.rtBlend[0].colorDst = readUI1;
    READ_CHECK(ReadUI1(in, &readUI1));
    blending.rtBlend[0].alphaFunc = readUI1;
    READ_CHECK(ReadUI1(in, &readUI1));
    blending.rtBlend[0].alphaSrc = readUI1;
    READ_CHECK(ReadUI1(in, &readUI1));
    blending.rtBlend[0].alphaDst = readUI1;
    READ_CHECK(ReadUI1(in, &readUI1));
    blending.rtBlend[0].writeMask = readUI1;
    READ_CHECK(ReadUI1(in, &readUI1));
    blending.rtBlend[0].blendEnabled = readUI1;
    READ_CHECK(ReadUI1(in, &readUI1));
    blending.rtBlend[0].alphaToCoverage = readUI1;
    READ_CHECK(in->Seek(3, DAVA::File::SEEK_FROM_CURRENT));
    
#undef READ_CHECK

    return true;
*/
}

//------------------------------------------------------------------------------

static inline bool
WriteUI1(DAVA::File* f, uint8 x)
{
    return (f->Write(&x) == sizeof(x));
}

static inline bool
WriteUI4(DAVA::File* f, uint32 x)
{
    return (f->Write(&x) == sizeof(x));
}

static inline bool
WriteS0(DAVA::File* f, const char* str)
{
    char s0[128 * 1024];
    uint32 sz = L_ALIGNED_SIZE(strlen(str) + 1, sizeof(uint32));

    memset(s0, 0x00, sz);
    strcpy(s0, str);

    if (WriteUI4(f, sz))
    {
        return (f->Write(s0, sz) == sz);
    }
    return false;
}

bool ShaderSource::Save(DAVA::File* out) const
{
#define WRITE_CHECK(exp) if (!(exp)) { return false; }

    return false;
    /*
    WRITE_CHECK(WriteUI4(out, type));
    WRITE_CHECK(WriteS0(out, code.c_str()));

    WRITE_CHECK(vertexLayout.Save(out));

    WRITE_CHECK(WriteUI4(out, static_cast<uint32>(property.size())));
    for (unsigned p = 0; p != property.size(); ++p)
    {
        WRITE_CHECK(WriteS0(out, property[p].uid.c_str()));
        WRITE_CHECK(WriteS0(out, property[p].tag.c_str()));
        WRITE_CHECK(WriteUI4(out, property[p].type));
        WRITE_CHECK(WriteUI4(out, property[p].storage));
        WRITE_CHECK(WriteUI4(out, property[p].isBigArray));
        WRITE_CHECK(WriteUI4(out, property[p].arraySize));
        WRITE_CHECK(WriteUI4(out, property[p].bufferindex));
        WRITE_CHECK(WriteUI4(out, property[p].bufferReg));
        WRITE_CHECK(WriteUI4(out, property[p].bufferRegCount));
        WRITE_CHECK(out->Write(property[p].defaultValue, 16 * sizeof(float)) == 16 * sizeof(float));
    }

    WRITE_CHECK(WriteUI4(out, static_cast<uint32>(buf.size())));
    for (unsigned b = 0; b != buf.size(); ++b)
    {
        WRITE_CHECK(WriteUI4(out, buf[b].storage));
        WRITE_CHECK(WriteS0(out, buf[b].tag.c_str()));
        WRITE_CHECK(WriteUI4(out, buf[b].regCount));
    }

    WRITE_CHECK(WriteUI4(out, static_cast<uint32>(sampler.size())));
    for (unsigned s = 0; s != sampler.size(); ++s)
    {
        WRITE_CHECK(WriteUI4(out, sampler[s].type));
        WRITE_CHECK(WriteS0(out, sampler[s].uid.c_str()));
    }

    WRITE_CHECK(WriteUI1(out, blending.rtBlend[0].colorFunc));
    WRITE_CHECK(WriteUI1(out, blending.rtBlend[0].colorSrc));
    WRITE_CHECK(WriteUI1(out, blending.rtBlend[0].colorDst));
    WRITE_CHECK(WriteUI1(out, blending.rtBlend[0].alphaFunc));
    WRITE_CHECK(WriteUI1(out, blending.rtBlend[0].alphaSrc));
    WRITE_CHECK(WriteUI1(out, blending.rtBlend[0].alphaDst));
    WRITE_CHECK(WriteUI1(out, blending.rtBlend[0].writeMask));
    WRITE_CHECK(WriteUI1(out, blending.rtBlend[0].blendEnabled));
    WRITE_CHECK(WriteUI1(out, blending.rtBlend[0].alphaToCoverage));
    WRITE_CHECK(WriteUI1(out, 0));
    WRITE_CHECK(WriteUI1(out, 0));
    WRITE_CHECK(WriteUI1(out, 0));

#undef WRITE_CHECK

    return true;
*/
}

//------------------------------------------------------------------------------

bool
ShaderSource::GetSourceCode(Api targetApi, std::string* code) const
{
    DVASSERT(ast);

    bool success = false;
    static sl::Allocator alloc;
    static sl::HLSLGenerator hlsl_gen(&alloc);
    static sl::GLESGenerator gles_gen(&alloc);
    static sl::MSLGenerator mtl_gen(&alloc);
    const char* main = (type == PROG_VERTEX) ? "vp_main" : "fp_main";

    switch (targetApi)
    {
    case RHI_DX11:
    {
        sl::HLSLGenerator::Target target = (type == PROG_VERTEX) ? sl::HLSLGenerator::Target_VertexShader : sl::HLSLGenerator::Target_PixelShader;

        success = hlsl_gen.Generate(ast, sl::HLSLGenerator::MODE_DX11, target, main, code);
    }
    break;

    case RHI_DX9:
    {
        sl::HLSLGenerator::Target target = (type == PROG_VERTEX) ? sl::HLSLGenerator::Target_VertexShader : sl::HLSLGenerator::Target_PixelShader;

        success = hlsl_gen.Generate(ast, sl::HLSLGenerator::MODE_DX9, target, main, code);
    }
    break;

    case RHI_GLES2:
    {
        sl::GLESGenerator::Target target = (type == PROG_VERTEX) ? sl::GLESGenerator::Target_VertexShader : sl::GLESGenerator::Target_FragmentShader;

        success = gles_gen.Generate(ast, target, main, code);
    }
    break;

    case RHI_METAL:
    {
        sl::MSLGenerator::Target target = (type == PROG_VERTEX) ? sl::MSLGenerator::Target_VertexShader : sl::MSLGenerator::Target_PixelShader;

        success = mtl_gen.Generate(ast, target, main, code);
    }
    break;
    }

    return success;
}

//------------------------------------------------------------------------------

const ShaderPropList&
ShaderSource::Properties() const
{
    return property;
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
    return vertexLayout;
}

//------------------------------------------------------------------------------

uint32
ShaderSource::ConstBufferCount() const
{
    return static_cast<uint32>(buf.size());
}

//------------------------------------------------------------------------------

uint32
ShaderSource::ConstBufferSize(uint32 bufIndex) const
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
ShaderSource::ConstBufferStorage(uint32 bufIndex) const
{
    return buf[bufIndex].storage;
}

//------------------------------------------------------------------------------

BlendState
ShaderSource::Blending() const
{
    return blending;
}

//------------------------------------------------------------------------------

void ShaderSource::_Reset()
{
    vertexLayout.Clear();
    property.clear();
    sampler.clear();
    buf.clear();
    //    code.clear();
    codeLineCount = 0;

    for (unsigned i = 0; i != countof(blending.rtBlend); ++i)
    {
        blending.rtBlend[i].blendEnabled = false;
        blending.rtBlend[i].alphaToCoverage = false;
    }
}

//------------------------------------------------------------------------------
/*
void ShaderSource::_AppendLine(const char* line, size_t lineLen)
{
    code.append(line, lineLen);
    code.push_back('\n');
    return;
}
*/
//------------------------------------------------------------------------------

void ShaderSource::Dump() const
{
    /*
    Logger::Info("src-code:");

    char src[64 * 1024];
    char* src_line[1024];
    unsigned line_cnt = 0;

    if (strlen(code.c_str()) < sizeof(src))
    {
        strcpy(src, code.c_str());
        memset(src_line, 0, sizeof(src_line));

        src_line[line_cnt++] = src;
        for (char* s = src; *s; ++s)
        {
            if (*s == '\n' || *s == '\r')
            {
                while (*s && (*s == '\n' || *s == '\r'))
                {
                    *s = 0;
                    ++s;
                }

                if (!(*s))
                    break;

                src_line[line_cnt] = s;
                ++line_cnt;
            }
        }

        for (unsigned i = 0; i != line_cnt; ++i)
        {
            Logger::Info("%4u |  %s", 1 + i, src_line[i]);
        }
    }
    else
    {
        Logger::Info(code.c_str());
    }
*/
    Logger::Info("properties (%u) :", property.size());
    for (std::vector<ShaderProp>::const_iterator p = property.begin(), p_end = property.end(); p != p_end; ++p)
    {
        if (p->type == ShaderProp::TYPE_FLOAT4 || p->type == ShaderProp::TYPE_FLOAT4X4)
        {
            if (p->arraySize == 1)
            {
                Logger::Info("  %-16s    buf#%u  -  %u, %u x float4", p->uid.c_str(), p->bufferindex, p->bufferReg, p->bufferRegCount);
            }
            else
            {
                char name[128];

                Snprintf(name, sizeof(name) - 1, "%s[%u]", p->uid.c_str(), p->arraySize);
                Logger::Info("  %-16s    buf#%u  -  %u, %u x float4", name, p->bufferindex, p->bufferReg, p->bufferRegCount);
            }
        }
        else
        {
            const char* xyzw = "xyzw";

            switch (p->type)
            {
            case ShaderProp::TYPE_FLOAT1:
                Logger::Info("  %-16s    buf#%u  -  %u, %c", p->uid.c_str(), p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount]);
                break;

            case ShaderProp::TYPE_FLOAT2:
                Logger::Info("  %-16s    buf#%u  -  %u, %c%c", p->uid.c_str(), p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount + 0], xyzw[p->bufferRegCount + 1]);
                break;

            case ShaderProp::TYPE_FLOAT3:
                Logger::Info("  %-16s    buf#%u  -  %u, %c%c%c", p->uid.c_str(), p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount + 0], xyzw[p->bufferRegCount + 1], xyzw[p->bufferRegCount + 2]);
                break;

            default:
                break;
            }
        }
    }

    Logger::Info("buffers (%u) :", buf.size());
    for (unsigned i = 0; i != buf.size(); ++i)
    {
        Logger::Info("  buf#%u  reg.count = %u", i, buf[i].regCount);
    }

    if (type == PROG_VERTEX)
    {
        Logger::Info("vertex-layout:");
        vertexLayout.Dump();
    }

    Logger::Info("samplers (%u) :", sampler.size());
    for (unsigned s = 0; s != sampler.size(); ++s)
    {
        Logger::Info("  sampler#%u  \"%s\"", s, sampler[s].uid.c_str());
    }
}

//==============================================================================

Mutex shaderSourceEntryMutex;
std::vector<ShaderSourceCache::entry_t> ShaderSourceCache::Entry;
const uint32 ShaderSourceCache::FormatVersion = 4;

const ShaderSource*
ShaderSourceCache::Get(FastName uid, uint32 srcHash)
{
    LockGuard<Mutex> guard(shaderSourceEntryMutex);

    //Logger::Info("get-shader-src");
    //Logger::Info("  uid= \"%s\"",uid.c_str());
    const ShaderSource* src = nullptr;

    for (std::vector<entry_t>::const_iterator e = Entry.begin(), e_end = Entry.end(); e != e_end; ++e)
    {
        if (e->uid == uid && e->srcHash == srcHash)
        {
            src = e->src;
            break;
        }
    }
    //Logger::Info("  %s",(src)?"found":"not found");

    return src;
}

//------------------------------------------------------------------------------

void ShaderSourceCache::Update(FastName uid, uint32 srcHash, const ShaderSource& source)
{
    LockGuard<Mutex> guard(shaderSourceEntryMutex);

    bool doAdd = true;

    for (std::vector<entry_t>::iterator e = Entry.begin(), e_end = Entry.end(); e != e_end; ++e)
    {
        if (e->uid == uid)
        {
            *(e->src) = source;
            e->srcHash = srcHash;
            doAdd = false;
            break;
        }
    }

    if (doAdd)
    {
        entry_t e;

        e.uid = uid;
        e.srcHash = srcHash;
        e.src = new ShaderSource();
        *(e.src) = source;

        Entry.push_back(e);
        //Logger::Info("cache-updated  uid= \"%s\"",e.uid.c_str());
    }
}

//------------------------------------------------------------------------------

void ShaderSourceCache::Clear()
{
    LockGuard<Mutex> guard(shaderSourceEntryMutex);

    for (std::vector<entry_t>::const_iterator e = Entry.begin(), e_end = Entry.end(); e != e_end; ++e)
        delete e->src;
    Entry.clear();
}

//------------------------------------------------------------------------------

void ShaderSourceCache::Save(const char* fileName)
{
    using namespace DAVA;

    static const FilePath cacheTempFile("~doc:/shader_source_cache_temp.bin");

    File* file = File::Create(cacheTempFile, File::WRITE | File::CREATE);
    if (file)
    {
        Logger::Info("saving cached-shaders (%u): ", Entry.size());

        LockGuard<Mutex> guard(shaderSourceEntryMutex);
        bool success = true;

        SCOPE_EXIT
        {
            SafeRelease(file);

            if (success)
            {
                FileSystem::Instance()->MoveFile(cacheTempFile, fileName, true);
            }
            else
            {
                FileSystem::Instance()->DeleteFile(cacheTempFile);
            }
        };
        
#define WRITE_CHECK(exp) if (!exp) { success = false; return; }

        WRITE_CHECK(WriteUI4(file, FormatVersion));
        WRITE_CHECK(WriteUI4(file, static_cast<uint32>(Entry.size())));
        for (std::vector<entry_t>::const_iterator e = Entry.begin(), e_end = Entry.end(); e != e_end; ++e)
        {
            WRITE_CHECK(WriteS0(file, e->uid.c_str()));
            WRITE_CHECK(WriteUI4(file, e->srcHash));
            WRITE_CHECK(e->src->Save(file));
        }
        
#undef WRITE_CHECK
    }
}

//------------------------------------------------------------------------------

void ShaderSourceCache::Load(const char* fileName)
{
    using namespace DAVA;

    ScopedPtr<File> file(File::Create(fileName, File::READ | File::OPEN));

    if (file)
    {
        Clear();

        bool success = true;
        SCOPE_EXIT
        {
            if (!success)
            {
                Clear();
            }
        };
        
#define READ_CHECK(exp) if (!exp) { success = false; return; }

        uint32 readUI4 = 0;
        READ_CHECK(ReadUI4(file, &readUI4));

        if (readUI4 == FormatVersion)
        {
            LockGuard<Mutex> guard(shaderSourceEntryMutex);

            READ_CHECK(ReadUI4(file, &readUI4));
            Entry.resize(readUI4);
            Logger::Info("loading cached-shaders (%u): ", Entry.size());

            for (std::vector<entry_t>::iterator e = Entry.begin(), e_end = Entry.end(); e != e_end; ++e)
            {
                std::string str;
                READ_CHECK(ReadS0(file, &str));

                e->uid = FastName(str.c_str());
                READ_CHECK(ReadUI4(file, &e->srcHash));
                e->src = new ShaderSource();

                READ_CHECK(e->src->Load(file));
            }
        }
        else
        {
            Logger::Warning("ShaderSource-Cache version mismatch, ignoring cached shaders\n");
            success = false;
        }
        
#undef READ_CHECK
    }
}

//==============================================================================
} // namespace rhi
