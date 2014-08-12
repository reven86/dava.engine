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

namespace DAVA
{

const char8* YamlNode::SAVE_INDEX_NAME = "##SAVE_INDEX_NAME##";

YamlNode::YamlNode(eType _type, eRepresentationType _repType)
{
    type = _type;
    representationType = _repType;
    mapIndex = 0;
    mapCount = 0;
    isWideString = false;
}

YamlNode::~YamlNode()
{
    for (MultiMap<String, YamlNode*>::iterator t = objectMap.begin(); t != objectMap.end(); ++t)
    {
        YamlNode * object = t->second;
        SafeRelease(object);
    }
    objectMap.clear();

    int32 size = (int32)objectArray.size();
    for (int32 k = 0; k < size; ++k)
    {
        SafeRelease(objectArray[k]);
    }
    objectArray.clear();
}

void YamlNode::Print(int32 identation)
{
    if (type == TYPE_STRING)
    {
        const char * str = nwStringValue.c_str();
        printf("%s", str);
    }else if (type == TYPE_ARRAY)
    {
        printf("[");
        for (int32 k = 0; k < (int32)objectArray.size(); ++k)
        {
            objectArray[k]->Print(identation + 1);
            printf(",");
        }
        printf("]");
    }else if (type == TYPE_MAP)
    {
        printf("{");
        for (MultiMap<String, YamlNode*>::iterator t = objectMap.begin(); t != objectMap.end(); ++t)
        {
            printf("key(%s, %d): ", t->first.c_str(), t->second->mapIndex);
            t->second->Print(identation + 1);
            printf(",");
        }
        printf("}");
    }
}

void YamlNode::PrintToFile(DAVA::File* file, uint32 identationDepth) const
{
    if (type == TYPE_STRING)
    {
        file->WriteNonTerminatedString(nwStringValue);
    }
    else if (type == TYPE_ARRAY)
    {
        //check if there are no maps inside

        bool isSimpleContent = true;
        for (int32 k = 0; k < (int32)objectArray.size(); ++k)
        {
            if(objectArray[k]->IsContainingMap())
            {
                isSimpleContent = false;
                break;
            }
        }

        if(isSimpleContent)
        {
              file->WriteNonTerminatedString("[");
        }

        for (int32 k = 0; k < (int32)objectArray.size(); ++k)
        {
            objectArray[k]->PrintToFile(file, identationDepth);
            if(k != ((int32)objectArray.size() - 1) && isSimpleContent )
            {
                file->WriteNonTerminatedString(", ");
            }
        }
        if(isSimpleContent)
        {
            file->WriteNonTerminatedString("]");
        }
    }
    else if (type == TYPE_MAP)
    {
        const int32 IDENTATION_SPACES_COUNT = 4;
        int32 spacesCount = identationDepth * IDENTATION_SPACES_COUNT;

        char8* spacesBuffer = new char8[spacesCount];
        memset(spacesBuffer, 0x20, spacesCount);
        String spaces(spacesBuffer, spacesCount);
        delete[] spacesBuffer;

        file->WriteNonTerminatedString("\r\n" + spaces );
        for (MultiMap<String, YamlNode*>::const_iterator t = objectMap.begin(); t != objectMap.end(); ++t )
        {

            String strToFile( t->first + ": ");
            file->WriteNonTerminatedString(strToFile);
            t->second->PrintToFile(file, ++identationDepth);
        }
    }
}

int32 YamlNode::GetCount() const
{
    switch (type)
    {
        case TYPE_MAP: return (int32)objectMap.size();
        case TYPE_ARRAY: return (int32)objectArray.size();
        default: break;
    }
    return 1;
}
int32  YamlNode::AsInt() const
{
    return AsInt32();
}

int32 YamlNode::AsInt32() const
{
    int32 ret;
    sscanf(nwStringValue.c_str(), "%d", &ret);
    return ret;
}

uint32 YamlNode::AsUInt32() const
{
    uint32 ret;
    sscanf(nwStringValue.c_str(), "%u", &ret);
    return ret;
}

int64 YamlNode::AsInt64() const
{
    int64 ret;
    sscanf(nwStringValue.c_str(), "%lld", &ret);
    return ret;
}

uint64 YamlNode::AsUInt64() const
{
    uint64 ret;
    sscanf(nwStringValue.c_str(), "%llu", &ret);
    return ret;
}

float32	YamlNode::AsFloat() const
{
    float32 ret;
    sscanf(nwStringValue.c_str(), "%f", &ret);
    return ret;
}

const String & YamlNode::AsString() const
{
    return nwStringValue;
}

FastName YamlNode::AsFastName() const
{
    return FastName(nwStringValue);
}

bool YamlNode::AsBool() const
{
    return ("true" == nwStringValue);
}

const WideString & YamlNode::AsWString() const
{
    return stringValue;
}

Vector2	YamlNode::AsPoint() const
{
    Vector2 result;
    if (type == TYPE_ARRAY)
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
    if (type == TYPE_ARRAY)
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
    if (type == TYPE_ARRAY)
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
    if (type == TYPE_ARRAY)
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
    if (type == TYPE_ARRAY)
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
        if(innerTypeName == DAVA::VariantType::TYPENAME_WIDESTRING)
        {
            retValue.SetWideString(it->second->AsWString());
        }
        if(innerTypeName == DAVA::VariantType::TYPENAME_BYTE_ARRAY)
        {
            const Vector<YamlNode*> &byteArrayNoodes = it->second->AsVector();
            int32 size = byteArrayNoodes.size();
            uint8* innerArray = new uint8[size];
            for (int32 i = 0; i < size; ++i )
            {
                int val = 0;
                int retCode = sscanf(byteArrayNoodes[i]->AsString().c_str(), "%x", &val);
                if(val > CHAR_MAX || retCode == 0)
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
            if(NULL == firstRowNode || NULL == secondRowNode )
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
               NULL == thirdRowNode )
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

const Vector<YamlNode*> & YamlNode::AsVector() const
{
    return objectArray;
}

const MultiMap<String, YamlNode*> & YamlNode::AsMap() const
{
    return objectMap;
}



const YamlNode * YamlNode::Get(int32 index) const
{
    if (type == TYPE_ARRAY)
    {
        return objectArray[index];
    }else if (type == TYPE_MAP)
    {
        return objectArray[index];
        /*Map<String, YamlNode*>::const_iterator end = objectMap.end();
        for (Map<String, YamlNode*>::iterator t = objectMap.begin(); t != end; ++t)
        {
            YamlNode * n = t->second;
            if (n->mapIndex == index)return n;
        }*/
    }
    return 0;
}

static String emptyString = String("");

const String &	YamlNode::GetItemKeyName(int32 index) const
{
    if (type == TYPE_MAP)
    {
        MultiMap<String, YamlNode*>::const_iterator end = objectMap.end();
        for (MultiMap<String, YamlNode*>::const_iterator t = objectMap.begin(); t != end; ++t)
        {
            YamlNode * n = t->second;
            if (n->mapIndex == index)return t->first;
        }
    }
    return emptyString;
}

const YamlNode * YamlNode::Get(const String & name) const
{
    if (type == TYPE_MAP)
    {
        MultiMap<String, YamlNode*>::const_iterator t;
        if ((t = objectMap.find(name)) != objectMap.end())
        {
            return t->second;
        }
    }
    return 0;
}

// "Adders" for different types.
void YamlNode::Add(const String& name, bool value)
{
    InternalSet(name, value, false);
}

void YamlNode::Add(const String& name, int32 value)
{
    InternalSet(name, value, false);
}

void YamlNode::Add(const String& name, float32 value)
{
    InternalSet(name, value, false);
}

void YamlNode::Add(const String& name, const char8* value)
{
    InternalSet(name, value, false);
}

void YamlNode::Add(const String& name, const String& value)
{
    InternalSet(name, value, false);
}

void YamlNode::Add(const String& name, const WideString& value)
{
    InternalSet(name, value, false);
}

void YamlNode::Add(const String& name, const Vector2& value)
{
    InternalSet(name, value, false);
}

void YamlNode::Add(const String& name, const Vector3& value)
{
    InternalSet(name, value, false);
}

void YamlNode::Add(const String& name, const Vector4& value)
{
    InternalSet(name, value, false);
}

void YamlNode::Add(const String& name, VariantType* varType)
{
    InternalSet(name, varType, false);
}

void YamlNode::AddNodeToMap(const String& name, YamlNode* node)
{
    InternalSetNodeToMap(name, node, false);
}

// "Setters" for different types.
void YamlNode::Set(const String& name, bool value)
{
    InternalSet(name, value, true);
}

void YamlNode::Set(const String& name, int32 value)
{
    InternalSet(name, value, true);
}

void YamlNode::Set(const String& name, float32 value)
{
    InternalSet(name, value, true);
}

void YamlNode::Set(const String& name, const char8* value)
{
    InternalSet(name, value, true);
}

void YamlNode::Set(const String& name, const String& value)
{
    InternalSet(name, value, true);
}

void YamlNode::Set(const String& name, const WideString& value)
{
    InternalSet(name, value, true);
}

void YamlNode::Set(const String& name, const Vector2& value)
{
    InternalSet(name, value, true);
}

void YamlNode::Set(const String& name, const Vector3& value)
{
    InternalSet(name, value, true);
}

void YamlNode::Set(const String& name, const Vector4& value)
{
    InternalSet(name, value, true);
}

void YamlNode::Set(const String& name, VariantType* varType)
{
    InternalSet(name, varType, true);
}

void YamlNode::SetNodeToMap(const String& name, YamlNode* node)
{
    InternalSetNodeToMap(name, node, true);
}

void  YamlNode::AddNodeToArray(YamlNode* node)
{
    DVASSERT(this->type == TYPE_ARRAY);
    objectArray.push_back(node);
}

void  YamlNode::AddValueToArray(int32 value)
{
    DVASSERT(this->type == TYPE_ARRAY);
    YamlNode* childNode = new YamlNode(YamlNode::TYPE_STRING);
    childNode->Set("", value);

    AddNodeToArray(childNode);
}

void  YamlNode::AddValueToArray(float32 value)
{
    DVASSERT(this->type == TYPE_ARRAY);
    YamlNode* childNode = new YamlNode(YamlNode::TYPE_STRING);
    childNode->Set("", value);

    AddNodeToArray(childNode);
}

void  YamlNode::AddValueToArray(const Vector2& value)
{
    DVASSERT(this->type == TYPE_ARRAY);
    YamlNode* childNode = new YamlNode(YamlNode::TYPE_STRING);
    childNode->Set("", value);

    AddNodeToArray(childNode);
}

void  YamlNode::AddValueToArray(const Vector3& value)
{
    DVASSERT(this->type == TYPE_ARRAY);
    YamlNode* childNode = new YamlNode(YamlNode::TYPE_STRING);
    childNode->Set("", value);

    AddNodeToArray(childNode);
}

void  YamlNode::AddValueToArray(const Vector4& value)
{
    DVASSERT(this->type == TYPE_ARRAY);
    YamlNode* childNode = new YamlNode(YamlNode::TYPE_STRING);
    childNode->Set("", value);

    AddNodeToArray(childNode);
}

void  YamlNode::AddValueToArray(VariantType* value)
{
    DVASSERT(this->type == TYPE_ARRAY);
    YamlNode* childNode = new YamlNode(YamlNode::TYPE_STRING);
    childNode->Set("", value);

    AddNodeToArray(childNode);
}

void  YamlNode::AddValueToArray(const String& value)
{
    DVASSERT(this->type == TYPE_ARRAY);
    YamlNode* childNode = new YamlNode(YamlNode::TYPE_STRING);
    childNode->Set("", value);

    AddNodeToArray(childNode);
}

void YamlNode::RemoveNodeFromMap(const String & name)
{
    MultiMap<String, YamlNode*>::iterator t = objectMap.find(name);
    if (t != objectMap.end())
    {
        objectMap.erase(t);
    }
}

void  YamlNode::InitFromVariantType(VariantType* varType)
{
    type = TYPE_MAP;

    //create key
    String variantName = VariantType::variantNamesMap[varType->type].variantName;

    //create value node
    YamlNode* valueNode = new YamlNode(YamlNode::TYPE_STRING);
    valueNode->FillContentAccordingToVariantTypeValue(varType);
    objectMap.insert(std::pair<String, YamlNode*>(variantName, valueNode));
}

void  YamlNode::FillContentAccordingToVariantTypeValue(VariantType* varType)
{
    type = TYPE_STRING;
    isWideString = false;
    char str[30];
    str[0]='\0';
    switch(varType->type)
    {
        case VariantType::TYPE_BOOLEAN:
        {
            nwStringValue = varType->AsBool() ? "true" : "false";
        }
            break;
        case VariantType::TYPE_INT32:
        {
            sprintf(str, "%d", varType->AsInt32());
        }
            break;
        case VariantType::TYPE_FLOAT:
        {
            String letterRepresentation(FloatToCuttedString(varType->AsFloat()));
            nwStringValue = letterRepresentation;
            stringValue = StringToWString(letterRepresentation);
        }
            break;
        case VariantType::TYPE_STRING:
        {
            String strToInitWith = '"'+varType->AsString()+'"';
            nwStringValue = String(strToInitWith);
            stringValue = StringToWString(strToInitWith);
        }
            break;
        case VariantType::TYPE_WIDE_STRING:
        {
            stringValue = L'"' + varType->AsWideString() +L'"';
            nwStringValue = WStringToString(stringValue);
            isWideString = true;
        }
            break;
        case VariantType::TYPE_UINT32:
        {
            sprintf(str, "%u", varType->AsUInt32());
        }
            break;
        case VariantType::TYPE_INT64:
        {
            sprintf(str, "%lld", varType->AsInt64());
        }
            break;
        case VariantType::TYPE_UINT64:
        {
            sprintf(str, "%llu", varType->AsUInt64());
        }
            break;
        case VariantType::TYPE_BYTE_ARRAY:
        {
            //! need to create subnodes
            type = TYPE_ARRAY;
            const uint8* byteArray = varType->AsByteArray();
            int32 byteArraySize = varType->AsByteArraySize();
            String arrayRepresentation = "";
            for (int32 i = 0; i < byteArraySize; ++i)
            {
                char letter[10];
                sprintf(letter, "%x",byteArray[i]);
                String letterRepresentation(letter);
                YamlNode* innerNode = new YamlNode(TYPE_STRING);
                innerNode->nwStringValue = letterRepresentation;
                innerNode->stringValue = StringToWString(letterRepresentation);
                objectArray.push_back(innerNode);
            }
        }
            break;
        case VariantType::TYPE_KEYED_ARCHIVE:
        {
            KeyedArchive* archive = varType->AsKeyedArchive();
            type = TYPE_ARRAY;

            //creation array with variables
            const Map<String, VariantType*> & innerArchiveMap =  archive->GetArchieveData();
            for (Map<String, VariantType*>::const_iterator it = innerArchiveMap.begin(); it != innerArchiveMap.end(); ++it)
            {
                YamlNode* arrayElementNode = new YamlNode(TYPE_MAP);
                YamlNode* arrayElementNodeValue = new YamlNode(TYPE_MAP);

                arrayElementNodeValue->InitFromVariantType(it->second);
                arrayElementNode->objectMap.insert(std::pair<String, YamlNode*>(it->first, arrayElementNodeValue));

                objectArray.push_back(arrayElementNode);
            }

        }
            break;
        case VariantType::TYPE_VECTOR2:
        {
            type = TYPE_ARRAY;
            const Vector2 & vector = varType->AsVector2();
            ProcessVector(vector.data,COUNT_OF(vector.data));
        }
            break;
        case VariantType::TYPE_VECTOR3:
        {
            type = TYPE_ARRAY;
            const Vector3& vector = varType->AsVector3();
            ProcessVector(vector.data,COUNT_OF(vector.data));
        }
            break;
        case VariantType::TYPE_VECTOR4:
        {
            type = TYPE_ARRAY;
            const Vector4& vector = varType->AsVector4();
            ProcessVector(vector.data,COUNT_OF(vector.data));
        }
            break;
        case VariantType::TYPE_MATRIX2:
        {
            type = TYPE_ARRAY;
            uint32 dimension = 2;
            const Matrix2& matrix = varType->AsMatrix2();
            const float32* array = &matrix._data[0][0];
            ProcessMatrix(array, dimension);
        }
            break;
        case VariantType::TYPE_MATRIX3:
        {
            type = TYPE_ARRAY;
            uint32 dimension = 3;
            const Matrix3& matrix = varType->AsMatrix3();
            const float32* array = &matrix._data[0][0];
            ProcessMatrix( array, dimension );
        }
            break;
        case VariantType::TYPE_MATRIX4:
        {
            type = TYPE_ARRAY;
            uint32 dimension = 4;
            const Matrix4& matrix = varType->AsMatrix4();
            const float32* array = &matrix._data[0][0];
            ProcessMatrix( array, dimension );
        }
            break;
        case VariantType::TYPE_COLOR:
            {
                type = TYPE_ARRAY;
                const Color& color = varType->AsColor();
                ProcessVector(color.color,COUNT_OF(color.color));
            }
            break;
        default:
            break;
    }

    if(str[0] != '\0')
    {
        String value(str);
        nwStringValue = value;
        stringValue = StringToWString(value);
    }
}

    bool IsZero(char c)
    {
        return (c == '0');
    }

void YamlNode::ProcessMatrix(const float32* array,uint32 dimension)
{
    YamlNode* rowNode;
    for (uint32 i = 0; i < dimension; ++i)
    {
        rowNode = new YamlNode(TYPE_ARRAY);
        rowNode->objectArray.reserve(dimension);

        YamlNode* columnNode = NULL;
        for (uint32 j = 0; j < dimension; ++j)
        {
            const float32* elementOfArray = array + ((i*dimension)+j);
            String letterRepresentation(FloatToCuttedString(*elementOfArray));
            columnNode = new YamlNode(TYPE_STRING);
            columnNode->nwStringValue = letterRepresentation;
            columnNode->stringValue = StringToWString(letterRepresentation);
            rowNode->objectArray.push_back(columnNode);
        }
        objectArray.push_back(rowNode);
    }
}

void YamlNode::ProcessVector(const float32* array,uint32 dimension)
{
    for (uint32 i = 0; i < dimension; ++i)
    {
        const float32* elementOfArray = array + i;
        String letterRepresentation(FloatToCuttedString(*elementOfArray));
        YamlNode* innerNode = new YamlNode(TYPE_STRING);
        innerNode->nwStringValue = letterRepresentation;
        innerNode->stringValue = StringToWString(letterRepresentation);
        objectArray.push_back(innerNode);
    }
}

bool YamlNode::IsContainingMap() const
{
    bool retValue = false;
    switch (type)
    {
        case YamlNode::TYPE_STRING:
            break;
        case YamlNode::TYPE_MAP:
            retValue =  true;
            break;
        case YamlNode::TYPE_ARRAY:
        {
            for (Vector<YamlNode*>::const_iterator it = objectArray.begin(); it != objectArray.end(); ++it)
            {
                retValue =  (*it)->IsContainingMap();
                if(retValue)
                    break;
            }
        }
            break;
    }
    return retValue;
}

String YamlNode::FloatToCuttedString(float f)
{
    char letter[20];
    sprintf(letter, "%.4f",f);
    String letterRepresentation(letter);
    return letterRepresentation;
}

void YamlNode::InitFromKeyedArchive(KeyedArchive* archive)
{
    type = TYPE_MAP;


    //creation array with variables
    YamlNode* arrayContentNode = new YamlNode(YamlNode::TYPE_ARRAY);
    const Map<String, VariantType*> & innerArchiveMap =  archive->GetArchieveData();

    arrayContentNode->objectArray.reserve(innerArchiveMap.size());
    for (Map<String, VariantType*>::const_iterator it = innerArchiveMap.begin(); it != innerArchiveMap.end(); ++it)
    {
        YamlNode* arrayElementNode = new YamlNode(TYPE_MAP);
        YamlNode* arrayElementNodeValue = new YamlNode(TYPE_MAP);

        arrayElementNodeValue->InitFromVariantType(it->second);
        arrayElementNode->objectMap.insert(std::pair<String, YamlNode*>(it->first, arrayElementNodeValue));

        arrayContentNode->objectArray.push_back(arrayElementNode);
    }

    objectMap.insert(std::pair<String, YamlNode*>(VariantType::TYPENAME_KEYED_ARCHIVE, arrayContentNode));

}

void YamlNode::InternalSet(const String& name, bool value, bool rewritePreviousValue)
{
    if (type == TYPE_MAP)
    {
        if (rewritePreviousValue)
        {
            RemoveNodeFromMap(name);
        }

        YamlNode* stringNode = new YamlNode(YamlNode::TYPE_STRING);
        String strValue = (value == true) ? "true" : "false";
        stringNode->Set(name, strValue);
        objectMap.insert(std::pair<String, YamlNode*>(name, stringNode));
    }
    else if (type == TYPE_STRING)
    {
        // Just initialize the value.
        VariantType variantValue;
        variantValue.SetBool(value);
        FillContentAccordingToVariantTypeValue(&variantValue);
    }
}

void YamlNode::InternalSet(const String& name, int32 value, bool rewritePreviousValue)
{
    if (type == TYPE_MAP)
    {
        if (rewritePreviousValue)
        {
            RemoveNodeFromMap(name);
        }

        YamlNode* stringNode = new YamlNode(YamlNode::TYPE_STRING);
        stringNode->Set(name, value);
        objectMap.insert(std::pair<String, YamlNode*>(name,stringNode));
    }
    else if (type == TYPE_STRING)
    {
        // Just initialize the value.
        VariantType variantValue;
        variantValue.SetInt32(value);
        FillContentAccordingToVariantTypeValue(&variantValue);
    }
}

void YamlNode::InternalSet(const String& name, float32 value, bool rewritePreviousValue)
{
    if (type == TYPE_MAP)
    {
        if (rewritePreviousValue)
        {
            RemoveNodeFromMap(name);
        }

        YamlNode* stringNode = new YamlNode(YamlNode::TYPE_STRING);
        stringNode->Set(name, value);
        objectMap.insert(std::pair<String, YamlNode*> (name, stringNode));
    }
    else if (type == TYPE_STRING)
    {
        // Just initialize the value.
        VariantType variantValue;
        variantValue.SetFloat(value);
        FillContentAccordingToVariantTypeValue(&variantValue);
    }
}

void YamlNode::InternalSet(const String& name, const char8* value, bool rewritePreviousValue)
{
    if (rewritePreviousValue)
    {
        RemoveNodeFromMap(name);
    }

    Set(name, (const String&)value);
}

void YamlNode::InternalSet(const String& name, const String& value, bool rewritePreviousValue)
{
    if (type == TYPE_MAP)
    {
        if (rewritePreviousValue)
        {
            RemoveNodeFromMap(name);
        }

        // For Maps just add the new String node.
        YamlNode* stringNode = new YamlNode(YamlNode::TYPE_STRING);
        stringNode->Set(name, value);
        objectMap.insert(std::pair<String, YamlNode*> (name, stringNode));
    }
    else if (type == TYPE_STRING)
    {
        // Just initialize the value.
        VariantType variantValue;
        variantValue.SetString(value);
        FillContentAccordingToVariantTypeValue(&variantValue);
    }
}

void YamlNode::InternalSet(const String& name, const WideString& value, bool rewritePreviousValue)
{
    if (type == TYPE_MAP)
    {
        if (rewritePreviousValue)
        {
            RemoveNodeFromMap(name);
        }

        YamlNode* stringNode = new YamlNode(YamlNode::TYPE_STRING);
        stringNode->Set(name, value);
        objectMap.insert(std::pair<String, YamlNode*> (name, stringNode));
    }
    else if (type == TYPE_STRING)
    {
        // Just initialize the value.
        VariantType variantValue;
        variantValue.SetWideString(value);
        FillContentAccordingToVariantTypeValue(&variantValue);
    }
}

void YamlNode::InternalSet(const String& name, const Vector2& value, bool rewritePreviousValue)
{
    if (type == TYPE_MAP)
    {
        if (rewritePreviousValue)
        {
            RemoveNodeFromMap(name);
        }

        YamlNode* stringNode = new YamlNode(YamlNode::TYPE_STRING);
        stringNode->Set(name, value);
        objectMap.insert(std::pair<String, YamlNode*> (name, stringNode));
    }
    else if (type == TYPE_STRING)
    {
        VariantType variantValue;
        variantValue.SetVector2(value);
        FillContentAccordingToVariantTypeValue(&variantValue);
    }
}

void YamlNode::InternalSet(const String& name, const Vector3& value, bool rewritePreviousValue)
{
    if (type == TYPE_MAP)
    {
        if (rewritePreviousValue)
        {
            RemoveNodeFromMap(name);
        }

        YamlNode* stringNode = new YamlNode(YamlNode::TYPE_STRING);
        stringNode->Set(name, value);
        objectMap.insert(std::pair<String, YamlNode*> (name, stringNode));
    }
    else if (type == TYPE_STRING)
    {
        // Just initialize the value.
        VariantType variantValue;
        variantValue.SetVector3(value);
        FillContentAccordingToVariantTypeValue(&variantValue);
    }
}

void YamlNode::InternalSet(const String& name, const Vector4& value, bool rewritePreviousValue)
{
    if (type == TYPE_MAP)
    {
        if (rewritePreviousValue)
        {
            RemoveNodeFromMap(name);
        }

        YamlNode* stringNode = new YamlNode(YamlNode::TYPE_STRING);
        stringNode->Set(name, value);
        objectMap.insert(std::pair<String, YamlNode*> (name, stringNode));
    }
    else if (type == TYPE_STRING)
    {
        // Just initialize the value.
        VariantType variantValue;
        variantValue.SetVector4(value);
        FillContentAccordingToVariantTypeValue(&variantValue);
    }
}

void YamlNode::InternalSet(const String& name, VariantType* varType, bool rewritePreviousValue)
{
    if (type == TYPE_MAP)
    {
        if (rewritePreviousValue)
        {
            RemoveNodeFromMap(name);
        }

        // For Maps just add the new String node.
        YamlNode* stringNode = new YamlNode(YamlNode::TYPE_STRING);
        stringNode->Set(name, varType);
        objectMap.insert(std::pair<String, YamlNode*> (name, stringNode));
    }
    else if (type == TYPE_STRING)
    {
        FillContentAccordingToVariantTypeValue(varType);
    }
}

void  YamlNode::InternalSetNodeToMap(const String& name, YamlNode* node, bool rewritePreviousValue)
{
    DVASSERT(this->type == TYPE_MAP);
    if (rewritePreviousValue)
    {
        RemoveNodeFromMap(name);
    }

    objectMap.insert(std::pair<String, YamlNode*> (name, node));
}
}