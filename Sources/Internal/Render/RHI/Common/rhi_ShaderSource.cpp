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

    
    #include "../rhi_ShaderSource.h"
    
    #include "FileSystem/Logger.h"
using DAVA::Logger;
    #include "FileSystem/DynamicMemoryFile.h"
using DAVA::DynamicMemoryFile;
    #include "Utils/Utils.h"
#include "Debug/Profiler.h"

    #include "PreProcess.h"

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

    // parse properties/samplers

    DAVA::ScopedPtr<DynamicMemoryFile> in(DynamicMemoryFile::Create((const uint8*)src.c_str(), (uint32)(src.length() + 1), DAVA::File::READ));

    if (in)
    {
        #if RHI__USE_STD_REGEX
        std::regex prop_re(".*property\\s*(float|float2|float3|float4|float4x4)\\s*([a-zA-Z_]+[a-zA-Z_0-9]*)\\s*\\:\\s*(.*)\\s+\\:(.*);.*");
        std::regex proparr_re(".*property\\s*(float4|float4x4)\\s*([a-zA-Z_]+[a-zA-Z_0-9]*)\\s*\\[(\\s*[0-9]+)\\s*\\]\\s*\\:\\s*(.*)\\s+\\:(.*);.*");
        std::regex fsampler2d_re(".*DECL_FP_SAMPLER2D\\s*\\(\\s*(.*)\\s*\\).*");
        std::regex vsampler2d_re(".*DECL_VP_SAMPLER2D\\s*\\(\\s*(.*)\\s*\\).*");
        std::regex samplercube_re(".*DECL_FP_SAMPLERCUBE\\s*\\(\\s*(.*)\\s*\\).*");
        std::regex ftexture2d_re(".*FP_TEXTURE2D\\s*\\(\\s*([a-zA-Z0-9_]+)\\s*\\,.*");
        std::regex vtexture2d_re(".*VP_TEXTURE2D\\s*\\(\\s*([a-zA-Z0-9_]+)\\s*\\,.*");
        std::regex texturecube_re(".*FP_TEXTURECUBE\\s*\\(\\s*([a-zA-Z0-9_]+)\\s*\\,.*");
        std::regex blend_re(".*BLEND_MODE\\s*\\(\\s*(.*)\\s*\\).*");
        std::regex blending2_re(".*blending\\s*\\:\\s*src=(zero|one|src_alpha|inv_src_alpha|src_color|dst_color)\\s+dst=(zero|one|src_alpha|inv_src_alpha|src_color|dst_color).*");
        std::regex colormask_re(".*color_mask\\s*\\:\\s*(all|none|rgb|a).*");
        std::regex comment_re("^\\s*//.*");
        #else
        RegExp prop_re;
        RegExp proparr_re;
        RegExp fsampler2d_re;
        RegExp vsampler2d_re;
        RegExp samplercube_re;
        RegExp ftexture2d_re;
        RegExp vtexture2d_re;
        RegExp texturecube_re;
        RegExp blend_re;
        RegExp blending2_re;
        RegExp colormask_re;
        RegExp comment_re;

#if !(RHI__OPTIMIZED_REGEX)
        prop_re.compile(".*property\\s*(float|float2|float3|float4|float4x4)\\s*([a-zA-Z_]+[a-zA-Z_0-9]*)\\s*\\:\\s*(.*)\\s+\\:(.*);.*");
        proparr_re.compile(".*property\\s*(float4|float4x4)\\s*([a-zA-Z_]+[a-zA-Z_0-9]*)\\s*\\[(\\s*[0-9]+)\\s*\\]\\s*\\:\\s*(.*)\\s+\\:(.*);.*");
        fsampler2d_re.compile(".*DECL_FP_SAMPLER2D\\s*\\(\\s*(.*)\\s*\\).*");
        vsampler2d_re.compile(".*DECL_VP_SAMPLER2D\\s*\\(\\s*(.*)\\s*\\).*");
        samplercube_re.compile(".*DECL_FP_SAMPLERCUBE\\s*\\(\\s*(.*)\\s*\\).*");
        ftexture2d_re.compile(".*FP_TEXTURE2D\\s*\\(\\s*([a-zA-Z0-9_]+)\\s*\\,.*");
        vtexture2d_re.compile(".*VP_TEXTURE2D\\s*\\(\\s*([a-zA-Z0-9_]+)\\s*\\,.*");
        texturecube_re.compile(".*FP_TEXTURECUBE\\s*\\(\\s*([a-zA-Z0-9_]+)\\s*\\,.*");
        blend_re.compile(".*BLEND_MODE\\s*\\(\\s*(.*)\\s*\\).*");
        blending2_re.compile(".*blending\\s*\\:\\s*src=(zero|one|src_alpha|inv_src_alpha|src_color|dst_color)\\s+dst=(zero|one|src_alpha|inv_src_alpha|src_color|dst_color).*");
        colormask_re.compile(".*color_mask\\s*\\:\\s*(all|none|rgb|a).*");
        comment_re.compile("^\\s*//.*");
#else
        prop_re.compile("property\\s*(\\w+)\\s*(\\w+)\\s*\\:\\s*([\\w,]*)\\s+\\:\\s*([\\w\\s=,\\.]*);");
        proparr_re.compile("property\\s*(float4|float4x4)\\s*(\\w+)\\s*\\[\\s*([\\d]+)\\s*\\]\\s*\\:\\s*([\\w,]*)\\s+\\:\\s*([\\w,]*)");
        fsampler2d_re.compile("DECL_FP_SAMPLER2D\\s*\\(\\s*(\\w+)\\s*\\)");
        vsampler2d_re.compile("DECL_VP_SAMPLER2D\\s*\\(\\s*(\\w+)\\s*\\)");
        samplercube_re.compile("DECL_FP_SAMPLERCUBE\\s*\\(\\s*(\\w+)\\s*\\)");
        ftexture2d_re.compile("FP_TEXTURE2D\\s*\\(\\s*(\\w+)\\s*\\,");
        vtexture2d_re.compile("VP_TEXTURE2D\\s*\\(\\s*(\\w+)\\s*\\,");
        texturecube_re.compile("FP_TEXTURECUBE\\s*\\(\\s*(\\w+)\\s*\\,");
        blend_re.compile("BLEND_MODE\\s*\\(\\s*(\\w+)\\s*\\)");
        blending2_re.compile("blending\\s*\\:\\s*src=(\\w+)\\s+dst=(\\w+)");
        colormask_re.compile("color_mask\\s*\\:\\s*(\\w+)");
        comment_re.compile("^\\s*//.*");
#endif
        #endif

        _Reset();

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
                }
                else if (stricmp(type.c_str(), "float2") == 0)
                {
                    p.type = ShaderProp::TYPE_FLOAT2;
                }
                else if (stricmp(type.c_str(), "float3") == 0)
                {
                    p.type = ShaderProp::TYPE_FLOAT3;
                }
                else if (stricmp(type.c_str(), "float4") == 0)
                {
                    p.type = ShaderProp::TYPE_FLOAT4;
                }
                else if (stricmp(type.c_str(), "float4x4") == 0)
                {
                    p.type = ShaderProp::TYPE_FLOAT4X4;
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
                int sl = sprintf(line + mbegin, "%u", (unsigned)(sampler.size()));
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
                int sl = sprintf(line + mbegin, "%u", (unsigned)(sampler.size()));
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
                int sl = sprintf(line + mbegin, "%u", (unsigned)(sampler.size()));
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

        type = progType;

        success = true;
    }

    if (success)
    {
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
                    var_len += Snprintf(var_def + var_len, sizeof(var_def) - var_len, "    float %s = float4(%cP_Buffer%u[%u]).%c;\n", p->uid.c_str(), pt, p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount]);
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
                        "    #define %s  %cP_Buffer%u[%u]\n",
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
                    "    float4x4 %s = float4x4( %cP_Buffer%u[%u], %cP_Buffer%u[%u], %cP_Buffer%u[%u], %cP_Buffer%u[%u] );\n",
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
    }

    return success;
}

