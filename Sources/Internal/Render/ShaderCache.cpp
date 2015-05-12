/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#include "Render/ShaderCache.h"
#include "Render/RHI/rhi_ShaderCache.h"
#include "FileSystem/FileSystem.h"

/*namespace
{
const char* vProgText =
"VPROG_IN_BEGIN\n"
"    VPROG_IN_POSITION\n"
"    VPROG_IN_TEXCOORD\n"
"VPROG_IN_END\n"
"\n"
"VPROG_OUT_BEGIN\n"
"    VPROG_OUT_POSITION\n"
"    VPROG_OUT_TEXCOORD0(uv,2)\n"
"VPROG_OUT_END\n"
"\n"
"property float4x4 worldViewProjMatrix : unique,dynamic : ui_hidden=yes ;\n"
"\n"
"VPROG_BEGIN\n"
"\n"
"    float3 in_pos      = VP_IN_POSITION;\n"
"    float2 in_texcoord = VP_IN_TEXCOORD;\n"
"    VP_OUT(uv) =  in_texcoord;\n"
"    VP_OUT_POSITION   = mul( float4(in_pos.x,in_pos.y,in_pos.z,1.0), worldViewProjMatrix );\n"
"\n"
"VPROG_END\n";


const char* fProgText =
"FPROG_IN_BEGIN\n"
"FPROG_IN_TEXCOORD0(uv,2)\n"
"FPROG_IN_END\n"
"\n"
"FPROG_OUT_BEGIN\n"
"    FPROG_OUT_COLOR\n"
"FPROG_OUT_END\n"
"\n"
"\n"
"DECL_FP_SAMPLER2D(albedo)\n"
"\n"
"FPROG_BEGIN\n"
"\n"
"float4  texColor = FP_TEXTURE2D( albedo, FP_IN(uv) );\n"
"\n"
"    texColor.a = 1.0;\n"
"    FP_OUT_COLOR = texColor;\n"
"FPROG_END\n";
}*/
namespace DAVA
{
namespace ShaderDescriptorCache
{
struct ShaderSourceCode
{
    char8* vProgText;
    char8* fProgText;
    ShaderSourceCode() : vProgText(nullptr), fProgText(nullptr){};
};
    
namespace
{
    Map<Vector<int32>, ShaderDescriptor *> shaderDescriptors;
    Map<FastName, ShaderSourceCode> shaderSourceCodes;
    bool initialized = false;
}
        

void Initialize()
{
    DVASSERT(!initialized);
    initialized = true;
}

void Uninitialize()
{
    DVASSERT(initialized);
    Clear();
    initialized = false;
}

void Clear()
{
    DVASSERT(initialized);
    for (auto &it : shaderSourceCodes)
    {        
        SafeDelete(it.second.vProgText);
        SafeDelete(it.second.fProgText);
    }
    shaderSourceCodes.clear();

}

void BuildFlagsKey(const FastName& name,const HashMap<FastName, int32>& defines, Vector<int32>& key)
{    
    key.clear();
    key.reserve(defines.size() * 2 + 1);
    for (auto& define : defines)
    {
        key.push_back(define.first.Index());
        key.push_back(define.second);
    }
    key.push_back(name.Index());
}

ShaderSourceCode LoadFromSource(const String& source)
{
    ShaderSourceCode sourceCode;
    FilePath vertexShaderPath = FilePath(source + "-vp.cg");
    FilePath fragmentShaderPath = FilePath(source + "-fp.cg");
    
    //later move it into FileSystem

    //vertex
    File * fp = File::Create(vertexShaderPath, File::OPEN | File::READ);
    if (fp)
    {
        uint32 fileSize = fp->GetSize();
        sourceCode.vProgText = new char8[fileSize + 1];
        sourceCode.vProgText[fileSize] = 0;
        uint32 dataRead = fp->Read((uint8*)sourceCode.vProgText, fileSize);
        if (dataRead != fileSize)
        {
            Logger::Error("Failed to open vertex shader source file: %s", vertexShaderPath.GetAbsolutePathname().c_str());
        }
    }
    else
    {
        Logger::Error("Failed to open vertex shader source file: %s", vertexShaderPath.GetAbsolutePathname().c_str());
    }
    SafeRelease(fp);

    //fragment
    fp = File::Create(fragmentShaderPath, File::OPEN | File::READ);
    if (fp)
    {
        uint32 fileSize = fp->GetSize();
        sourceCode.fProgText = new char8[fileSize + 1];
        sourceCode.fProgText[fileSize] = 0;
        uint32 dataRead = fp->Read((uint8*)sourceCode.fProgText, fileSize);
        if (dataRead != fileSize)
        {
            Logger::Error("Failed to open fragment shader source file: %s", fragmentShaderPath.GetAbsolutePathname().c_str());
        }
    }
    else
    {
        Logger::Error("Failed to open fragment shader source file: %s", fragmentShaderPath.GetAbsolutePathname().c_str());
    }
    SafeRelease(fp);

    return sourceCode;
}

ShaderDescriptor* GetShaderDescriptor(const FastName& name, const HashMap<FastName, int32>& defines)
{    
    DVASSERT(initialized);
 
    Vector<int32> key;
    BuildFlagsKey(name, defines, key);    

    auto descriptorIt = shaderDescriptors.find(key);
    if (descriptorIt != shaderDescriptors.end())
        return descriptorIt->second;

    //not found - create new shader
    Vector<String> progDefines;
    progDefines.reserve(defines.size() * 2);
    String resName;
    for (auto& it : defines)
    {
        progDefines.push_back(String(it.first.c_str()));
        progDefines.push_back(DAVA::Format("%d", it.second));        
        resName += Format("#define %s %d\n", it.first.c_str(), it.second);
    }


    ShaderSourceCode sourceCode;
    auto sourceIt = shaderSourceCodes.find(name);
    if (sourceIt != shaderSourceCodes.end()) //source found
    {
        sourceCode = sourceIt->second;
    }
    else
    {
        sourceCode = LoadFromSource(name.c_str());
        //sourceCode = LoadFromSource("~res:/Materials/Shaders/Default/materials");
        shaderSourceCodes[name] = sourceCode;
    }

    rhi::ShaderSource vSource, fSource;
    vSource.Construct(rhi::PROG_VERTEX, sourceCode.vProgText, progDefines);
    fSource.Construct(rhi::PROG_FRAGMENT, sourceCode.fProgText, progDefines);
    /*vSource.Dump();
    fSource.Dump();    */
    
    FastName vProgUid, fProgUid;    
    vProgUid = FastName(String("vSource: ") + resName);
    fProgUid = FastName(String("fSource: ") + resName);

    rhi::ShaderCache::UpdateProg(rhi::HostApi(), rhi::PROG_VERTEX, vProgUid, vSource.SourceCode());
    rhi::ShaderCache::UpdateProg(rhi::HostApi(), rhi::PROG_FRAGMENT, fProgUid, fSource.SourceCode());

    rhi::PipelineState::Descriptor  psDesc;
    psDesc.vprogUid = vProgUid;
    psDesc.fprogUid = fProgUid;
    psDesc.vertexLayout = vSource.ShaderVertexLayout();
    psDesc.blending = fSource.Blending();
    rhi::HPipelineState piplineState = rhi::AcquireRenderPipelineState(psDesc);
    
    ShaderDescriptor *res = new ShaderDescriptor(&vSource, &fSource, piplineState);
    shaderDescriptors[key] = res;

    return res;
}

}
};

