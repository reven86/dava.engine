#include "FileSystem/KeyedArchive.h"
#include "FileSystem/File.h"
#include "FileSystem/DynamicMemoryFile.h"
#include "FileSystem/UnmanagedMemoryFile.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/VariantType.h"
#include "FileSystem/YamlEmitter.h"
#include "Reflection/ReflectionRegistrator.h"

#include "Logger/Logger.h"

namespace DAVA
{
const Type* GetTypeByVariantType(VariantType* value)
{
    if (value == nullptr)
    {
        return nullptr;
    };

    const Type* result = nullptr;
    switch (value->GetType())
    {
    case VariantType::TYPE_BOOLEAN:
        result = Type::Instance<bool>();
        break;
    case VariantType::TYPE_FLOAT:
        result = Type::Instance<float32>();
        break;
    case VariantType::TYPE_STRING:
        result = Type::Instance<String>();
        break;
    case VariantType::TYPE_WIDE_STRING:
        result = Type::Instance<WideString>();
        break;
    case VariantType::TYPE_BYTE_ARRAY:
        result = nullptr;
        break;
    case VariantType::TYPE_KEYED_ARCHIVE:
        result = Type::Instance<KeyedArchive*>();
        break;
    case VariantType::TYPE_INT64:
        result = Type::Instance<int64>();
        break;
    case VariantType::TYPE_UINT64:
        result = Type::Instance<uint64>();
        break;
    case VariantType::TYPE_VECTOR2:
        result = Type::Instance<Vector2>();
        break;
    case VariantType::TYPE_VECTOR3:
        result = Type::Instance<Vector3>();
        break;
    case VariantType::TYPE_VECTOR4:
        result = Type::Instance<Vector4>();
        break;
    case VariantType::TYPE_MATRIX2:
        result = Type::Instance<Matrix2>();
        break;
    case VariantType::TYPE_MATRIX3:
        result = Type::Instance<Matrix3>();
        break;
    case VariantType::TYPE_MATRIX4:
        result = Type::Instance<Matrix4>();
        break;
    case VariantType::TYPE_COLOR:
        result = Type::Instance<Color>();
        break;
    case VariantType::TYPE_FASTNAME:
        result = Type::Instance<FastName>();
        break;
    case VariantType::TYPE_AABBOX3:
        result = Type::Instance<AABBox3>();
        break;
    case VariantType::TYPE_FILEPATH:
        result = Type::Instance<FilePath>();
        break;
    case VariantType::TYPE_FLOAT64:
        result = Type::Instance<float64>();
        break;
    case VariantType::TYPE_INT8:
    case VariantType::TYPE_INT16:
    case VariantType::TYPE_INT32:
        result = Type::Instance<int32>();
        break;
    case VariantType::TYPE_UINT8:
    case VariantType::TYPE_UINT16:
    case VariantType::TYPE_UINT32:
        result = Type::Instance<uint32>();
        break;
    default:
        break;
    }

    return result;
}

class KeyedArchiveElementValueWrapper : public ValueWrapper
{
public:
    const Type* GetType(const ReflectedObject& object) const override
    {
        VariantType* value = object.GetPtr<VariantType>();
        return GetTypeByVariantType(value);
    }

    bool IsReadonly(const ReflectedObject& object) const override
    {
        return object.IsConst();
    }