//------------------------------------------------------------------------------

static inline uint8
ReadUI1(DAVA::File* f)
{
    uint8 x;

    f->Read(&x);
    return x;
}

static inline uint32
ReadUI4(DAVA::File* f)
{
    uint32 x;

    f->Read(&x);
    return x;
}

static inline void
ReadS0(DAVA::File* f, std::string* str)
{
    char s0[128 * 1024];
    uint32 sz = ReadUI4(f);

    f->Read(s0, sz);
    *str = s0;
}

bool ShaderSource::Load(DAVA::File* in)
{
    bool success = false;
    std::string s0;

    _Reset();

    type = ProgType(ReadUI4(in));
    ReadS0(in, &code);

    vdecl.Load(in);

    prop.resize(ReadUI4(in));
    for (unsigned p = 0; p != prop.size(); ++p)
    {
        ReadS0(in, &s0);
        prop[p].uid = FastName(s0.c_str());

        ReadS0(in, &s0);
        prop[p].tag = FastName(s0.c_str());

        prop[p].type = ShaderProp::Type(ReadUI4(in));
        prop[p].storage = ShaderProp::Storage(ReadUI4(in));
        prop[p].isBigArray = ReadUI4(in);
        prop[p].arraySize = ReadUI4(in);
        prop[p].bufferindex = ReadUI4(in);
        prop[p].bufferReg = ReadUI4(in);
        prop[p].bufferRegCount = ReadUI4(in);

        in->Read(prop[p].defaultValue, 16 * sizeof(float));
    }

    buf.resize(ReadUI4(in));
    for (unsigned b = 0; b != buf.size(); ++b)
    {
        buf[b].storage = ShaderProp::Storage(ReadUI4(in));

        ReadS0(in, &s0);
        buf[b].tag = FastName(s0.c_str());

        buf[b].regCount = ReadUI4(in);
    }

    sampler.resize(ReadUI4(in));
    for (unsigned s = 0; s != sampler.size(); ++s)
    {
        sampler[s].type = TextureType(ReadUI4(in));

        ReadS0(in, &s0);
        sampler[s].uid = FastName(s0.c_str());
    }

    blending.rtBlend[0].colorFunc = ReadUI1(in);
    blending.rtBlend[0].colorSrc = ReadUI1(in);
    blending.rtBlend[0].colorDst = ReadUI1(in);
    blending.rtBlend[0].alphaFunc = ReadUI1(in);
    blending.rtBlend[0].alphaSrc = ReadUI1(in);
    blending.rtBlend[0].alphaDst = ReadUI1(in);
    blending.rtBlend[0].writeMask = ReadUI1(in);
    blending.rtBlend[0].blendEnabled = ReadUI1(in);
    blending.rtBlend[0].alphaToCoverage = ReadUI1(in);
    in->Seek(3, DAVA::File::SEEK_FROM_CURRENT);

    return success;
}

