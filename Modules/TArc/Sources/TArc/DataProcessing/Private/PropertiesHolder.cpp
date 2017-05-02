#include "TArc/DataProcessing/PropertiesHolder.h"

#include <FileSystem/FilePath.h>
#include <Logger/Logger.h>
#include <Debug/DVAssert.h>
#include <Utils/StringFormat.h>
#include <Base/FastName.h>

#include <QVariant>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QRect>
#include <QDataStream>

namespace DAVA
{
namespace TArc
{
namespace PropertiesHolderDetails
{
const char* stringListDelimiter = ";# ";
struct JSONObject
{
    JSONObject(JSONObject* parent_, const QString& name_)
        : name(name_)
        , parent(parent_)
    {
        if (parent != nullptr)
        {
            jsonObject = parent->jsonObject[name].toObject();

#if defined(__DAVAENGINE_DEBUG__)
            auto iter = std::find_if(parent->children.begin(), parent->children.end(), [this](const JSONObject* child) {
                return child->name == name;
            });
            DVASSERT(iter == parent->children.end());
            parent->children.push_back(this);
#endif //__DAVAENGINE_DEBUG__
        }
    }

#if defined(__DAVAENGINE_DEBUG__)
    ~JSONObject()
    {
        DVASSERT(children.empty());
        if (parent != nullptr)
        {
            parent->children.remove(this);
        }
    }
    List<JSONObject*> children;
#endif //__DAVAENGINE_DEBUG__

    virtual void Sync() = 0;

    QString name;
    QJsonObject jsonObject;
    JSONObject* parent = nullptr;
};
}

struct PropertiesItem::Impl : public PropertiesHolderDetails::JSONObject
{
    Impl(JSONObject* impl_, const String& name_);
    ~Impl();

    template <typename T>
    T Get(const QString& key, const T& defaultValue);

    template <typename T>
    void Set(const QString& key, T value);

    template <typename T, typename std::enable_if<std::is_pointer<T>::value, int>::type = 0>
    T FromValue(const QJsonValue& value, const T& defaultValue);

    template <typename T, typename std::enable_if<!std::is_pointer<T>::value, int>::type = 0>
    T FromValue(const QJsonValue& value, const T& defaultValue);

    template <typename T, typename std::enable_if<std::is_pointer<T>::value, int>::type = 0>
    QJsonValue ToValue(const T& value);

    template <typename T, typename std::enable_if<!std::is_pointer<T>::value, int>::type = 0>
    QJsonValue ToValue(const T& value);

    void Sync() override;
};

struct PropertiesHolder::Impl : public PropertiesHolderDetails::JSONObject
{
    Impl(const String& name_, const FilePath& dirPath);
    ~Impl();

    void SetDirectory(const FilePath& dirPath);

    void LoadFromFile();
    void SaveToFile();

    void Sync() override;

