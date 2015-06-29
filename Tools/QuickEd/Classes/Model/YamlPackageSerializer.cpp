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


#include "YamlPackageSerializer.h"

#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlEmitter.h"
#include "FileSystem/DynamicMemoryFile.h"

using namespace DAVA;


YamlPackageSerializer::YamlPackageSerializer()
{
    nodesStack.push_back(YamlNode::CreateMapNode(false, YamlNode::MR_BLOCK_REPRESENTATION, YamlNode::SR_PLAIN_REPRESENTATION));
}

YamlPackageSerializer::~YamlPackageSerializer()
{
    DVASSERT(nodesStack.size() == 1);
    
    for (auto it : nodesStack)
        it->Release();
    nodesStack.clear();
}

void YamlPackageSerializer::PutValue(const DAVA::String &name, const DAVA::VariantType &value)
{
    nodesStack.back()->Add(name, value);
}

void YamlPackageSerializer::PutValue(const DAVA::String &name, const DAVA::String &value)
{
    nodesStack.back()->Add(name, value);
}

void YamlPackageSerializer::PutValue(const DAVA::String &name, const DAVA::Vector<DAVA::String> &value)
{
    YamlNode *node = YamlNode::CreateArrayNode(YamlNode::AR_FLOW_REPRESENTATION);
    for (const auto &str : value)
        node->Add(str);
    
    nodesStack.back()->Add(name, node);
}

void YamlPackageSerializer::PutValue(const DAVA::VariantType &value)
{
    nodesStack.back()->Add(value);
}

void YamlPackageSerializer::PutValue(const DAVA::String &value)
{
    nodesStack.back()->Add(value);
}

void YamlPackageSerializer::BeginMap()
{
    YamlNode *node = YamlNode::CreateMapNode(false, YamlNode::MR_BLOCK_REPRESENTATION, YamlNode::SR_PLAIN_REPRESENTATION);
    nodesStack.back()->Add(node);
    nodesStack.push_back(node);
}

void YamlPackageSerializer::BeginMap(const DAVA::String &name, bool quotes)
{
    YamlNode *node = YamlNode::CreateMapNode(false, YamlNode::MR_BLOCK_REPRESENTATION,
                                             quotes ? YamlNode::SR_DOUBLE_QUOTED_REPRESENTATION : YamlNode::SR_PLAIN_REPRESENTATION);
    nodesStack.back()->Add(name, node);
    nodesStack.push_back(node);
}

void YamlPackageSerializer::EndMap()
{
    nodesStack.pop_back();
}

void YamlPackageSerializer::BeginArray(const DAVA::String &name, bool flow)
{
    YamlNode *node = YamlNode::CreateArrayNode(flow ? YamlNode::AR_FLOW_REPRESENTATION : YamlNode::AR_BLOCK_REPRESENTATION);
    nodesStack.back()->Add(name, node);
    nodesStack.push_back(node);
}

void YamlPackageSerializer::BeginArray()
{
    YamlNode *node = YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION);
    nodesStack.back()->Add(node);
    nodesStack.push_back(node);
}

void YamlPackageSerializer::EndArray()
{
    nodesStack.pop_back();
}

YamlNode *YamlPackageSerializer::GetYamlNode() const
{
    DVASSERT(nodesStack.size() == 1);
    return nodesStack.back();
}

void YamlPackageSerializer::WriteToFile(const FilePath &path)
{
    YamlEmitter::SaveToYamlFile(path, GetYamlNode());
}

String YamlPackageSerializer::WriteToString() const
{
    DynamicMemoryFile *file = DynamicMemoryFile::Create(File::WRITE);
    YamlEmitter::SaveToYamlFile(GetYamlNode(), file);
    String str((const char*)file->GetData(), file->GetSize());
    SafeRelease(file);
    return str;
}
