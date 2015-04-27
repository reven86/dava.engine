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


#include "FileSystem/KeyedArchive.h"
#include "FileSystem/File.h"
#include "Utils/Utils.h"
#include "FileSystem/DynamicMemoryFile.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/VariantType.h"
#include "FileSystem/YamlEmitter.h"

namespace DAVA 
{
	
KeyedArchive::KeyedArchive()
{
}
    
KeyedArchive::KeyedArchive(const KeyedArchive &arc)
{
    const Map<String, VariantType*> &customMap = arc.GetArchieveData();
    for (Map<String, VariantType*>::const_iterator it = customMap.begin(); it != customMap.end(); it++)
    {
        SetVariant(it->first, *it->second);
    }
}

KeyedArchive::~KeyedArchive()
{
    DeleteAllKeys();
}

bool KeyedArchive::Load(const FilePath & pathName)
{
	File * archive = File::Create(pathName, File::OPEN|File::READ);
	if (!archive)return false;

	Load(archive);

	SafeRelease(archive);
	return true;
}
	
bool KeyedArchive::Load(File *archive)
{
    char header[2];
    uint32 wasRead = archive->Read(header, 2);
    if (wasRead != 2)
    {
        Logger::Error("[KeyedArchive] error loading keyed archive from file: %s, filesize: %d", archive->GetFilename().GetAbsolutePathname().c_str(), archive->GetSize());
    }
    else if ((header[0] != 'K') || (header[1] != 'A'))
    {
        archive->Seek(0,File::SEEK_FROM_START);
        while(!archive->IsEof())
        {
            VariantType key;
            key.Read(archive);
            if (archive->IsEof())break;
            VariantType *value = new VariantType();
            value->Read(archive);
            objectMap[key.AsString()] = value;
        }
        return true;
    }
    
    uint16 version = 0;
    archive->Read(&version, 2);
    if (version != 1)
    {
        Logger::Error("[KeyedArchive] error loading keyed archive, because version is incorrect");
        return false;
    }
    uint32 numberOfItems = 0;
    archive->Read(&numberOfItems, 4);
    
    for (uint32 item = 0; item < numberOfItems; ++item)
	{
		VariantType key;
		key.Read(archive);
		if (archive->IsEof())break;
        VariantType *value = new VariantType();
        value->Read(archive);
		objectMap[key.AsString()] = value;
	}
	return true;
}
	
bool KeyedArchive::Save(const FilePath & pathName) const
{
	File * archive = File::Create(pathName, File::CREATE|File::WRITE);
	if (!archive)return false;

	Save(archive);

	SafeRelease(archive);
	return true;
}

bool KeyedArchive::Save(File *archive) const
{
    char header[2];
    uint16 version = 1;
    header[0] = 'K'; header[1] = 'A';
    
    archive->Write(header, 2);
    archive->Write(&version, 2);
    uint32 size = static_cast<uint32>(objectMap.size());
    archive->Write(&size, 4);
    
	for (Map<String, VariantType*>::const_iterator it = objectMap.begin(); it != objectMap.end(); ++it)
	{
		VariantType key;
		key.SetString(it->first);
		key.Write(archive);
		it->second->Write(archive);
	}
	return true;
}
    
bool KeyedArchive::LoadFromYamlFile(const FilePath & pathName)
{
	YamlParser	*parser	= YamlParser::Create(pathName);
    if(NULL == parser)
    {
        return false;
    }
	
    YamlNode *rootNode = parser->GetRootNode();
    bool retValue = LoadFromYamlNode(rootNode);
    
	SafeRelease(parser);
	return retValue;
}

bool KeyedArchive::LoadFromYamlNode(const YamlNode* rootNode)
{
    if(NULL == rootNode)
    {
        return  false;
    }

    const YamlNode * archieveNode = rootNode->Get(VariantType::TYPENAME_KEYED_ARCHIVE);
    if(!archieveNode)
    {
        return false;
    }

    int32 count = archieveNode->GetCount();
    for (int32 i = 0; i < count; ++i)
    {
		const YamlNode * node = archieveNode->Get(i);
		const String &variableNameToArchMap = archieveNode->GetItemKeyName(i);

        VariantType *value = new VariantType(node->AsVariantType());
                
        if(value->GetType() == VariantType::TYPE_NONE)
        {
            SafeDelete(value);
            continue;
        }

        objectMap[variableNameToArchMap] = value;
    }
    
    return true;
}

   
bool KeyedArchive::SaveToYamlFile(const FilePath & pathName)
{
    ScopedPtr<YamlNode> node( YamlNode::CreateMapNode() );
    node->Set(VariantType::TYPENAME_KEYED_ARCHIVE, VariantType(this));

    return YamlEmitter::SaveToYamlFile(pathName, node);
}

void KeyedArchive::SetBool(const String & key, bool value)
{
    DeleteKey(key);
	VariantType *variantValue = new VariantType();
	variantValue->SetBool(value);
	objectMap[key] = variantValue;
}

void KeyedArchive::SetInt32(const String & key, int32 value)
{
    DeleteKey(key);
	VariantType *variantValue = new VariantType();
	variantValue->SetInt32(value);
	objectMap[key] = variantValue;
}
    
void KeyedArchive::SetUInt32(const String & key, uint32 value)
{
    DeleteKey(key);
	VariantType *variantValue = new VariantType();
    variantValue->SetUInt32(value);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetFloat(const String & key, float32 value)
{
    DeleteKey(key);
	VariantType *variantValue = new VariantType();
	variantValue->SetFloat(value);
	objectMap[key] = variantValue;
}

void KeyedArchive::SetString(const String & key, const String & value)
{
    DeleteKey(key);
	VariantType *variantValue = new VariantType();
	variantValue->SetString(value);
	objectMap[key] = variantValue;
}

void KeyedArchive::SetWideString(const String & key, const WideString & value)
{
    DeleteKey(key);
	VariantType *variantValue = new VariantType();
	variantValue->SetWideString(value);
	objectMap[key] = variantValue;
}

void KeyedArchive::SetFastName(const String & key, const FastName & value)
{
    DeleteKey(key);
    VariantType *variantValue = new VariantType();
    variantValue->SetFastName(value);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetByteArray(const String & key, const uint8 * value, int32 arraySize)
{
    VariantType *variantValue = new VariantType();
    variantValue->SetByteArray(value, arraySize);
    
    DeleteKey(key);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetVariant(const String & key, const VariantType &value)
{
    DeleteKey(key);
	VariantType *variantValue = new VariantType(value);
	objectMap[key] = variantValue;
}
    
void KeyedArchive::SetByteArrayFromArchive(const String & key, KeyedArchive * archive)
{
    //DVWARNING(false, "Method is depriceted! Use SetArchive()");
    DynamicMemoryFile * file = DynamicMemoryFile::Create(File::CREATE | File::WRITE);
    archive->Save(file);
    SetByteArray(key, (uint8*)file->GetData(), file->GetSize());
    SafeRelease(file);
}

void KeyedArchive::SetArchive(const String & key, KeyedArchive * archive)
{
    VariantType *variantValue = new VariantType();
    variantValue->SetKeyedArchive(archive);
    
    DeleteKey(key);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetInt64(const String & key, const int64 &value)
{
    DeleteKey(key);
	VariantType *variantValue = new VariantType();
	variantValue->SetInt64(value);
	objectMap[key] = variantValue;
}

void KeyedArchive::SetUInt64(const String & key, const uint64 &value)
{
    DeleteKey(key);
	VariantType *variantValue = new VariantType();
	variantValue->SetUInt64(value);
	objectMap[key] = variantValue;
}

void KeyedArchive::SetVector2(const String & key, const Vector2 &value)
{
    DeleteKey(key);
	VariantType *variantValue = new VariantType();
	variantValue->SetVector2(value);
	objectMap[key] = variantValue;
}

void KeyedArchive::SetVector3(const String & key, const Vector3 &value)
{
    DeleteKey(key);
	VariantType *variantValue = new VariantType();
	variantValue->SetVector3(value);
	objectMap[key] = variantValue;
}

void KeyedArchive::SetVector4(const String & key, const Vector4 &value)
{
    DeleteKey(key);
	VariantType *variantValue = new VariantType();
	variantValue->SetVector4(value);
	objectMap[key] = variantValue;
}

void KeyedArchive::SetMatrix2(const String & key, const Matrix2 &value)
{
    DeleteKey(key);
	VariantType *variantValue = new VariantType();
	variantValue->SetMatrix2(value);
	objectMap[key] = variantValue;
}

void KeyedArchive::SetMatrix3(const String & key, const Matrix3 &value)
{
    DeleteKey(key);
	VariantType *variantValue = new VariantType();
	variantValue->SetMatrix3(value);
	objectMap[key] = variantValue;
}

void KeyedArchive::SetMatrix4(const String & key, const Matrix4 &value)
{
    DeleteKey(key);
	VariantType *variantValue = new VariantType();
	variantValue->SetMatrix4(value);
	objectMap[key] = variantValue;
}

void KeyedArchive::SetColor(const String& key, const Color& value)
{
    DeleteKey(key);
    VariantType* variantValue = new VariantType();
    variantValue->SetColor(value);
    objectMap[key] = variantValue;
}
	
    
bool KeyedArchive::IsKeyExists(const String & key)
{
	Map<String, VariantType*>::iterator t = objectMap.find(key);
	if (t != objectMap.end())
	{
		return true;
	}
	return false;
}

bool KeyedArchive::GetBool(const String & key, bool defaultValue)
{
	if (IsKeyExists(key))
		return objectMap[key]->AsBool();
	return defaultValue;
}

int32 KeyedArchive::GetInt32(const String & key, int32 defaultValue)
{
	if (IsKeyExists(key))
		return objectMap[key]->AsInt32();
	return defaultValue;
}

uint32 KeyedArchive::GetUInt32(const String & key, uint32 defaultValue)
{
    if (IsKeyExists(key))
        return objectMap[key]->AsUInt32();
    return defaultValue;
}

float32 KeyedArchive::GetFloat(const String & key, float32 defaultValue)
{
	if (IsKeyExists(key))
		return objectMap[key]->AsFloat();
	return defaultValue;
}

String KeyedArchive::GetString(const String & key, const String & defaultValue)
{
	if (IsKeyExists(key))
		return objectMap[key]->AsString();
	return defaultValue;
}

WideString KeyedArchive::GetWideString(const String & key, const WideString & defaultValue)
{
	if (IsKeyExists(key))
		return objectMap[key]->AsWideString();
	return defaultValue;
}

FastName KeyedArchive::GetFastName(const String & key, const FastName & defaultValue)
{
    if (IsKeyExists(key))
        return objectMap[key]->AsFastName();
    return defaultValue;
}

const uint8 *KeyedArchive::GetByteArray(const String & key, const uint8 *defaultValue)
{
	if (IsKeyExists(key))
		return objectMap[key]->AsByteArray();
	return defaultValue;
}
int32 KeyedArchive::GetByteArraySize(const String & key, int32 defaultValue)
{
	if (IsKeyExists(key))
		return objectMap[key]->AsByteArraySize();
	return defaultValue;
}
    
KeyedArchive * KeyedArchive::GetArchiveFromByteArray(const String & key)
{
    //DVWARNING(false, "Method is depriceted! Use GetArchive()");
    KeyedArchive * archive = new KeyedArchive;
    int32 size = GetByteArraySize(key);
    if (size == 0)return 0;
    const uint8 * array = GetByteArray(key);
    DynamicMemoryFile * file = DynamicMemoryFile::Create(array, size, File::OPEN | File::READ);
    if (!archive->Load(file))
    {
        SafeRelease(file);
        SafeRelease(archive);
        return 0;
    }
    SafeRelease(file);
    return archive;
}	
    
KeyedArchive * KeyedArchive::GetArchive(const String & key, KeyedArchive * defaultValue)
{
	if (IsKeyExists(key))
		return objectMap[key]->AsKeyedArchive();
	return defaultValue;
}


VariantType * KeyedArchive::GetVariant(const String & key)
{
	if(IsKeyExists(key))
		return objectMap[key];

	return NULL;
}
    
int64 KeyedArchive::GetInt64(const String & key, int64 defaultValue)
{
    if (IsKeyExists(key))
        return objectMap[key]->AsInt64();
    return defaultValue;
}

uint64 KeyedArchive::GetUInt64(const String & key, uint64 defaultValue)
{
    if (IsKeyExists(key))
        return objectMap[key]->AsUInt64();
    return defaultValue;
}

Vector2 KeyedArchive::GetVector2(const String & key, const Vector2 & defaultValue  )
{
    if (IsKeyExists(key))
        return objectMap[key]->AsVector2();
    return defaultValue;
}

Vector3 KeyedArchive::GetVector3(const String & key, const Vector3 & defaultValue )
{
    if (IsKeyExists(key))
        return objectMap[key]->AsVector3();
    return defaultValue;
}

Vector4 KeyedArchive::GetVector4(const String & key, const Vector4 & defaultValue )
{
    if (IsKeyExists(key))
        return objectMap[key]->AsVector4();
    return defaultValue;
}

Matrix2 KeyedArchive::GetMatrix2(const String & key, const Matrix2 & defaultValue )
{
    if (IsKeyExists(key))
        return objectMap[key]->AsMatrix2();
    return defaultValue;
}

Matrix3 KeyedArchive::GetMatrix3(const String & key, const Matrix3 & defaultValue )
{
    if (IsKeyExists(key))
        return objectMap[key]->AsMatrix3();
    return defaultValue;
}

Matrix4 KeyedArchive::GetMatrix4(const String & key, const Matrix4 & defaultValue )
{
    if (IsKeyExists(key))
        return objectMap[key]->AsMatrix4();
    return defaultValue;
}

Color KeyedArchive::GetColor(const String & key, const Color& defaultValue)
{
    if (IsKeyExists(key))
        return objectMap[key]->AsColor();
    return defaultValue;
}
    
void KeyedArchive::DeleteKey(const String & key)
{
	Map<String, VariantType*>::iterator t = objectMap.find(key);
	if (t != objectMap.end())
    {
        delete t->second;
        objectMap.erase(key);
    }
}

void KeyedArchive::DeleteAllKeys()
{
    for (Map<String, VariantType*>::iterator it = objectMap.begin(); it != objectMap.end(); it++)
    {
        delete it->second;
    }
	objectMap.clear();
}

uint32 KeyedArchive::Count(const String &key)
{
	if(key.empty())
	{
		return static_cast<uint32>(objectMap.size());
	}
	else
	{
		return static_cast<uint32>(objectMap.count(key));
	}
}

	
void KeyedArchive::Dump()
{
	Logger::FrameworkDebug("============================================================");
	Logger::FrameworkDebug("--------------- Archive Currently contain ----------------");
	for(Map<String, VariantType*>::iterator it = objectMap.begin(); it != objectMap.end(); ++it)
	{
		switch(it->second->GetType())
		{
			case VariantType::TYPE_BOOLEAN:
			{
				if(it->second->boolValue)
				{
					Logger::FrameworkDebug("%s : true", it->first.c_str());
				}
				else 
				{
					Logger::FrameworkDebug("%s : false", it->first.c_str());
				}

			}
				break;
			case VariantType::TYPE_INT32:
			{
				Logger::FrameworkDebug("%s : %d", it->first.c_str(), it->second->int32Value);
			}
				break;	
			case VariantType::TYPE_UINT32:
			{
				Logger::FrameworkDebug("%s : %d", it->first.c_str(), it->second->uint32Value);
			}
				break;	
			case VariantType::TYPE_FLOAT:
			{
				Logger::FrameworkDebug("%s : %f", it->first.c_str(), it->second->floatValue);
			}
				break;	
			case VariantType::TYPE_STRING:
			{
				Logger::FrameworkDebug("%s : %s", it->first.c_str(), it->second->stringValue->c_str());
			}
				break;	
			case VariantType::TYPE_WIDE_STRING:
			{
				Logger::FrameworkDebug("%s : %S", it->first.c_str(), it->second->wideStringValue->c_str());
			}
				break;
                
            default:
                break;
		}
	}
	Logger::FrameworkDebug("============================================================");
}


const Map<String, VariantType*> & KeyedArchive::GetArchieveData()
{
    return objectMap;
}
const Map<String, VariantType*> & KeyedArchive::GetArchieveData() const
{
    return objectMap;
}

const char* KeyedArchive::GenKeyFromIndex(uint32 index)
{
	static char tmpKey[32];

	sprintf(tmpKey, "%04u", index);
	return tmpKey;
}

uint32 KeyedArchive::Serialize(uint8 *data, uint32 size) const
{
    ScopedPtr<DynamicMemoryFile> file(DynamicMemoryFile::Create(File::CREATE | File::WRITE));

    Save(file);
    
    auto archieveSize = file->GetSize();
    if(data && size >= archieveSize)
    {
        Memcpy(data, file->GetData(), archieveSize);
    }
    return archieveSize;
}

void KeyedArchive::Deserialize(uint8 *data, uint32 size)
{
    if(nullptr == data || 0 == size)
    {
        return;
    }
    
    ScopedPtr<DynamicMemoryFile> file(DynamicMemoryFile::Create(File::CREATE | File::WRITE));
    auto written = file->Write(data, size);
    DVASSERT(written == size);
    
    Load(file);
}



	
	
};

