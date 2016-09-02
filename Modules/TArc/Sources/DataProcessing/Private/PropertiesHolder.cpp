#include "DataProcessing/PropertiesHolder.h"
#include "Base/Any.h"
#include "FileSystem/FileSystem.h"
#include "Debug/DVAssert.h"

#include <QVariant>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJSonValue>
#include <QJsonArray>
#include <QDebug>

namespace DAVA
{
namespace TArc
{

struct PropertiesHolder::Impl
{
    Impl(const String &name_, const FilePath &directory);
    Impl(Impl *impl_, const String &name_);
    ~Impl();

    template<typename T, typename std::enable_if<std::is_fundamental<T>::value, int>::type = 0>
    void Save(T value, const QString &key);

    template<typename T, typename std::enable_if<std::is_pointer<T>::value, int>::type = 0>
    void Save(const T &value, const QString &key);

    template<typename T, typename std::enable_if<!std::is_pointer<T>::value && !std::is_fundamental<T>::value, int>::type = 0>
    void Save(const T &value, const QString &key);

    QJsonValue ToValue(const QString &value);
    QJsonValue ToValue(const QByteArray &value);

    template<typename T>
    T Load(const QString &key, const T &defaultValue);

    template<typename T, typename std::enable_if<std::is_fundamental<T>::value, int>::type = 0>
    T FromValue(const QJsonValue &value, const T &defaultValue);

    template<typename T, typename std::enable_if<std::is_pointer<T>::value, int>::type = 0>
    T FromValue(const QJsonValue &value, const T &defaultValue);

    QString FromValue(const QJsonValue &value, const QString &defaultValue);
    QByteArray FromValue(const QJsonValue &value, const QByteArray &defaultValue);

    void SetDirectory(const FilePath &filePath);

private:
    void LoadFromFile();
    void SaveToFile();
    void SaveToParent();