    Any GetValue(const ReflectedObject& object) const override
    {
        VariantType* value = object.GetPtr<VariantType>();
        if (value == nullptr)
        {
            return Any();
        }

        Any result;
        switch (value->GetType())
        {
        case VariantType::TYPE_BOOLEAN:
            result = value->AsBool();
            break;
        case VariantType::TYPE_INT32:
            result = value->AsInt32();
            break;
        case VariantType::TYPE_FLOAT:
            result = value->AsFloat();
            break;
        case VariantType::TYPE_STRING:
            result = value->AsString();
            break;
        case VariantType::TYPE_WIDE_STRING:
            result = value->AsWideString();
            break;
        case VariantType::TYPE_BYTE_ARRAY:
            result = Any();
            break;
        case VariantType::TYPE_UINT32:
            result = value->AsUInt32();
            break;
        case VariantType::TYPE_KEYED_ARCHIVE:
            result = value->AsKeyedArchive();
            break;
        case VariantType::TYPE_INT64:
            result = value->AsInt64();
            break;
        case VariantType::TYPE_UINT64:
            result = value->AsUInt64();
            break;
        case VariantType::TYPE_VECTOR2:
            result = value->AsVector2();
            break;
        case VariantType::TYPE_VECTOR3:
            result = value->AsVector3();
            break;
        case VariantType::TYPE_VECTOR4:
            result = value->AsVector4();
            break;
        case VariantType::TYPE_MATRIX2:
            result = value->AsMatrix2();
            break;
        case VariantType::TYPE_MATRIX3:
            result = value->AsMatrix3();
            break;
        case VariantType::TYPE_MATRIX4:
            result = value->AsMatrix4();
            break;
        case VariantType::TYPE_COLOR:
            result = value->AsColor();
            break;
        case VariantType::TYPE_FASTNAME:
            result = value->AsFastName();
            break;
        case VariantType::TYPE_AABBOX3:
            result = value->AsAABBox3();
            break;
        case VariantType::TYPE_FILEPATH:
            result = value->AsFilePath();
            break;
        case VariantType::TYPE_FLOAT64:
            result = value->AsFloat64();
            break;
        case VariantType::TYPE_INT8:
            result = static_cast<int32>(value->AsInt8());
            break;
        case VariantType::TYPE_UINT8:
            result = static_cast<uint32>(value->AsUInt8());
            break;
        case VariantType::TYPE_INT16:
            result = static_cast<int32>(value->AsInt16());
            break;
        case VariantType::TYPE_UINT16:
            result = static_cast<uint32>(value->AsUInt16());
            break;
        default:
            break;
        }

        return result;
    }

    bool SetValue(const ReflectedObject& object, const Any& value) const override
    {
        VariantType* v = object.GetPtr<VariantType>();
        const Type* srcType = GetTypeByVariantType(v);
        if (srcType != value.GetType())
        {
            return false;
        }

        switch (v->GetType())
        {
        case VariantType::TYPE_BOOLEAN:
            v->SetBool(value.Get<bool>());
            break;
        case VariantType::TYPE_INT32:
            v->SetInt32(value.Get<int32>());
            break;
        case VariantType::TYPE_FLOAT:
            v->SetFloat(value.Get<float32>());
            break;
        case VariantType::TYPE_STRING:
            v->SetString(value.Get<String>());
            break;
        case VariantType::TYPE_WIDE_STRING:
            v->SetWideString(value.Get<WideString>());
            break;
        case VariantType::TYPE_UINT32:
            v->SetUInt32(value.Get<uint32>());
            break;
        case VariantType::TYPE_INT64:
            v->SetInt64(value.Get<int64>());
            break;
        case VariantType::TYPE_UINT64:
            v->SetUInt64(value.Get<uint64>());
            break;
        case VariantType::TYPE_VECTOR2:
            v->SetVector2(value.Get<Vector2>());
            break;
        case VariantType::TYPE_VECTOR3:
            v->SetVector3(value.Get<Vector3>());
            break;
        case VariantType::TYPE_VECTOR4:
            v->SetVector4(value.Get<Vector4>());
            break;
        case VariantType::TYPE_MATRIX2:
            v->SetMatrix2(value.Get<Matrix2>());
            break;
        case VariantType::TYPE_MATRIX3:
            v->SetMatrix3(value.Get<Matrix3>());
            break;
        case VariantType::TYPE_MATRIX4:
            v->SetMatrix4(value.Get<Matrix4>());
            break;
        case VariantType::TYPE_COLOR:
            v->SetColor(value.Get<Color>());
            break;
        case VariantType::TYPE_FASTNAME:
            v->SetFastName(value.Get<FastName>());
            break;
        case VariantType::TYPE_FILEPATH:
            v->SetFilePath(value.Get<FilePath>());
            break;
        case VariantType::TYPE_FLOAT64:
            v->SetFloat64(value.Get<float64>());
            break;
        case VariantType::TYPE_INT8:
            v->SetInt32(value.Get<int8>());
            break;
        case VariantType::TYPE_UINT8:
            v->SetUInt32(value.Get<uint8>());
            break;
        case VariantType::TYPE_INT16:
            v->SetInt32(value.Get<int16>());
            break;
        case VariantType::TYPE_UINT16:
            v->SetUInt32(value.Get<uint16>());
            break;
        case VariantType::TYPE_AABBOX3:
            v->SetAABBox3(value.Get<AABBox3>());
            break;
        case VariantType::TYPE_BYTE_ARRAY:
        case VariantType::TYPE_KEYED_ARCHIVE:
        default:
            DVASSERT(false);
            return false;
        }

        return true;
    }

    bool SetValueWithCast(const ReflectedObject& object, const Any& value) const override
    {
        VariantType* v = object.GetPtr<VariantType>();

        VariantType result = PrepareValueForKeyedArchive(value, v->GetType());
        if (result.GetType() != VariantType::TYPE_NONE)
        {
            DVASSERT(v->GetType() == result.GetType());
            *v = result;
            return true;
        }

        return false;
    }