    QFileInfo storagePath;
};

PropertiesItem PropertiesHolder::CreateSubHolder(const String& holderName) const
{
    PropertiesItem ph;
    ph.impl.reset(new PropertiesItem::Impl(impl.get(), holderName));
    return ph;
}

PropertiesHolder::Impl::Impl(const String& name_, const FilePath& dirPath)
    : PropertiesHolderDetails::JSONObject(nullptr, QString::fromStdString(name_))
{
    SetDirectory(dirPath);
}

PropertiesItem::Impl::Impl(JSONObject* parent, const String& name_)
    : PropertiesHolderDetails::JSONObject(parent, QString::fromStdString(name_))
{
}

PropertiesItem::Impl::~Impl()
{
    DVASSERT(parent != nullptr);
}

PropertiesHolder::Impl::~Impl()
{
    SaveToFile();
}

template <typename T>
T PropertiesItem::Impl::Get(const QString& key, const T& defaultValue)
{
    auto iter = jsonObject.find(key);
    if (iter == jsonObject.end())
    {
        return defaultValue;
    }
    else
    {
        return FromValue(*iter, defaultValue);
    }
}

template <typename T>
void PropertiesItem::Impl::Set(const QString& key, T value)
{
    jsonObject[key] = ToValue(value);
    Sync();
}

template <typename T, typename std::enable_if<std::is_pointer<T>::value, int>::type>
T PropertiesItem::Impl::FromValue(const QJsonValue& value, const T& defaultValue)
{
    DVASSERT(false, "unsupported type: pointer");
}

template <typename T, typename std::enable_if<!std::is_pointer<T>::value, int>::type>
T PropertiesItem::Impl::FromValue(const QJsonValue& value, const T& defaultValue)
{
    DVASSERT(false, "conversion between QJsonValue and T is not declared");
}

template <typename T, typename std::enable_if<std::is_pointer<T>::value, int>::type>
QJsonValue ToValue(const T& value)
{
    DVASSERT(false, "unsupported type: pointer");
    return QJsonValue();
}

template <typename T, typename std::enable_if<!std::is_pointer<T>::value, int>::type>
QJsonValue ToValue(const T& value)
{
    DVASSERT(false, "conversion between QJsonValue and T is not declared");
    return QJsonValue();
}

void PropertiesHolder::Impl::SetDirectory(const FilePath& dirPath)
{
    DVASSERT(!dirPath.IsEmpty());
    QString dirPathStr = QString::fromStdString(dirPath.GetAbsolutePathname());
    QFileInfo fileInfo(dirPathStr);
    if (!fileInfo.isDir())
    {
        DVASSERT(false, "Given filePath must be a directory");
        return;
    }
    QString filePathStr = dirPathStr + name;
    storagePath = QFileInfo(filePathStr);
    LoadFromFile();
}

void PropertiesHolder::Impl::LoadFromFile()
{
    jsonObject = {};
    if (!storagePath.exists())
    {
        return;
    }
    QString filePath = storagePath.absoluteFilePath();
    QFile file(filePath);
    if (file.exists() == false)
    {
        return;
    }

    if (!file.open(QFile::ReadOnly))
    {
        Logger::Error("Can not open file %s for read", filePath.toUtf8().data());
        return;
    }
    QByteArray fileContent = file.readAll();
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(fileContent, &error);
    if (error.error != QJsonParseError::NoError)
    {
        Logger::Warning("JSON file corrupted: error %s", error.errorString().toUtf8().data());
        return;
    }
    if (document.isObject())
    {
        jsonObject = document.object();
    }
    else
    {
        DVASSERT(false, "Unsupported format of JSON file");
    }
}

void PropertiesHolder::Impl::SaveToFile()
{
    QString filePath = storagePath.absoluteFilePath();
    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
    {
        Logger::Error("Can not open file %s for write", filePath.toUtf8().data());
        return;
    }
    QJsonDocument document(jsonObject);
    QByteArray json = document.toJson(QJsonDocument::Indented);
    if (file.write(json) != json.size())
    {
        Logger::Error("File %s can not be written!", filePath.toUtf8().data());
        return;
    }
}

void PropertiesHolder::Impl::Sync()
{
    SaveToFile();
}

void PropertiesItem::Impl::Sync()
{
    DVASSERT(parent != nullptr);
    parent->jsonObject[name] = jsonObject;
    parent->Sync();
}

PropertiesHolder::PropertiesHolder(const String& projectName, const FilePath& directory)
    : impl(new Impl(projectName, directory))
{
}

PropertiesHolder::~PropertiesHolder() = default;

PropertiesItem::PropertiesItem() = default;
PropertiesItem::~PropertiesItem() = default;

PropertiesItem::PropertiesItem(PropertiesItem&& holder)
    : impl(std::move(holder.impl))
{
}

PropertiesItem& PropertiesItem::operator=(PropertiesItem&& holder)
{
    if (this != &holder)
    {
        impl = std::move(holder.impl);
    }
    return *this;
}

PropertiesItem PropertiesItem::CreateSubHolder(const String& holderName) const
{
    return PropertiesItem(*this, holderName);
}

PropertiesItem::PropertiesItem(const PropertiesItem& parent, const String& name)
    : impl(new Impl(parent.impl.get(), name))
{
}

#define ENUM_CAP \
        else \
        { \
            DVASSERT(false, "type is not enumerated in the ENUM_TYPES define!");\
        }

#define SAVE_IF_ACCEPTABLE(value, type, T, key) \
    if (type == Type::Instance<T>()) \
    { \
        DVASSERT(nullptr != impl); \
        try \
        { \
            impl->Set(key, value.Get<T>()); \
        } \
        catch (const Exception& exception) \
        { \
            Logger::Debug("PropertiesHolder::Save: can not get type %s with message %s", type->GetName(), exception.what()); \
        } \
        return; \
    }

#define LOAD_IF_ACCEPTABLE(value, type, T, key) \
    if (type == Type::Instance<T>()) \
    { \
        DVASSERT(nullptr != impl); \
        Any retVal; \
        try \
        { \
            retVal = Any(impl->Get(key, value.Get<T>())); \
        } \
        catch (const Exception& exception) \
        { \
            Logger::Debug("PropertiesHolder::Load: can not get type %s with message %s", type->GetName(), exception.what()); \
        } \
        return retVal; \
    }

#define ENUM_TYPES(METHOD, value, type, key) \
    METHOD(value, type, bool, key) \
    METHOD(value, type, int32, key) \
    METHOD(value, type, uint32, key) \
    METHOD(value, type, int64, key) \
    METHOD(value, type, uint64, key) \
    METHOD(value, type, float32, key) \
    METHOD(value, type, float64, key) \
    METHOD(value, type, QString, key) \
    METHOD(value, type, QRect, key) \
    METHOD(value, type, QByteArray, key) \
    METHOD(value, type, FilePath, key) \
    METHOD(value, type, String, key) \
    METHOD(value, type, Vector<String>, key) \
    METHOD(value, type, Vector<FastName>, key) \
    ENUM_CAP

void PropertiesItem::Set(const String& key, const Any& value)
{
    const Type* type = value.GetType();
    QString keyStr = QString::fromStdString(key);
    DVASSERT(!impl->jsonObject[keyStr].isObject());
    ENUM_TYPES(SAVE_IF_ACCEPTABLE, value, type, keyStr);
}

void PropertiesHolder::SaveToFile()
{
    static_cast<Impl*>(impl.get())->SaveToFile();
}

Any PropertiesItem::Get(const String& key, const Any& defaultValue, const Type* type) const
{
    DVASSERT(type != nullptr);
    DVASSERT(!defaultValue.IsEmpty());
    QString keyStr = QString::fromStdString(key);
    ENUM_TYPES(LOAD_IF_ACCEPTABLE, defaultValue, type, keyStr);
    return Any();
}
} // namespace TArc
} // namespace DAVA

#include "TArc/DataProcessing/Private/PropertiesTypesConvertion.h"