    QString name;
    FilePath storagePath;
    UnorderedSet<String> createdSubholders;
    Impl *parent = nullptr;
    QJsonObject jsonObject;
    bool wasChanged = false;
    
};

PropertiesHolder::Impl::Impl(const String &name_, const FilePath &directory)
    : name(QString::fromStdString(name_))
{
    DVASSERT(!directory.IsEmpty());
    SetDirectory(directory);
}

PropertiesHolder::Impl::Impl(Impl *impl_, const String &name_)
    : name(QString::fromStdString(name_))
    , parent(impl_)
{
    DVASSERT(parent->createdSubholders.find(name_) == parent->createdSubholders.end());
    parent->createdSubholders.insert(name_);
    jsonObject = parent->jsonObject[name].toObject();
}

PropertiesHolder::Impl::~Impl()
{
    if (parent != nullptr)
    {
        parent->createdSubholders.erase(name.toStdString());
        if (wasChanged)
        {
            SaveToParent();
        }
    }
    else
    {
        SaveToFile();
    }
}

template<typename T, typename std::enable_if<std::is_fundamental<T>::value, int>::type>
void PropertiesHolder::Impl::Save(T value, const QString &key)
{
    wasChanged = true;
    jsonObject[key] = value;
}

template<typename T, typename std::enable_if<std::is_pointer<T>::value, int>::type>
void PropertiesHolder::Impl::Save(const T &value, const QString &key)
{
    static_assert("unsupported type: pointer");
}

template<typename T, typename std::enable_if<!std::is_pointer<T>::value && !std::is_fundamental<T>::value, int>::type>
void PropertiesHolder::Impl::Save(const T &value, const QString &key)
{
    wasChanged = true;
    jsonObject[key] = ToValue(value);
}

QJsonValue PropertiesHolder::Impl::ToValue(const QString &value)
{
    return QJsonValue(value);
}

QJsonValue PropertiesHolder::Impl::ToValue(const QByteArray &value)
{
    return QJsonValue(QString(value.toBase64()));
}

template<typename T>
T PropertiesHolder::Impl::Load(const QString &key, const T &defaultValue)
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

template<typename T, typename std::enable_if<std::is_fundamental<T>::value, int>::type>
T PropertiesHolder::Impl::FromValue(const QJsonValue &value, const T &defaultValue)
{
    QVariant var = value.toVariant();
    if (var.canConvert<T>())
    {
        return var.value<T>();
    }
    return defaultValue;
}

template<typename T, typename std::enable_if<std::is_pointer<T>::value, int>::type>
T PropertiesHolder::Impl::FromValue(const QJsonValue &value, const T &defaultValue)
{
    static_assert("unsupported type: pointer");
}

QString PropertiesHolder::Impl::FromValue(const QJsonValue &value, const QString &defaultValue)
{
    return value.toString(defaultValue);
}

QByteArray PropertiesHolder::Impl::FromValue(const QJsonValue &value, const QByteArray &defaultValue)
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

void PropertiesHolder::Impl::SetDirectory(const FilePath &filePath)
{
    if (!FileSystem::Instance()->Exists(filePath))
    {
        DVASSERT("Given filePath must be a directory");
        return;
    }
    storagePath = filePath + name.toUtf8().data();
    LoadFromFile();
}

void PropertiesHolder::Impl::LoadFromFile()
{
    DVASSERT(FileSystem::Instance()->Exists(storagePath));
    String content = FileSystem::Instance()->ReadFileContents(storagePath);
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(QByteArray::fromStdString(content), &error);
    if (error.error != QJsonParseError::NoError)
    {
        DVASSERT_MSG(false, Format("JSON file corrupted: error %s", error.errorString().toUtf8().data()).c_str());
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
    DVASSERT(!storagePath.IsEmpty());
    ScopedPtr<File> file(File::Create(storagePath, File::CREATE | File::WRITE));
    DVASSERT(file);
    QJsonDocument document(jsonObject);
    QByteArray json = document.toJson(QJsonDocument::Indented);
    DVVERIFY(file->WriteString(json.toStdString(), false));
}

void PropertiesHolder::Impl::SaveToParent()
{
    parent->wasChanged = true;
    parent->jsonObject[name] = jsonObject;
}

PropertiesHolder::PropertiesHolder(const String &projectName, const FilePath &directory)
    : impl(new Impl(projectName, directory))
{

}

PropertiesHolder::~PropertiesHolder() = default;


PropertiesHolder::PropertiesHolder(PropertiesHolder &&holder)
    : impl(std::move(holder.impl))
{
}

PropertiesHolder& PropertiesHolder::operator = (PropertiesHolder &&holder)
{
    if (this != &holder)
    {
        impl = std::move(holder.impl);
    }
    return *this;
}

PropertiesHolder PropertiesHolder::SubHolder(const String &holderName) const
{
    return PropertiesHolder(*this, holderName);
}

PropertiesHolder::PropertiesHolder(const PropertiesHolder &parent, const String &name)
    : impl(new Impl(parent.impl.get(), name))
{

}

#define SAVE_IF_ACCEPTABLE(value, type, T, key) \
    if(type == Type::Instance<T>()) \
    { \
        impl->Save(value.Get<T>(), key); \
    }

#define LOAD_IF_ACCEPTABLE(value, type, T, key) \
    if(type == Type::Instance<T>()) \
    { \
        return Any(impl->Load(key, value.Get<T>())); \
    }

#define ENUM_TYPES(METHOD, value, type, key) \
    METHOD(value, type, bool, key) \
    METHOD(value, type, int32, key) \
    METHOD(value, type, int64, key) \
    METHOD(value, type, float32, key) \
    METHOD(value, type, float64, key) \
    METHOD(value, type, QString, key) \
    METHOD(value, type, QByteArray, key)

void PropertiesHolder::Save(const Any &value, const String &key)
{
    const Type *type = value.GetType();
    QString keyStr = QString::fromStdString(key);
    ENUM_TYPES(SAVE_IF_ACCEPTABLE, value, type, keyStr);
}

Any PropertiesHolder::Load(const String &key, const Any& defaultValue) const
{
    const Type *type = defaultValue.GetType();
    QString keyStr = QString::fromStdString(key);
    ENUM_TYPES(LOAD_IF_ACCEPTABLE, defaultValue, type, keyStr);
    return Any();
}

} // namespace TArc
} // namespace DAVA