    ReflectedObject GetValueObject(const ReflectedObject& object) const override
    {
        VariantType* value = object.GetPtr<VariantType>();
        ReflectedObject result = object;
        switch (value->GetType())
        {
        case VariantType::TYPE_KEYED_ARCHIVE:
            result = ReflectedObject(value->AsKeyedArchive());
            break;
        case VariantType::TYPE_VECTOR2:
            result = ReflectedObject(value->vector2Value);
            break;
        case VariantType::TYPE_VECTOR3:
            result = ReflectedObject(value->vector3Value);
            break;
        case VariantType::TYPE_VECTOR4:
            result = ReflectedObject(value->vector4Value);
            break;
        case VariantType::TYPE_MATRIX2:
            result = ReflectedObject(value->matrix2Value);
            break;
        case VariantType::TYPE_MATRIX3:
            result = ReflectedObject(value->matrix3Value);
            break;
        case VariantType::TYPE_MATRIX4:
            result = ReflectedObject(value->matrix4Value);
            break;
        case VariantType::TYPE_COLOR:
            result = ReflectedObject(value->colorValue);
            break;
        case VariantType::TYPE_AABBOX3:
            result = ReflectedObject(value->aabbox3);
            break;
        default:
            break;
        }

        return result;
    }
};

class KeyedArchiveStructureWrapper : public StructureWrapper
{
public:
    KeyedArchiveStructureWrapper()
    {
        caps.canAddField = true;
        caps.canInsertField = true;
        caps.canRemoveField = true;
        caps.hasDynamicStruct = true;
    }

    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        KeyedArchive* archive = vw->GetValueObject(object).GetPtr<KeyedArchive>();
        return archive->objectMap.empty() == false;
    }

    Reflection CreateReflection(VariantType* v, bool isArchiveConst) const
    {
        ReflectedObject elementObj = ReflectedObject(v);
        if (isArchiveConst)
        {
            elementObj = ReflectedObject(const_cast<const VariantType*>(v));
        }

        const StructureWrapper* structureWrapper = nullptr;
        if (v->GetType() == VariantType::TYPE_KEYED_ARCHIVE)
        {
            structureWrapper = this;
        }

        return Reflection(elementObj, &valueWrapper, structureWrapper, nullptr);
    }

    Reflection GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override
    {
        if (key.CanCast<String>())
        {
            String stringKey = key.Cast<String>();

            ReflectedObject archiveObject = vw->GetValueObject(object);
            KeyedArchive* archive = archiveObject.GetPtr<KeyedArchive>();
            if (archive->objectMap.count(stringKey) > 0)
            {
                VariantType* v = archive->objectMap[stringKey];
                return CreateReflection(v, archiveObject.IsConst());
            }
        }

        return Reflection();
    }

    Vector<Reflection::Field> GetFields(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        Vector<Reflection::Field> fields;

        ReflectedObject archiveObject = vw->GetValueObject(object);
        KeyedArchive* archive = archiveObject.GetPtr<KeyedArchive>();
        fields.reserve(archive->objectMap.size());

        bool isConst = archiveObject.IsConst();

        for (auto& node : archive->objectMap)
        {
            fields.emplace_back();
            Reflection::Field& f = fields.back();
            f.key = node.first;
            f.ref = CreateReflection(node.second, isConst);
        }

        return fields;
    }

    const Reflection::FieldCaps& GetFieldsCaps(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return caps;
    }