//------------------------------------------------------------------------------

static inline void
WriteUI1(DAVA::File* f, uint8 x)
{
    f->Write(&x);
}

static inline void
WriteUI4(DAVA::File* f, uint32 x)
{
    f->Write(&x);
}

static inline void
WriteS0(DAVA::File* f, const char* str)
{
    char s0[128 * 1024];
    uint32 sz = L_ALIGNED_SIZE(strlen(str) + 1, sizeof(uint32));

    memset(s0, 0x00, sz);
    strcpy(s0, str);

    WriteUI4(f, sz);
    f->Write(s0, sz);
}

bool ShaderSource::Save(DAVA::File* out) const
{
    bool success = false;

    WriteUI4(out, type);
    WriteS0(out, code.c_str());

    vdecl.Save(out);

    WriteUI4(out, static_cast<uint32>(prop.size()));
    for (unsigned p = 0; p != prop.size(); ++p)
    {
        WriteS0(out, prop[p].uid.c_str());
        WriteS0(out, prop[p].tag.c_str());
        WriteUI4(out, prop[p].type);
        WriteUI4(out, prop[p].storage);
        WriteUI4(out, prop[p].isBigArray);
        WriteUI4(out, prop[p].arraySize);
        WriteUI4(out, prop[p].bufferindex);
        WriteUI4(out, prop[p].bufferReg);
        WriteUI4(out, prop[p].bufferRegCount);
        out->Write(prop[p].defaultValue, 16 * sizeof(float));
    }

    WriteUI4(out, static_cast<uint32>(buf.size()));
    for (unsigned b = 0; b != buf.size(); ++b)
    {
        WriteUI4(out, buf[b].storage);
        WriteS0(out, buf[b].tag.c_str());
        WriteUI4(out, buf[b].regCount);
    }

    WriteUI4(out, static_cast<uint32>(sampler.size()));
    for (unsigned s = 0; s != sampler.size(); ++s)
    {
        WriteUI4(out, sampler[s].type);
        WriteS0(out, sampler[s].uid.c_str());
    }

    WriteUI1(out, blending.rtBlend[0].colorFunc);
    WriteUI1(out, blending.rtBlend[0].colorSrc);
    WriteUI1(out, blending.rtBlend[0].colorDst);
    WriteUI1(out, blending.rtBlend[0].alphaFunc);
    WriteUI1(out, blending.rtBlend[0].alphaSrc);
    WriteUI1(out, blending.rtBlend[0].alphaDst);
    WriteUI1(out, blending.rtBlend[0].writeMask);
    WriteUI1(out, blending.rtBlend[0].blendEnabled);
    WriteUI1(out, blending.rtBlend[0].alphaToCoverage);
    WriteUI1(out, 0);
    WriteUI1(out, 0);
    WriteUI1(out, 0);

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
    vdecl.Clear();
    prop.clear();
    sampler.clear();
    buf.clear();
    code.clear();
    codeLineCount = 0;

    for (unsigned i = 0; i != countof(blending.rtBlend); ++i)
    {
        blending.rtBlend[i].blendEnabled = false;
        blending.rtBlend[i].alphaToCoverage = false;
    }
}

