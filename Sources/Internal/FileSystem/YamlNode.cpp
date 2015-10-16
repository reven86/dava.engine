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


#include "YamlNode.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/KeyedArchive.h"
#include "Utils/Utils.h"
#include "Utils/UTF8Utils.h"

namespace DAVA
{

static const String EMPTY_STRING = "";
static const Vector<YamlNode*> EMPTY_VECTOR;
static const MultiMap<String, YamlNode*> EMPTY_MAP = MultiMap<String, YamlNode*>();

YamlNode * YamlNode::CreateStringNode()
{
    YamlNode * node = new YamlNode(TYPE_STRING);
    return node;
}

YamlNode * YamlNode::CreateArrayNode(eArrayRepresentation representation/* = AR_FLOW_REPRESENTATION*/)
{
    YamlNode * node = new YamlNode(TYPE_ARRAY);
    node->objectArray->style = representation;
    return node;
}

YamlNode * YamlNode::CreateMapNode(bool orderedSave/* = true*/, eMapRepresentation valRepresentation /*= MR_BLOCK_REPRESENTATION*/, eStringRepresentation keyRepresentation /*= SR_PLAIN_REPRESENTATION*/)
{
    YamlNode * node = new YamlNode(TYPE_MAP);
    node->objectMap->style = valRepresentation;
    node->objectMap->keyStyle = keyRepresentation;
    node->objectMap->orderedSave = orderedSave;
    return node;
}

YamlNode::YamlNode(eType _type)
    : type(_type)
{
    switch(GetType())
    {
    case TYPE_STRING:
        objectString = new ObjectString();
        objectString->style = SR_DOUBLE_QUOTED_REPRESENTATION;
        break;
    case TYPE_ARRAY:
        objectArray = new ObjectArray();
        objectArray->style = AR_FLOW_REPRESENTATION;
        break;
    case TYPE_MAP:
        objectMap = new ObjectMap();
        objectMap->style = MR_BLOCK_REPRESENTATION;
        objectMap->keyStyle = SR_PLAIN_REPRESENTATION;
        objectMap->orderedSave = true;
        break;
    }
}

YamlNode::~YamlNode()
{
    switch(GetType())
    {
    case TYPE_STRING:
        {
            SafeDelete(objectString);
        }
        break;
    case TYPE_ARRAY:
        {
            int32 size = (int32)objectArray->array.size();
            for (int32 k = 0; k < size; ++k)
            {
                SafeRelease(objectArray->array[k]);
            }
            objectArray->array.clear();
            SafeDelete(objectArray);
        }
        break;
    case TYPE_MAP:
        {
            MultiMap<String, YamlNode*>::iterator iter= objectMap->ordered.begin(),
                                                  end = objectMap->ordered.end();
            for (; iter != end; ++iter)
            {
                SafeRelease(iter->second);
            }
            objectMap->ordered.clear();
            objectMap->unordered.clear();
            SafeDelete(objectMap);
        }
        break;
    }
}

uint32 YamlNode::GetCount() const
{
    switch (GetType())
    {
        case TYPE_MAP: return static_cast<uint32>(objectMap->unordered.size());
        case TYPE_ARRAY: return static_cast<uint32>(objectArray->array.size());
        default: break;
    }
    return 0;//string nodes does not contain content
}

int32  YamlNode::AsInt() const
{
    return AsInt32();
}

int32 YamlNode::AsInt32() const
{
    DVASSERT(GetType() == TYPE_STRING);
    int32 ret = 0;
    if (GetType() == TYPE_STRING)
    {
        sscanf(objectString->nwStringValue.c_str(), "%d", &ret);
    }
    return ret;
}

uint32 YamlNode::AsUInt32() const
{
    DVASSERT(GetType() == TYPE_STRING);
    uint32 ret = 0;
    if (GetType() == TYPE_STRING)
    {
        sscanf(objectString->nwStringValue.c_str(), "%u", &ret);
    }
    return ret;
}

int64 YamlNode::AsInt64() const
{
    DVASSERT(GetType() == TYPE_STRING);
    int64 ret = 0;
    if (GetType() == TYPE_STRING)
    {
        sscanf(objectString->nwStringValue.c_str(), "%lld", &ret);
    }
    return ret;
}

uint64 YamlNode::AsUInt64() const
{
    DVASSERT(GetType() == TYPE_STRING);
    uint64 ret = 0;
    if (GetType() == TYPE_STRING)
    {
        sscanf(objectString->nwStringValue.c_str(), "%llu", &ret);
    }
    return ret;
}

float32	YamlNode::AsFloat() const
{
    DVASSERT(GetType() == TYPE_STRING);
    float32 ret = 0.0f;
    if (GetType() == TYPE_STRING)
    {
        sscanf(objectString->nwStringValue.c_str(), "%f", &ret);
    }
    return ret;
}

const String & YamlNode::AsString() const
{
    DVASSERT(GetType() == TYPE_STRING);
    if (GetType() == TYPE_STRING)
        return objectString->nwStringValue;

    return EMPTY_STRING;
}

FastName YamlNode::AsFastName() const
{
    return FastName(AsString());
}

bool YamlNode::AsBool() const
{
    return ("true" == AsString() || "yes" == AsString());
}

WideString YamlNode::AsWString() const
{
    DVASSERT(GetType() == TYPE_STRING);
    if (GetType() == TYPE_STRING)
    {
        return UTF8Utils::EncodeToWideString(objectString->nwStringValue);
    }

    return L"";
}

Vector2	YamlNode::AsPoint() const
{
    Vector2 result;
    if (GetType() == TYPE_ARRAY)
    {
        const YamlNode * x = Get(0);
        if (x)result.x = x->AsFloat();
        const YamlNode * y = Get(1);
        if (y)result.y = y->AsFloat();
    }
    return result;
}

Vector3 YamlNode::AsVector3() const
{
    Vector3 result(0, 0, 0);
    if (GetType() == TYPE_ARRAY)
    {
        const YamlNode * x = Get(0);
        if (x)
            result.x = x->AsFloat();

        const YamlNode * y = Get(1);
        if (y)
            result.y = y->AsFloat();

        const YamlNode * z = Get(2);
        if (z)
            result.z = z->AsFloat();
    }
    return result;
}

Vector4 YamlNode::AsVector4() const
{
    Vector4 result(0, 0, 0, 0);
    if (GetType() == TYPE_ARRAY)
    {
        const YamlNode * x = Get(0);
        if (x)
            result.x = x->AsFloat();

        const YamlNode * y = Get(1);
        if (y)
            result.y = y->AsFloat();

        const YamlNode * z = Get(2);
        if (z)
            result.z = z->AsFloat();

        const YamlNode * w = Get(3);
        if (w)
            result.w = w->AsFloat();
    }
    return result;
}

Color YamlNode::AsColor() const
{
    Color result = Color::White;
    if (GetType() == TYPE_ARRAY)
    {
        const YamlNode * r = Get(0);
        if (r)
            result.r = r->AsFloat();

        const YamlNode * g = Get(1);
        if (g)
            result.g = g->AsFloat();

        const YamlNode * b = Get(2);
        if (b)
            result.b = b->AsFloat();

        const YamlNode * a = Get(3);
        if (a)
            result.a = a->AsFloat();
    }
    return result;
}

Vector2 YamlNode::AsVector2() const
{
    return AsPoint();
}

Rect	YamlNode::AsRect() const
{
    Rect result;
    if (GetType() == TYPE_ARRAY)
    {
        const YamlNode * x = Get(0);
        if (x)result.x = x->AsFloat();
        const YamlNode * y = Get(1);
        if (y)result.y = y->AsFloat();
        const YamlNode * dx = Get(2);
        if (dx)result.dx = dx->AsFloat();
        const YamlNode * dy = Get(3);
        if (dy)result.dy = dy->AsFloat();
    }
    return result;
}

VariantType YamlNode::AsVariantType() const
{
    VariantType retValue;

    const MultiMap<String, YamlNode*> & mapFromNode = AsMap();

    for(MultiMap<String, YamlNode*>::const_iterator it = mapFromNode.begin(); it != mapFromNode.end(); ++it)
    {
        const String &innerTypeName = it->first;

        if(innerTypeName == DAVA::VariantType::TYPENAME_BOOLEAN)
        {
            retValue.SetBool(it->second->AsBool());
        }
        if(innerTypeName == DAVA::VariantType::TYPENAME_INT32)
        {
            retValue.SetInt32(it->second->AsInt32());
        }
        if(innerTypeName == DAVA::VariantType::TYPENAME_UINT32)
        {
            retValue.SetUInt32(it->second->AsUInt32());
        }
        if(innerTypeName == DAVA::VariantType::TYPENAME_INT64)
        {
            retValue.SetInt64(it->second->AsInt64());
        }
        if(innerTypeName == DAVA::VariantType::TYPENAME_UINT64)
        {
            retValue.SetUInt64(it->second->AsUInt64());
        }
        if(innerTypeName == DAVA::VariantType::TYPENAME_FLOAT)
        {
            retValue.SetFloat(it->second->AsFloat());
        }
        if(innerTypeName == DAVA::VariantType::TYPENAME_STRING)
        {
            retValue.SetString(it->second->AsString());
        }
        if(innerTypeName == DAVA::VariantType::TYPENAME_FASTNAME)
        {
            retValue.SetFastName(it->second->AsFastName());
        }
        if(innerTypeName == DAVA::VariantType::TYPENAME_WIDESTRING)
        {
            retValue.SetWideString(it->second->AsWString());
        }
        if(innerTypeName == DAVA::VariantType::TYPENAME_BYTE_ARRAY)
        {
            const Vector<YamlNode*> &byteArrayNoodes = it->second->AsVector();
            int32 size = static_cast<int32>(byteArrayNoodes.size());
            uint8* innerArray = new uint8[size];
            for (int32 i = 0; i < size; ++i)
            {
                int32 val = 0;
                int32 retCode = sscanf(byteArrayNoodes[i]->AsString().c_str(), "%x", &val);
                if ((val < 0) || (val > UCHAR_MAX) || (retCode == 0))
                {
                    delete [] innerArray;
                    return retValue;
                }
                innerArray[i] = static_cast<uint8>(val);
            }
            retValue.SetByteArray(innerArray, size);
            delete [] innerArray;
        }
        if(innerTypeName == DAVA::VariantType::TYPENAME_KEYED_ARCHIVE)
        {
            ScopedPtr<KeyedArchive> innerArch(new KeyedArchive());
            innerArch->LoadFromYamlNode(this);
            retValue.SetKeyedArchive(innerArch);
        }
        if(innerTypeName == DAVA::VariantType::TYPENAME_VECTOR2)
        {
            retValue.SetVector2(it->second->AsVector2());
        }
        if(innerTypeName == DAVA::VariantType::TYPENAME_VECTOR3)
        {
            retValue.SetVector3(it->second->AsVector3());
        }
        if(innerTypeName == DAVA::VariantType::TYPENAME_VECTOR4)
        {
            retValue.SetVector4(it->second->AsVector4());
        }
        if(innerTypeName == DAVA::VariantType::TYPENAME_MATRIX2)
        {
            const YamlNode* firstRowNode  = it->second->Get(0);
            const YamlNode* secondRowNode = it->second->Get(1);
            if(NULL == firstRowNode || NULL == secondRowNode)
            {
                return retValue;
            }
            Vector2 fRowVect = firstRowNode->AsVector2();
            Vector2 sRowVect = secondRowNode->AsVector2();
            retValue.SetMatrix2(Matrix2(fRowVect.x,fRowVect.y,sRowVect.x,sRowVect.y));
        }
        if(innerTypeName == VariantType::TYPENAME_MATRIX3)
        {
            const YamlNode* firstRowNode  = it->second->Get(0);
            const YamlNode* secondRowNode = it->second->Get(1);
            const YamlNode* thirdRowNode  = it->second->Get(2);

            if(NULL == firstRowNode  ||
               NULL == secondRowNode ||
               NULL == thirdRowNode)
            {
                return retValue;
            }
            Vector3 fRowVect = firstRowNode ->AsVector3();
            Vector3 sRowVect = secondRowNode->AsVector3();
            Vector3 tRowVect = thirdRowNode ->AsVector3();

            retValue.SetMatrix3(Matrix3(fRowVect.x,fRowVect.y,fRowVect.z,
                               sRowVect.x,sRowVect.y,sRowVect.z,
                               tRowVect.x,tRowVect.y,tRowVect.z));
        }
        if(innerTypeName == VariantType::TYPENAME_MATRIX4)
        {
            const YamlNode* firstRowNode  = it->second->Get(0);
            const YamlNode* secondRowNode = it->second->Get(1);
            const YamlNode* thirdRowNode  = it->second->Get(2);
            const YamlNode* fourthRowNode = it->second->Get(3);

            if(NULL == firstRowNode || NULL == secondRowNode ||
               NULL == thirdRowNode || NULL == fourthRowNode)
            {
                return retValue;
            }
            Vector4 fRowVect  = firstRowNode ->AsVector4();
            Vector4 sRowVect  = secondRowNode->AsVector4();
            Vector4 tRowVect  = thirdRowNode ->AsVector4();
            Vector4 foRowVect = fourthRowNode->AsVector4();

            retValue.SetMatrix4(Matrix4(fRowVect.x,fRowVect.y,fRowVect.z,fRowVect.w,
                                        sRowVect.x,sRowVect.y,sRowVect.z,sRowVect.w,
                                        tRowVect.x,tRowVect.y,tRowVect.z,tRowVect.w,
                                        foRowVect.x,foRowVect.y,foRowVect.z,foRowVect.w));
        }
        if(innerTypeName == DAVA::VariantType::TYPENAME_COLOR)
        {
            retValue.SetColor(it->second->AsColor());
        }
    }

    return retValue;
}

VariantType YamlNode::AsVariantType(const InspMember* insp) const
{
    if (insp->Desc().type == InspDesc::T_ENUM)
    {
        int32 val = 0;
        if (insp->Desc().enumMap->ToValue(AsString().c_str(), val))
        {
            return VariantType(val);
        }
        else
        {
            DVASSERT(false);
        }
    }
    else if (insp->Desc().type == InspDesc::T_FLAGS)
    {
        int32 val = 0;
        for (uint32 i = 0; i < GetCount(); i++)
        {
            const YamlNode *flagNode = Get(i);
            int32 flag = 0;
            if (insp->Desc().enumMap->ToValue(flagNode->AsString().c_str(), flag))
            {
                val |= flag;
            }
            else
            {
                DVASSERT(false);
            }
        }
        return VariantType(val);
    }
    else if (insp->Type() == MetaInfo::Instance<bool>())
        return VariantType(AsBool());
    else if (insp->Type() == MetaInfo::Instance<int32>())
        return VariantType(AsInt32());
    else if (insp->Type() == MetaInfo::Instance<uint32>())
        return VariantType(AsUInt32());
    else if (insp->Type() == MetaInfo::Instance<String>())
        return VariantType(AsString());
    else if (insp->Type() == MetaInfo::Instance<WideString>())
        return VariantType(AsWString());
    else if (insp->Type() == MetaInfo::Instance<float32>())
        return VariantType(AsFloat());
    else if (insp->Type() == MetaInfo::Instance<Vector2>())
        return VariantType(AsVector2());
    else if (insp->Type() == MetaInfo::Instance<Color>())
        return VariantType(AsColor());
    else if (insp->Type() == MetaInfo::Instance<Vector4>())
        return VariantType(AsVector4());
    else if (insp->Type() == MetaInfo::Instance<FilePath>())
        return VariantType(FilePath(AsString()));
        
    DVASSERT(false);
    return VariantType();
}

const Vector<YamlNode*> & YamlNode::AsVector() const
{
    DVASSERT(GetType() == TYPE_ARRAY);
    if (GetType() == TYPE_ARRAY)
        return objectArray->array;

    return EMPTY_VECTOR;
}

const MultiMap<String, YamlNode*> & YamlNode::AsMap() const
{
    DVASSERT(GetType() == TYPE_MAP);
    if (GetType() == TYPE_MAP)
        return objectMap->ordered;

    return EMPTY_MAP;
}

const YamlNode * YamlNode::Get(uint32 index) const
{
    if (GetType() == TYPE_ARRAY)
    {
        return objectArray->array[index];
    }
    else if (GetType() == TYPE_MAP)
    {
        return objectMap->unordered[index].second;
    }
    return NULL;
}

const String &	YamlNode::GetItemKeyName(uint32 index) const
{
    DVASSERT(GetType() == TYPE_MAP);
    if (GetType() == TYPE_MAP)
    {
        return objectMap->unordered[index].first;
    }
    return EMPTY_STRING;
}

const YamlNode * YamlNode::Get(const String & name) const
{
    //DVASSERT(GetType() == TYPE_MAP);
    if (GetType() == TYPE_MAP)
    {
        MultiMap<String, YamlNode*>::const_iterator iter = objectMap->ordered.find(name);
        if (iter != objectMap->ordered.end())
        {
            return iter->second;
        }
    }
    return NULL;
}

struct EqualToFirst
{
    EqualToFirst(const String &strValue):value(strValue){}
    bool operator()(const std::pair<String, YamlNode*>& val)
    {
        return val.first == value;
    }
    const String &value;
};

void YamlNode::RemoveNodeFromMap(const String & name)
{
    DVASSERT(GetType() == TYPE_MAP);
    MultiMap<String, YamlNode*>::iterator begin = objectMap->ordered.lower_bound(name),
                                          end= objectMap->ordered.upper_bound(name);
    if (begin == end)
        return;

    MultiMap<String, YamlNode*>::iterator it = begin;
    for (; it!=end; ++it)
    {
        SafeRelease(it->second);
    }

    objectMap->ordered.erase(begin, end);

    Vector< std::pair<String, YamlNode*> > &array = objectMap->unordered;
    array.erase( std::remove_if( array.begin(), array.end(), EqualToFirst(name) ), array.end() );
}
YamlNode::eStringRepresentation YamlNode::GetStringRepresentation() const
{
    DVASSERT(GetType() == TYPE_STRING);
    return objectString->style;
}

YamlNode::eArrayRepresentation YamlNode::GetArrayRepresentation() const
{
    DVASSERT(GetType() == TYPE_ARRAY);
    return objectArray->style;
}

YamlNode::eMapRepresentation YamlNode::GetMapRepresentation() const
{
    DVASSERT(GetType() == TYPE_MAP);
    return objectMap->style;
}

YamlNode::eStringRepresentation YamlNode::GetMapKeyRepresentation() const
{
    DVASSERT(GetType() == TYPE_MAP);
    return objectMap->keyStyle;
}

bool YamlNode::GetMapOrderRepresentation() const
{
    DVASSERT(GetType() == TYPE_MAP);
    return objectMap->orderedSave;
}

void YamlNode::InternalSetToString(const VariantType &varType)
{
    DVVERIFY(InitStringFromVariantType(varType));
}

void YamlNode::InternalSetToString(const String &value)
{
    InternalSetString(value, SR_DOUBLE_QUOTED_REPRESENTATION);
}

void YamlNode::InternalAddToMap(const String& name, const VariantType &varType, bool rewritePreviousValue)
{
    YamlNode * node = CreateNodeFromVariantType(varType);
    InternalAddNodeToMap(name, node, rewritePreviousValue);
}

void YamlNode::InternalAddToMap(const String& name, const String &value, bool rewritePreviousValue)
{
    YamlNode * node = CreateStringNode();
    node->InternalSetString(value, SR_DOUBLE_QUOTED_REPRESENTATION);
    InternalAddNodeToMap(name, node, rewritePreviousValue);
}

void YamlNode::InternalAddToArray(const VariantType &varType)
{
    YamlNode * node = CreateNodeFromVariantType(varType);
    InternalAddNodeToArray(node);
}

void YamlNode::InternalAddToArray(const String &value)
{
    YamlNode * node = CreateStringNode();
    node->InternalSetString(value, SR_DOUBLE_QUOTED_REPRESENTATION);
    InternalAddNodeToArray(node);
}

void YamlNode::InternalAddNodeToArray(YamlNode* node)
{
    DVASSERT(GetType() == TYPE_ARRAY);
    objectArray->array.push_back(node);
}

void  YamlNode::InternalAddNodeToMap(const String& name, YamlNode* node, bool rewritePreviousValue)
{
    DVASSERT(GetType() == TYPE_MAP);
    if (rewritePreviousValue)
    {
        RemoveNodeFromMap(name);
    }

    objectMap->ordered.insert(std::pair<String, YamlNode*>(name, node));
    objectMap->unordered.push_back(std::pair<String, YamlNode*>(name, node));
}

void YamlNode::InternalSetString(const String &value, eStringRepresentation style/* = SR_DOUBLE_QUOTED_REPRESENTATION*/)
{
    DVASSERT(GetType() == TYPE_STRING);
    objectString->nwStringValue = value;
    objectString->style = style;
}

void YamlNode::InternalSetMatrix(const float32* array,uint32 dimension)
{
    for (uint32 i = 0; i < dimension; ++i)
    {
        YamlNode* rowNode = CreateArrayNode();
        rowNode->InternalSetVector(&array[i*dimension], dimension);
        InternalAddNodeToArray(rowNode);
    }
    objectArray->style = AR_FLOW_REPRESENTATION;
}

void YamlNode::InternalSetVector(const float32* array,uint32 dimension)
{
    objectArray->array.reserve(dimension);
    for (uint32 i = 0; i < dimension; ++i)
    {
        YamlNode* innerNode = CreateNodeFromVariantType(VariantType(array[i]));
        InternalAddNodeToArray(innerNode);
    }
    objectArray->style = AR_FLOW_REPRESENTATION;
}

void YamlNode::InternalSetByteArray(const uint8* byteArray, int32 byteArraySize)
{
    objectArray->array.reserve(byteArraySize);
    for (int32 i = 0; i < byteArraySize; ++i)
    {
        YamlNode* innerNode = CreateStringNode();
        innerNode->InternalSetString(Format("%x",byteArray[i]), SR_PLAIN_REPRESENTATION);
        InternalAddNodeToArray(innerNode);
    }
    objectArray->style = AR_FLOW_REPRESENTATION;
}

void YamlNode::InternalSetKeyedArchive(KeyedArchive* archive)
{
    //creation array with variables
    const KeyedArchive::ObjectMap& innerArchiveMap = archive->GetArchieveData();
    for (auto it = innerArchiveMap.begin(); it != innerArchiveMap.end(); ++it)
    {
        YamlNode* arrayElementNodeValue = CreateMapNode(true, MR_BLOCK_REPRESENTATION);
        arrayElementNodeValue->InternalAddNodeToMap(it->second->GetTypeName(), CreateNodeFromVariantType(*it->second), false);

        InternalAddNodeToMap(it->first, arrayElementNodeValue, false);
    }
    objectMap->style = MR_BLOCK_REPRESENTATION;
}

bool YamlNode::InitStringFromVariantType(const VariantType &varType)
{
    DVASSERT(GetType() == TYPE_STRING);
    bool result = true;
    switch(varType.GetType())
    {
    case VariantType::TYPE_BOOLEAN:
        {
            InternalSetString(varType.AsBool() ? "true" : "false", SR_PLAIN_REPRESENTATION);
        }
        break;
    case VariantType::TYPE_INT32:
        {
            InternalSetString(Format("%d", varType.AsInt32()), SR_PLAIN_REPRESENTATION);
        }
        break;
    case VariantType::TYPE_FLOAT:
        {
            InternalSetString(Format("%f", varType.AsFloat()), SR_PLAIN_REPRESENTATION);
        }
        break;
    case VariantType::TYPE_STRING:
        {
            InternalSetString(varType.AsString(), SR_DOUBLE_QUOTED_REPRESENTATION);
        }
        break;
    case VariantType::TYPE_WIDE_STRING:
        {
            InternalSetString(UTF8Utils::EncodeToUTF8(varType.AsWideString()), SR_DOUBLE_QUOTED_REPRESENTATION);
        }
        break;
    case VariantType::TYPE_FILEPATH:
        {
            InternalSetString(varType.AsFilePath().GetStringValue(), SR_DOUBLE_QUOTED_REPRESENTATION);
        }
        break;
    case VariantType::TYPE_UINT32:
        {
            InternalSetString(Format("%u", varType.AsUInt32()), SR_PLAIN_REPRESENTATION);
        }
        break;
    case VariantType::TYPE_INT64:
        {
            InternalSetString(Format("%lld", varType.AsInt64()), SR_PLAIN_REPRESENTATION);
        }
        break;
    case VariantType::TYPE_UINT64:
        {
            InternalSetString(Format("%llu", varType.AsUInt64()), SR_PLAIN_REPRESENTATION);
        }
        break;
	case VariantType::TYPE_FASTNAME:
		{
			InternalSetString(varType.AsFastName().c_str(), SR_DOUBLE_QUOTED_REPRESENTATION);
		}
		break;

    default:
        result = false;
        break;
    }
    return result;
}

bool YamlNode::InitArrayFromVariantType(const VariantType &varType)
{
    DVASSERT(GetType() == TYPE_ARRAY);
    bool result = true;
    switch(varType.GetType())
    {
    case VariantType::TYPE_BYTE_ARRAY:
        {
            const uint8* byteArray = varType.AsByteArray();
            int32 byteArraySize = varType.AsByteArraySize();
            InternalSetByteArray(byteArray, byteArraySize);
        }
        break;
    case VariantType::TYPE_VECTOR2:
        {
            const Vector2 & vector = varType.AsVector2();
            InternalSetVector(vector.data, Vector2::AXIS_COUNT);
        }
        break;
    case VariantType::TYPE_VECTOR3:
        {
            const Vector3& vector = varType.AsVector3();
            InternalSetVector(vector.data, Vector3::AXIS_COUNT);
        }
        break;
    case VariantType::TYPE_VECTOR4:
        {
            const Vector4& vector = varType.AsVector4();
            InternalSetVector(vector.data, Vector4::AXIS_COUNT);
        }
        break;
    case VariantType::TYPE_MATRIX2:
        {
            uint32 dimension = 2;
            const Matrix2& matrix = varType.AsMatrix2();
            const float32* array = &matrix._data[0][0];
            InternalSetMatrix(array, dimension);
        }
        break;
    case VariantType::TYPE_MATRIX3:
        {
            uint32 dimension = 3;
            const Matrix3& matrix = varType.AsMatrix3();
            const float32* array = &matrix._data[0][0];
            InternalSetMatrix(array, dimension);
        }
        break;
    case VariantType::TYPE_MATRIX4:
        {
            uint32 dimension = 4;
            const Matrix4& matrix = varType.AsMatrix4();
            const float32* array = &matrix._data[0][0];
            InternalSetMatrix(array, dimension);
        }
        break;
    case VariantType::TYPE_COLOR:
        {
            const Color& color = varType.AsColor();
            InternalSetVector(color.color, Color::CHANNEL_COUNT);
        }
        break;
    default:
        result = false;
        break;
    }

    return result;
}

bool YamlNode::InitMapFromVariantType(const VariantType &varType)
{
    DVASSERT(GetType() == TYPE_MAP);
    bool result = true;
    switch(varType.GetType())
    {
    case VariantType::TYPE_KEYED_ARCHIVE:
        {
            KeyedArchive* archive = varType.AsKeyedArchive();
            InternalSetKeyedArchive(archive);
        }
        break;
    default:
        result = false;
        break;
    }

    return result;
}

YamlNode * YamlNode::CreateNodeFromVariantType(const VariantType &varType)
{
    eType nodeType = VariantTypeToYamlNodeType(varType.GetType());
    YamlNode * node = NULL;
    switch(nodeType)
    {
    case TYPE_STRING:
        {
            node = CreateStringNode();
            DVVERIFY(node->InitStringFromVariantType(varType));
        }
        break;
    case TYPE_ARRAY:
        {
            node = CreateArrayNode();
            DVVERIFY(node->InitArrayFromVariantType(varType));
        }
        break;
    case TYPE_MAP:
        {
            node = CreateMapNode();
            DVVERIFY(node->InitMapFromVariantType(varType));
        }
        break;
    }

    return node;
}

DAVA::YamlNode::eType YamlNode::VariantTypeToYamlNodeType(VariantType::eVariantType variantType)
{
    switch(variantType)
    {
    case VariantType::TYPE_BOOLEAN:
    case VariantType::TYPE_INT32:
    case VariantType::TYPE_FLOAT:
    case VariantType::TYPE_STRING:
    case VariantType::TYPE_WIDE_STRING:
    case VariantType::TYPE_UINT32:
    case VariantType::TYPE_INT64:
    case VariantType::TYPE_UINT64:
    case VariantType::TYPE_FILEPATH:
    case VariantType::TYPE_FASTNAME:
        return TYPE_STRING;

    case VariantType::TYPE_BYTE_ARRAY:
    case VariantType::TYPE_VECTOR2:
    case VariantType::TYPE_VECTOR3:
    case VariantType::TYPE_VECTOR4:
    case VariantType::TYPE_MATRIX2:
    case VariantType::TYPE_MATRIX3:
    case VariantType::TYPE_MATRIX4:
    case VariantType::TYPE_COLOR:
        return TYPE_ARRAY;

    case VariantType::TYPE_KEYED_ARCHIVE:
        return TYPE_MAP;
    default:
        break;
    }
    DVASSERT(false);
    return TYPE_MAP;
}

}