    bool HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return false;
    }

    AnyFn GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override
    {
        return AnyFn();
    }

    Vector<Reflection::Method> GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return Vector<Reflection::Method>();
    }

    AnyFn GetFieldCreator(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return AnyFn();
    }

    bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const override
    {
        if (key.CanCast<String>())
        {
            String k = key.Cast<String>();
            KeyedArchive* archive = vw->GetValueObject(object).GetPtr<KeyedArchive>();
            const Type* t = value.GetType();
            if (t == Type::Instance<int8>())
                archive->SetInt32(k, value.Get<int8>());
            else if (t == Type::Instance<uint8>())
                archive->SetUInt32(k, value.Get<uint8>());
            else if (t == Type::Instance<int16>())
                archive->SetInt32(k, value.Get<int16>());
            else if (t == Type::Instance<uint16>())
                archive->SetUInt32(k, value.Get<uint16>());
            else if (t == Type::Instance<int32>())
                archive->SetInt32(k, value.Get<int32>());
            else if (t == Type::Instance<uint32>())
                archive->SetUInt32(k, value.Get<uint32>());
            else if (t == Type::Instance<int64>())
                archive->SetInt64(k, value.Get<int64>());
            else if (t == Type::Instance<uint64>())
                archive->SetUInt64(k, value.Get<uint64>());
            else if (t == Type::Instance<float32>())
                archive->SetFloat(k, value.Get<float32>());
            else if (t == Type::Instance<float64>())
                archive->SetFloat64(k, value.Get<float64>());
            else if (t == Type::Instance<String>())
                archive->SetString(k, value.Get<String>());
            else if (t == Type::Instance<WideString>())
                archive->SetWideString(k, value.Get<WideString>());
            else if (t == Type::Instance<Vector2>())
                archive->SetVector2(k, value.Get<Vector2>());
            else if (t == Type::Instance<Vector3>())
                archive->SetVector3(k, value.Get<Vector3>());
            else if (t == Type::Instance<Vector4>())
                archive->SetVector4(k, value.Get<Vector4>());
            else if (t == Type::Instance<Matrix2>())
                archive->SetMatrix2(k, value.Get<Matrix2>());
            else if (t == Type::Instance<Matrix3>())
                archive->SetMatrix3(k, value.Get<Matrix3>());
            else if (t == Type::Instance<Matrix4>())
                archive->SetMatrix4(k, value.Get<Matrix4>());
            else if (t == Type::Instance<Color>())
                archive->SetColor(k, value.Get<Color>());
            else if (t == Type::Instance<bool>())
                archive->SetBool(k, value.Get<bool>());

            return true;
        }

        return false;
    }

    bool InsertField(const ReflectedObject& object, const ValueWrapper* vw, const Any& beforeKey, const Any& key, const Any& value) const override
    {
        return AddField(object, vw, key, value);
    }

    bool RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override
    {
        if (key.CanCast<String>())
        {
            KeyedArchive* archive = vw->GetValueObject(object).GetPtr<KeyedArchive>();
            archive->DeleteKey(key.Cast<String>());
            return true;
        }

        return false;
    }

private:
    KeyedArchiveElementValueWrapper valueWrapper;
    Reflection::FieldCaps caps;
};

VariantType PrepareValueForKeyedArchive(const Any& value, VariantType::eVariantType resultType)
{
    VariantType result;
    switch (resultType)
    {
    case VariantType::TYPE_BOOLEAN:
        if (value.CanCast<bool>())
        {
            result.SetBool(value.Cast<bool>());
        }
        break;
    case VariantType::TYPE_INT32:
        if (value.CanCast<int32>())
        {
            result.SetInt32(value.Cast<int32>());
        }
        break;
    case VariantType::TYPE_FLOAT:
        if (value.CanCast<float32>())
        {
            result.SetFloat(value.Cast<float32>());
        }
        break;
    case VariantType::TYPE_STRING:
        if (value.CanCast<String>())
        {
            result.SetString(value.Cast<String>());
        }
        break;
    case VariantType::TYPE_WIDE_STRING:
        if (value.CanCast<WideString>())
        {
            result.SetWideString(value.Cast<WideString>());
        }
        break;
    case VariantType::TYPE_UINT32:
        if (value.CanCast<uint32>())
        {
            result.SetUInt32(value.Cast<uint32>());
        }
        break;
    case VariantType::TYPE_INT64:
        if (value.CanCast<int64>())
        {
            result.SetInt64(value.Cast<int64>());
        }
        break;
    case VariantType::TYPE_UINT64:
        if (value.CanCast<uint64>())
        {
            result.SetUInt64(value.Cast<uint64>());
        }
        break;
    case VariantType::TYPE_VECTOR2:
        if (value.CanCast<Vector2>())
        {
            result.SetVector2(value.Cast<Vector2>());
        }
        break;
    case VariantType::TYPE_VECTOR3:
        if (value.CanCast<Vector3>())
        {
            result.SetVector3(value.Cast<Vector3>());
        }
        break;
    case VariantType::TYPE_VECTOR4:
        if (value.CanCast<Vector4>())
        {
            result.SetVector4(value.Cast<Vector4>());
        }
        break;
    case VariantType::TYPE_MATRIX2:
        if (value.CanCast<Matrix2>())
        {
            result.SetMatrix2(value.Cast<Matrix2>());
        }
        break;
    case VariantType::TYPE_MATRIX3:
        if (value.CanCast<Matrix3>())
        {
            result.SetMatrix3(value.Cast<Matrix3>());
        }
        break;
    case VariantType::TYPE_MATRIX4:
        if (value.CanCast<Matrix4>())
        {
            result.SetMatrix4(value.Cast<Matrix4>());
        }
        break;
    case VariantType::TYPE_COLOR:
        if (value.CanCast<Color>())
        {
            result.SetColor(value.Cast<Color>());
        }
        break;
    case VariantType::TYPE_FASTNAME:
        if (value.CanCast<FastName>())
        {
            result.SetFastName(value.Cast<FastName>());
        }
        break;
    case VariantType::TYPE_FILEPATH:
        if (value.CanCast<FilePath>())
        {
            result.SetFilePath(value.Cast<FilePath>());
        }
        break;
    case VariantType::TYPE_FLOAT64:
        if (value.CanCast<float64>())
        {
            result.SetFloat64(value.Cast<float64>());
        }
        break;
    case VariantType::TYPE_INT8:
        if (value.CanCast<int8>())
        {
            result.SetInt32(value.Cast<int8>());
        }
        break;
    case VariantType::TYPE_UINT8:
        if (value.CanCast<uint8>())
        {
            result.SetUInt32(value.Cast<uint8>());
        }
        break;
    case VariantType::TYPE_INT16:
        if (value.CanCast<int16>())
        {
            result.SetInt32(value.Cast<int16>());
        }
        break;
    case VariantType::TYPE_UINT16:
        if (value.CanCast<uint16>())
        {
            result.SetUInt32(value.Cast<uint16>());
        }
        break;
    case VariantType::TYPE_AABBOX3:
        if (value.CanCast<AABBox3>())
        {
            result.SetAABBox3(value.Cast<AABBox3>());
        }
        break;
    case VariantType::TYPE_BYTE_ARRAY:
    case VariantType::TYPE_KEYED_ARCHIVE:
    default:
        DVASSERT(false);
        break;
    }

    return result;
}