//------------------------------------------------------------------------------

void ShaderSource::_AppendLine(const char* line, size_t lineLen)
{
    code.append(line, lineLen);
    code.push_back('\n');
    return;

    char text[4 * 1024];
    int len = Snprintf(text, sizeof(text) - 1, "/*%04u*/ ", codeLineCount + 1);

    strncpy(text + len, line, lineLen);
    text[len + lineLen] = '\0';

    code.append(text, strlen(text));
    code.push_back('\n');

    ++codeLineCount;
}

//------------------------------------------------------------------------------

void ShaderSource::Dump() const
{
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

    Logger::Info("properties (%u) :", prop.size());
    for (std::vector<ShaderProp>::const_iterator p = prop.begin(), p_end = prop.end(); p != p_end; ++p)
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
        vdecl.Dump();
    }

    Logger::Info("samplers (%u) :", sampler.size());
    for (unsigned s = 0; s != sampler.size(); ++s)
    {
        Logger::Info("  sampler#%u  \"%s\"", s, sampler[s].uid.c_str());
    }
}

//==============================================================================

std::vector<ShaderSourceCache::entry_t> ShaderSourceCache::Entry;
const uint32 ShaderSourceCache::FormatVersion = 2;

const ShaderSource*
ShaderSourceCache::Get(FastName uid, uint32 srcHash)
{
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
    for (std::vector<entry_t>::const_iterator e = Entry.begin(), e_end = Entry.end(); e != e_end; ++e)
        delete e->src;
    Entry.clear();
}

//------------------------------------------------------------------------------

void ShaderSourceCache::Save(const char* fileName)
{
    using namespace DAVA;

    File* file = File::Create(fileName, File::WRITE | File::CREATE);

    if (file)
    {
        WriteUI4(file, FormatVersion);

        WriteUI4(file, static_cast<uint32>(Entry.size()));
        Logger::Info("saving cached-shaders (%u) :", Entry.size());
        for (std::vector<entry_t>::const_iterator e = Entry.begin(), e_end = Entry.end(); e != e_end; ++e)
        {
            Logger::Info("  uid= \"%s\"", e->uid.c_str());
            WriteS0(file, e->uid.c_str());
            WriteUI4(file, e->srcHash);
            e->src->Save(file);
        }

        file->Release();
    }
}

//------------------------------------------------------------------------------

void ShaderSourceCache::Load(const char* fileName)
{
    using namespace DAVA;

    File* file = File::Create(fileName, File::READ | File::OPEN);

    if (file)
    {
        Clear();
        uint32 version = ReadUI4(file);

        if (version == FormatVersion)
        {
            Entry.resize(ReadUI4(file));
            Logger::Info("loading cached-shaders (%u) :", Entry.size());
            for (std::vector<entry_t>::iterator e = Entry.begin(), e_end = Entry.end(); e != e_end; ++e)
            {
                std::string str;
                ReadS0(file, &str);

                e->uid = FastName(str.c_str());
                e->srcHash = ReadUI4(file);
                Logger::Info("  uid= \"%s\"", e->uid.c_str());
                e->src = new ShaderSource();

                e->src->Load(file);
            }
            Logger::Info("loaded ShaderSource cache (%u shaders)", Entry.size());
        }
        else
        {
            Logger::Warning("ShaderSource-Cache version mismatch, ignoring cached shaders\n");
        }

        file->Release();
    }
}

//==============================================================================
} // namespace rhi
