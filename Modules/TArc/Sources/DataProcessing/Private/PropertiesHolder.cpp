#include "DataProcessing/PropertiesHolder.h"
#include "FileSystem/FilePath.h"
#include "Debug/DVAssert.h"

#include <QVariant>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJSonValue>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <QFile>

namespace DAVA
{
namespace TArc
{
namespace PropertiesHolderDetails
{
struct JSONObject
{
    JSONObject(const QString& name_)
        : name(name_)
    {
    }

    ~JSONObject()
    {
        DVASSERT(children.empty());
    }

    QString name;
    List<JSONObject*> children;
    QJsonObject jsonObject;
};
}

struct PropertiesItem::Impl : public PropertiesHolderDetails::JSONObject
{
    Impl(JSONObject* impl_, const String& name_);
    ~Impl();

    //realization for the fundamental types
    template <typename T, typename std::enable_if<std::is_fundamental<T>::value, int>::type = 0>
    void Set(const QString& key, T value);

    //realization for the pointers
    template <typename T, typename std::enable_if<std::is_pointer<T>::value, int>::type = 0>
    void Set(const QString& key, const T& value);

    //realization for the pointers
    template <typename T, typename std::enable_if<!std::is_pointer<T>::value && !std::is_fundamental<T>::value, int>::type = 0>
    void Set(const QString& key, const T& value);

    QJsonValue ToValue(const QString& value);
    QJsonValue ToValue(const QByteArray& value);

    template <typename T>
    T Get(const QString& key, const T& defaultValue);

    template <typename T, typename std::enable_if<std::is_fundamental<T>::value, int>::type = 0>
    T FromValue(const QJsonValue& value, const T& defaultValue);

    template <typename T, typename std::enable_if<std::is_pointer<T>::value, int>::type = 0>
    T FromValue(const QJsonValue& value, const T& defaultValue);

    QString FromValue(const QJsonValue& value, const QString& defaultValue);
    QByteArray FromValue(const QJsonValue& value, const QByteArray& defaultValue);

    void SaveToParent();

    JSONObject* parent = nullptr;

    bool wasChanged = false;
};

struct PropertiesHolder::Impl : public PropertiesHolderDetails::JSONObject
{
    Impl(const String& name_, const FilePath& dirPath);
    ~Impl();

    void SetDirectory(const FilePath& dirPath);

    void LoadFromFile();
    void SaveToFile();