DAVA_VIRTUAL_REFLECTION_IMPL(KeyedArchive)
{
    ReflectionRegistrator<KeyedArchive>::Begin(std::make_unique<KeyedArchiveStructureWrapper>())
    .End();
}

KeyedArchive::KeyedArchive()
{
}

KeyedArchive::KeyedArchive(const KeyedArchive& arc)
{
    for (const auto& obj : arc.GetArchieveData())
    {
        SetVariant(obj.first, *obj.second);
    }
}

KeyedArchive::~KeyedArchive()
{
    DeleteAllKeys();
}

bool KeyedArchive::Load(const FilePath& pathName)
{
    File* archive = File::Create(pathName, File::OPEN | File::READ);
    if (nullptr == archive)
    {
        return false;
    }
    bool ret = Load(archive);
    SafeRelease(archive);

    return ret;
}

bool KeyedArchive::Load(File* archive)
{
    DAVA::Array<char, 2> header;
    uint32 wasRead = archive->Read(header.data(), 2);
    if (wasRead != 2)
    {
        Logger::Error("[KeyedArchive] error loading keyed archive from file: %s, filesize: %d", archive->GetFilename().GetAbsolutePathname().c_str(), archive->GetSize());
        return false;
    }
    else if ((header[0] != 'K') || (header[1] != 'A'))
    {
        const bool seekResult = archive->Seek(0, File::SEEK_FROM_START);
        if (!seekResult)
        {
            Logger::Error("[KeyedArchive] seek failed from file: %s, filesize: %d", archive->GetFilename().GetAbsolutePathname().c_str(), archive->GetSize());
            return false;
        }
        while (!archive->IsEof())
        {
            VariantType key;

            if (archive->IsEof())
            {
                break;
            }
            if (!key.Read(archive))
            {
                return false;
            }
            VariantType* value = new VariantType();
            if (!value->Read(archive))
            {
                SafeDelete(value);
                return false;
            }
            DeleteKey(key.AsString());
            objectMap[key.AsString()] = value;
        }
        return true;
    }

    uint16 version = 0;
    if (2 != archive->Read(&version, 2))
    {
        return false;
    }
    if (version != 1)
    {
        Logger::Error("[KeyedArchive] error loading keyed archive, because version is incorrect");
        return false;
    }
    uint32 numberOfItems = 0;
    if (4 != archive->Read(&numberOfItems, 4))
    {
        return false;
    }
    for (uint32 item = 0; item < numberOfItems; ++item)
    {
        VariantType key;

        if (archive->IsEof())
        {
            break;
        }
        if (!key.Read(archive))
        {
            return false;
        }
        VariantType* value = new VariantType();
        if (!value->Read(archive))
        {
            SafeDelete(value);
            return false;
        }
        DeleteKey(key.AsString());
        objectMap[key.AsString()] = value;
    }
    return true;
}

