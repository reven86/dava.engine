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

void YamlPackageSerializer::BeginMap(const DAVA::String &name)
{
    YamlNode *node = YamlNode::CreateMapNode(false, YamlNode::MR_BLOCK_REPRESENTATION, YamlNode::SR_PLAIN_REPRESENTATION);
    nodesStack.back()->Add(name, node);
    nodesStack.push_back(node);
}

void YamlPackageSerializer::EndMap()
{
    nodesStack.pop_back();
}

void YamlPackageSerializer::BeginArray(const DAVA::String &name)
{
    YamlNode *node = YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION);
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