    QFileInfo storagePath;
};

PropertiesItem PropertiesHolder::CreateSubHolder(const String& holderName) const
{
    PropertiesItem ph;
    ph.impl.reset(new PropertiesItem::Impl(impl.get(), holderName));
    return ph;
}

PropertiesHolder::Impl::Impl(const String& name_, const FilePath& dirPath)
    : PropertiesHolderDetails::JSONObject(QString::fromStdString(name_))
{
    SetDirectory(dirPath);
}

PropertiesItem::Impl::Impl(JSONObject* impl_, const String& name_)
    : PropertiesHolderDetails::JSONObject(QString::fromStdString(name_))
    , parent(impl_)
{
    auto iter = std::find_if(parent->children.begin(), parent->children.end(), [this](const JSONObject* child) {
        return child->name == name;
    });
    DVASSERT(iter == parent->children.end());
    parent->children.push_back(this);
    jsonObject = parent->jsonObject[name].toObject();
}

PropertiesItem::Impl::~Impl()
{
    DVASSERT(parent != nullptr)
    if (wasChanged)
    {
        SaveToParent();
    }
    parent->children.remove(this);
}

PropertiesHolder::Impl::~Impl()
{
    SaveToFile();
}

template <typename T, typename std::enable_if<std::is_fundamental<T>::value, int>::type>
void PropertiesItem::Impl::Set(const QString& key, T value)
{
    jsonObject[key] = value;
}

template <typename T, typename std::enable_if<std::is_pointer<T>::value, int>::type>
void PropertiesItem::Impl::Set(const QString& key, const T& value)
{
    static_assert("unsupported type: pointer");
}

template <typename T, typename std::enable_if<!std::is_pointer<T>::value && !std::is_fundamental<T>::value, int>::type>
void PropertiesItem::Impl::Set(const QString& key, const T& value)
{
    jsonObject[key] = ToValue(value);
}

QJsonValue PropertiesItem::Impl::ToValue(const QString& value)
{
    return QJsonValue(value);
}

QJsonValue PropertiesItem::Impl::ToValue(const QByteArray& value)
{
    return QJsonValue(QString(value.toBase64()));
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

template <typename T, typename std::enable_if<std::is_fundamental<T>::value, int>::type>
T PropertiesItem::Impl::FromValue(const QJsonValue& value, const T& defaultValue)
{
    QVariant var = value.toVariant();
    if (var.canConvert<T>())
    {
        return var.value<T>();
    }
    return defaultValue;
}

template <typename T, typename std::enable_if<std::is_pointer<T>::value, int>::type>
T PropertiesItem::Impl::FromValue(const QJsonValue& value, const T& defaultValue)
{
    static_assert("unsupported type: pointer");
}

QString PropertiesItem::Impl::FromValue(const QJsonValue& value, const QString& defaultValue)
{
    return value.toString(defaultValue);
}

QByteArray PropertiesItem::Impl::FromValue(const QJsonValue& value, const QByteArray& defaultValue)
{
    if (value.isString())
    {
        return QByteArray::fromBase64(value.toString().toUtf8());
    }
    else
    {
        return defaultValue;
    }
}

void PropertiesHolder::Impl::SetDirectory(const FilePath& dirPath)
{
    DVASSERT(!dirPath.IsEmpty());
    QString dirPathStr = QString::fromStdString(dirPath.GetAbsolutePathname());
    QFileInfo fileInfo(dirPathStr);
    if (!fileInfo.isDir())
    {
        DVASSERT_MSG(false, "Given filePath must be a directory");
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
    if (!file.open(QFile::ReadOnly))
    {
        Logger::Error("Can not open file %s", filePath.toUtf8().data());
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
        DVASSERT_MSG(false, "Unsupported format of JSON file");
    }
}

void PropertiesHolder::Impl::SaveToFile()
{
    // right now we not need to save root element with existed child
    DVASSERT(children.empty());
    QString filePath = storagePath.absoluteFilePath();
    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
    {
        Logger::Error("Can not open file %s", filePath.toUtf8().data());
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

void PropertiesItem::Impl::SaveToParent()
{
    static_cast<Impl*>(parent)->wasChanged = true;
    parent->jsonObject[name] = jsonObject;
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

#define SAVE_IF_ACCEPTABLE(value, type, T, key) \
    if (type == Type::Instance<T>()) \
    { \
        DVASSERT(nullptr != impl); \
        try \
        { \
            impl->Set(keyStr, value.Get<T>()); \
        } \
        catch (const Any::Exception& exception) \
        { \
            Logger::Error("PropertiesHolder::Save: can not get type %s with message %s", type->GetName(), exception.what()); \
        } \
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
        catch (const Any::Exception& exception) \
        { \
            Logger::Error("PropertiesHolder::Load: can not get type %s with message %s", type->GetName(), exception.what()); \
        } \
        return retVal; \
    }

#define ENUM_TYPES(METHOD, value, type, key) \
    METHOD(value, type, bool, key) \
    METHOD(value, type, int32, key) \
    METHOD(value, type, int64, key) \
    METHOD(value, type, float32, key) \
    METHOD(value, type, float64, key) \
    METHOD(value, type, QString, key) \
    METHOD(value, type, QByteArray, key)

void PropertiesItem::Set(const String& key, const Any& value)
{
    const Type* type = value.GetType();
    QString keyStr = QString::fromStdString(key);
    impl->wasChanged = true;
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
    Logger::Error("PropertiesHolder::Load: Can not load value for type %s by key %s", type->GetName(), key.c_str());
    return Any();
}

} // namespace TArc
} // namespace DAVA
