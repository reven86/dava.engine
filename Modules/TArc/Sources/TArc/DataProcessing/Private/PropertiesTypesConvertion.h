#pragma once

namespace DAVA
{
namespace TArc
{
template <>
bool PropertiesItem::Impl::FromValue(const QJsonValue& value, const bool& defaultValue)
{
    return value.toBool(defaultValue);
}

template <>
int32 PropertiesItem::Impl::FromValue(const QJsonValue& value, const int32& defaultValue)
{
    //QJsonValue actually doesn't store int
    return static_cast<int32>(value.toDouble(defaultValue));
}

template <>
uint32 PropertiesItem::Impl::FromValue(const QJsonValue& value, const uint32& defaultValue)
{
    //QJsonValue actually doesn't store int
    return static_cast<uint32>(value.toDouble(defaultValue));
}

template <>
int64 PropertiesItem::Impl::FromValue(const QJsonValue& value, const int64& defaultValue)
{
    //QJsonValue actually doesn't store int
    return static_cast<int64>(value.toDouble(defaultValue));
}

template <>
uint64 PropertiesItem::Impl::FromValue(const QJsonValue& value, const uint64& defaultValue)
{
    //QJsonValue actually doesn't store int
    return static_cast<uint64>(value.toDouble(defaultValue));
}

template <>
float32 PropertiesItem::Impl::FromValue(const QJsonValue& value, const float32& defaultValue)
{
    return static_cast<float32>(value.toDouble(defaultValue));
}

template <>
float64 PropertiesItem::Impl::FromValue(const QJsonValue& value, const float64& defaultValue)
{
    return static_cast<float64>(value.toDouble(defaultValue));
}

template <>
QString PropertiesItem::Impl::FromValue(const QJsonValue& value, const QString& defaultValue)
{
    return value.toString(defaultValue);
}

template <>
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

template <>
QRect PropertiesItem::Impl::FromValue(const QJsonValue& value, const QRect& defaultValue)
{
    QByteArray defaultData;
    QDataStream defaultStream(&defaultData, QIODevice::WriteOnly);
    defaultStream << defaultValue;

    QByteArray loadedData = FromValue(value, defaultData);
    QDataStream loadedStream(&loadedData, QIODevice::ReadOnly);

    QRect rect;
    loadedStream >> rect;
    return rect;
}

template <>
FilePath PropertiesItem::Impl::FromValue(const QJsonValue& value, const FilePath& defaultValue)
{
    if (value.isString())
    {
        return FilePath(value.toString().toStdString());
    }
    else
    {
        return defaultValue;
    }
}

template <>
String PropertiesItem::Impl::FromValue(const QJsonValue& value, const String& defaultValue)
{
    if (value.isString())
    {
        return value.toString().toStdString();
    }
    else
    {
        return defaultValue;
    }
}

template <>
Vector<String> PropertiesItem::Impl::FromValue(const QJsonValue& value, const Vector<String>& defaultValue)
{
    if (value.isString())
    {
        Vector<String> retVal;
        QString stringValue = value.toString();
        QStringList stringList = stringValue.split(PropertiesHolderDetails::stringListDelimiter, QString::SkipEmptyParts);
        std::transform(stringList.begin(), stringList.end(), std::back_inserter(retVal), [](const QString& string) {
            return string.toStdString();
        });
        return retVal;
    }
    else
    {
        return defaultValue;
    }
}

template <>
Vector<FastName> PropertiesItem::Impl::FromValue(const QJsonValue& value, const Vector<FastName>& defaultValue)
{
    if (value.isString())
    {
        Vector<FastName> retVal;
        QString stringValue = value.toString();
        QStringList stringList = stringValue.split(PropertiesHolderDetails::stringListDelimiter, QString::SkipEmptyParts);
        std::transform(stringList.begin(), stringList.end(), std::back_inserter(retVal), [](const QString& string)
                       {
                           return FastName(string.toStdString());
                       });
        return retVal;
    }
    else
    {
        return defaultValue;
    }
}

template <>
QJsonValue PropertiesItem::Impl::ToValue(const bool& value)
{
    return QJsonValue(value);
}

template <>
QJsonValue PropertiesItem::Impl::ToValue(const int32& value)
{
    return QJsonValue(value);
}

template <>
QJsonValue PropertiesItem::Impl::ToValue(const uint32& value)
{
    return QJsonValue(static_cast<int32>(value));
}
template <>
QJsonValue PropertiesItem::Impl::ToValue(const int64& value)
{
    return QJsonValue(value);
}

template <>
QJsonValue PropertiesItem::Impl::ToValue(const uint64& value)
{
    return QJsonValue(static_cast<int64>(value));
}

template <>
QJsonValue PropertiesItem::Impl::ToValue(const float32& value)
{
    return QJsonValue(value);
}

template <>
QJsonValue PropertiesItem::Impl::ToValue(const float64& value)
{
    return QJsonValue(value);
}

template <>
QJsonValue PropertiesItem::Impl::ToValue(const QString& value)
{
    return QJsonValue(value);
}

template <>
QJsonValue PropertiesItem::Impl::ToValue(const QByteArray& value)
{
    return QJsonValue(QString(value.toBase64()));
}

template <>
QJsonValue PropertiesItem::Impl::ToValue(const QRect& value)
{
    QByteArray rectData;
    QDataStream rectStream(&rectData, QIODevice::WriteOnly);
    rectStream << value;
    return ToValue(rectData);
}

template <>
QJsonValue PropertiesItem::Impl::ToValue(const FilePath& value)
{
    return QJsonValue(QString::fromStdString(value.GetAbsolutePathname()));
}

template <>
QJsonValue PropertiesItem::Impl::ToValue(const String& value)
{
    return QJsonValue(QString::fromStdString(value));
}

template <>
QJsonValue PropertiesItem::Impl::ToValue(const Vector<String>& value)
{
    QStringList stringList;
    std::transform(value.begin(), value.end(), std::back_inserter(stringList), [](const String& string) {
#ifdef __DAVAENGINE_DEBUG__
        String errorMessage = Format("string to save %s contains special character used to save: %s", string.c_str(), PropertiesHolderDetails::stringListDelimiter);
        DVASSERT(string.find(PropertiesHolderDetails::stringListDelimiter) == String::npos, errorMessage.c_str());
#endif //__DAVAENGINE_DEBUG__
        return QString::fromStdString(string);
    });
    return stringList.join(PropertiesHolderDetails::stringListDelimiter);
}

template <>
QJsonValue PropertiesItem::Impl::ToValue(const Vector<FastName>& value)
{
    QStringList stringList;
    std::transform(value.begin(), value.end(), std::back_inserter(stringList), [](const FastName& string)
                   {
#ifdef __DAVAENGINE_DEBUG__
                       String errorMessage = Format("string to save %s contains special character used to save: %s", string.c_str(), PropertiesHolderDetails::stringListDelimiter);
                       DVASSERT(string.find(PropertiesHolderDetails::stringListDelimiter) == String::npos, errorMessage.c_str());
#endif //__DAVAENGINE_DEBUG__
                       return QString(string.c_str());
                   });
    return stringList.join(PropertiesHolderDetails::stringListDelimiter);
}

} // namespace TArc
} // namespace DAVA