bool KeyedArchive::Save(const FilePath& pathName) const
{
    File* archive = File::Create(pathName, File::CREATE | File::WRITE);
    if (nullptr == archive)
    {
        return false;
    }

    bool ret = Save(archive);
    SafeRelease(archive);

    return ret;
}

bool KeyedArchive::Save(File* archive) const
{
    Map<UnderlyingMap::key_type, UnderlyingMap::mapped_type> orderedMap;
    orderedMap.insert(objectMap.begin(), objectMap.end());

    Array<char, 2> header;
    uint16 version = 1;
    uint32 size = static_cast<uint32>(orderedMap.size());

    header[0] = 'K';
    header[1] = 'A';

    if (2 != archive->Write(header.data(), 2)
        || 2 != archive->Write(&version, 2)
        || 4 != archive->Write(&size, 4))
    {
        return false;
    }
    for (const auto& obj : orderedMap)
    {
        VariantType key;
        key.SetString(obj.first);
        if (!key.Write(archive)
            || !obj.second->Write(archive))
        {
            return false;
        }
    }
    return true;
}

uint32 KeyedArchive::Save(uint8* data, uint32 size) const
{
    ScopedPtr<DynamicMemoryFile> buffer(DynamicMemoryFile::Create(File::CREATE | File::WRITE));

    Save(buffer);

    auto archieveSize = buffer->GetSize();
    if ((nullptr != data) && (size >= archieveSize))
    { // if data is null, we just return requested size for data
        Memcpy(data, buffer->GetData(), static_cast<size_t>(archieveSize));
    }
    return static_cast<uint32>(archieveSize);
}

bool KeyedArchive::Load(const uint8* data, uint32 size)
{
    if (nullptr == data || 0 == size)
    {
        return false;
    }

    ScopedPtr<DynamicMemoryFile> buffer(DynamicMemoryFile::Create(File::CREATE | File::WRITE | File::READ));
    auto written = buffer->Write(data, size);
    DVASSERT(written == size);

    buffer->Seek(0, File::SEEK_FROM_START);

    return Load(buffer);
}

bool KeyedArchive::LoadFromYamlFile(const FilePath& pathName)
{
    ScopedPtr<YamlParser> parser(YamlParser::Create(pathName));
    return (parser.get() != nullptr) && LoadFromYamlNode(parser->GetRootNode());
}

bool KeyedArchive::LoadFromYamlNode(const YamlNode* rootNode)
{
    if (nullptr == rootNode)
    {
        return false;
    }

    const YamlNode* archieveNode = rootNode->Get(VariantType::TYPENAME_KEYED_ARCHIVE);
    if (nullptr == archieveNode)
    {
        return false;
    }

    int32 count = archieveNode->GetCount();
    for (int32 i = 0; i < count; ++i)
    {
        const YamlNode* node = archieveNode->Get(i);
        const String& variableNameToArchMap = archieveNode->GetItemKeyName(i);

        VariantType* value = new VariantType(node->AsVariantType());

        if (value->GetType() == VariantType::TYPE_NONE)
        {
            SafeDelete(value);
            continue;
        }

        objectMap[variableNameToArchMap] = value;
    }

    return true;
}

bool KeyedArchive::SaveToYamlFile(const FilePath& pathName) const
{
    ScopedPtr<YamlNode> node(YamlNode::CreateMapNode());
    node->Set(VariantType::TYPENAME_KEYED_ARCHIVE, VariantType(const_cast<KeyedArchive*>(this)));

    return YamlEmitter::SaveToYamlFile(pathName, node);
}

void KeyedArchive::SetBool(const String& key, bool value)
{
    DeleteKey(key);
    VariantType* variantValue = new VariantType();
    variantValue->SetBool(value);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetInt32(const String& key, int32 value)
{
    DeleteKey(key);
    VariantType* variantValue = new VariantType();
    variantValue->SetInt32(value);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetUInt32(const String& key, uint32 value)
{
    DeleteKey(key);
    VariantType* variantValue = new VariantType();
    variantValue->SetUInt32(value);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetFloat(const String& key, float32 value)
{
    DeleteKey(key);
    VariantType* variantValue = new VariantType();
    variantValue->SetFloat(value);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetFloat64(const String& key, float64 value)
{
    DeleteKey(key);
    VariantType* variantValue = new VariantType();
    variantValue->SetFloat64(value);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetString(const String& key, const String& value)
{
    DeleteKey(key);
    VariantType* variantValue = new VariantType();
    variantValue->SetString(value);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetWideString(const String& key, const WideString& value)
{
    DeleteKey(key);
    VariantType* variantValue = new VariantType();
    variantValue->SetWideString(value);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetFastName(const String& key, const FastName& value)
{
    DeleteKey(key);
    VariantType* variantValue = new VariantType();
    variantValue->SetFastName(value);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetByteArray(const String& key, const uint8* value, int32 arraySize)
{
    VariantType* variantValue = new VariantType();
    variantValue->SetByteArray(value, arraySize);

    DeleteKey(key);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetVariant(const String& key, const VariantType& value)
{
    DeleteKey(key);
    VariantType* variantValue = new VariantType(value);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetByteArrayFromArchive(const String& key, KeyedArchive* archive)
{
    //DVWARNING(false, "Method is depriceted! Use SetArchive()");
    DynamicMemoryFile* file = DynamicMemoryFile::Create(File::CREATE | File::WRITE);
    archive->Save(file);
    SetByteArray(key, file->GetData(), static_cast<uint32>(file->GetSize()));
    SafeRelease(file);
}

void KeyedArchive::SetArchive(const String& key, KeyedArchive* archive)
{
    VariantType* variantValue = new VariantType();
    variantValue->SetKeyedArchive(archive);

    DeleteKey(key);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetInt64(const String& key, const int64& value)
{
    DeleteKey(key);
    VariantType* variantValue = new VariantType();
    variantValue->SetInt64(value);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetUInt64(const String& key, const uint64& value)
{
    DeleteKey(key);
    VariantType* variantValue = new VariantType();
    variantValue->SetUInt64(value);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetVector2(const String& key, const Vector2& value)
{
    DeleteKey(key);
    VariantType* variantValue = new VariantType();
    variantValue->SetVector2(value);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetVector3(const String& key, const Vector3& value)
{
    DeleteKey(key);
    VariantType* variantValue = new VariantType();
    variantValue->SetVector3(value);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetVector4(const String& key, const Vector4& value)
{
    DeleteKey(key);
    VariantType* variantValue = new VariantType();
    variantValue->SetVector4(value);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetMatrix2(const String& key, const Matrix2& value)
{
    DeleteKey(key);
    VariantType* variantValue = new VariantType();
    variantValue->SetMatrix2(value);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetMatrix3(const String& key, const Matrix3& value)
{
    DeleteKey(key);
    VariantType* variantValue = new VariantType();
    variantValue->SetMatrix3(value);
    objectMap[key] = variantValue;
}

void KeyedArchive::SetMatrix4(const String& key, const Matrix4& value)
{
    DeleteKey(key);
    VariantType* variantValue = new VariantType();
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

bool KeyedArchive::IsKeyExists(const String& key) const
{
    auto it = objectMap.find(key);
    if (it != objectMap.end())
    {
        return true;
    }
    return false;
}

bool KeyedArchive::GetBool(const String& key, bool defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsBool() : defaultValue;
}

int32 KeyedArchive::GetInt32(const String& key, int32 defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsInt32() : defaultValue;
}

uint32 KeyedArchive::GetUInt32(const String& key, uint32 defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsUInt32() : defaultValue;
}

float32 KeyedArchive::GetFloat(const String& key, float32 defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsFloat() : defaultValue;
}

float64 KeyedArchive::GetFloat64(const String& key, float64 defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsFloat64() : defaultValue;
}

String KeyedArchive::GetString(const String& key, const String& defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsString() : defaultValue;
}

WideString KeyedArchive::GetWideString(const String& key, const WideString& defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsWideString() : defaultValue;
}

FastName KeyedArchive::GetFastName(const String& key, const FastName& defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsFastName() : defaultValue;
}

const uint8* KeyedArchive::GetByteArray(const String& key, const uint8* defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsByteArray() : defaultValue;
}
int32 KeyedArchive::GetByteArraySize(const String& key, int32 defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsByteArraySize() : defaultValue;
}

KeyedArchive* KeyedArchive::GetArchiveFromByteArray(const String& key) const
{
    //DVWARNING(false, "Method is depriceted! Use GetArchive()");
    KeyedArchive* archive = new KeyedArchive;
    int32 size = GetByteArraySize(key);
    if (size == 0)
    {
        return nullptr;
    }
    ScopedPtr<UnmanagedMemoryFile> file(new UnmanagedMemoryFile(GetByteArray(key), size));
    if (!archive->Load(file))
    {
        SafeRelease(archive);
        return nullptr;
    }
    return archive;
}

KeyedArchive* KeyedArchive::GetArchive(const String& key, KeyedArchive* defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsKeyedArchive() : defaultValue;
}

VariantType* KeyedArchive::GetVariant(const String& key) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second : nullptr;
}

int64 KeyedArchive::GetInt64(const String& key, int64 defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsInt64() : defaultValue;
}

uint64 KeyedArchive::GetUInt64(const String& key, uint64 defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsUInt64() : defaultValue;
}

Vector2 KeyedArchive::GetVector2(const String& key, const Vector2& defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsVector2() : defaultValue;
}

Vector3 KeyedArchive::GetVector3(const String& key, const Vector3& defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsVector3() : defaultValue;
}

Vector4 KeyedArchive::GetVector4(const String& key, const Vector4& defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsVector4() : defaultValue;
}

Matrix2 KeyedArchive::GetMatrix2(const String& key, const Matrix2& defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsMatrix2() : defaultValue;
}

Matrix3 KeyedArchive::GetMatrix3(const String& key, const Matrix3& defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsMatrix3() : defaultValue;
}

Matrix4 KeyedArchive::GetMatrix4(const String& key, const Matrix4& defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsMatrix4() : defaultValue;
}

Color KeyedArchive::GetColor(const String& key, const Color& defaultValue) const
{
    auto it = objectMap.find(key);
    return it != objectMap.end() ? it->second->AsColor() : defaultValue;
}

void KeyedArchive::DeleteKey(const String& key)
{
    auto it = objectMap.find(key);
    if (it != objectMap.end())
    {
        delete it->second;
        objectMap.erase(key);
    }
}

void KeyedArchive::DeleteAllKeys()
{
    for (const auto& obj : objectMap)
    {
        delete obj.second;
    }
    objectMap.clear();
}

uint32 KeyedArchive::Count(const String& key) const
{
    if (key.empty())
    {
        return static_cast<uint32>(objectMap.size());
    }
    else
    {
        return static_cast<uint32>(objectMap.count(key));
    }
}

void KeyedArchive::Dump() const
{
    Logger::FrameworkDebug("============================================================");
    Logger::FrameworkDebug("--------------- Archive Currently contain ----------------");
    for (const auto& obj : objectMap)
    {
        switch (obj.second->GetType())
        {
        case VariantType::TYPE_BOOLEAN:
        {
            if (obj.second->boolValue)
            {
                Logger::FrameworkDebug("%s : true", obj.first.c_str());
            }
            else
            {
                Logger::FrameworkDebug("%s : false", obj.first.c_str());
            }
        }
        break;
        case VariantType::TYPE_INT8:
        {
            Logger::FrameworkDebug("%s : %hhd", obj.first.c_str(), obj.second->int8Value);
        }
        break;
        case VariantType::TYPE_UINT8:
        {
            Logger::FrameworkDebug("%s : %hu", obj.first.c_str(), obj.second->uint8Value);
        }
        break;
        case VariantType::TYPE_INT16:
        {
            Logger::FrameworkDebug("%s : %hd", obj.first.c_str(), obj.second->int16Value);
        }
        break;
        case VariantType::TYPE_UINT16:
        {
            Logger::FrameworkDebug("%s : %hu", obj.first.c_str(), obj.second->uint16Value);
        }
        break;
        case VariantType::TYPE_INT32:
        {
            Logger::FrameworkDebug("%s : %d", obj.first.c_str(), obj.second->int32Value);
        }
        break;
        case VariantType::TYPE_UINT32:
        {
            Logger::FrameworkDebug("%s : %u", obj.first.c_str(), obj.second->uint32Value);
        }
        break;
        case VariantType::TYPE_FLOAT:
        {
            Logger::FrameworkDebug("%s : %f", obj.first.c_str(), obj.second->floatValue);
        }
        break;
        case VariantType::TYPE_STRING:
        {
            Logger::FrameworkDebug("%s : %s", obj.first.c_str(), obj.second->stringValue->c_str());
        }
        break;
        case VariantType::TYPE_WIDE_STRING:
        {
            Logger::FrameworkDebug("%s : %S", obj.first.c_str(), obj.second->wideStringValue->c_str());
        }
        break;

        default:
            break;
        }
    }
    Logger::FrameworkDebug("============================================================");
}

const KeyedArchive::UnderlyingMap& KeyedArchive::GetArchieveData() const
{
    return objectMap;
}

const char* KeyedArchive::GenKeyFromIndex(uint32 index)
{
    static char tmpKey[32];

    sprintf(tmpKey, "%04u", index);
    return tmpKey;
}
};
