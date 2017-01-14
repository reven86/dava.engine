#include "QtDavaConvertion.h"
#include "QtTools/Utils/Utils.h"
#include "Utils/StringFormat.h"
#include <QString>
#include <QColor>
#include <QVariant>
#include <QVector2D>

using namespace DAVA;

DAVA::String QStringToString(const QString& str)
{
    return DAVA::String(str.toStdString());
}

QString StringToQString(const DAVA::String& str)
{
    return QString::fromStdString(str);
}

DAVA::WideString QStringToWideString(const QString& str)
{
#ifdef __DAVAENGINE_MACOS__
    return str.toStdWString();
#else
    return DAVA::WideString((wchar_t*)str.unicode(), str.length());
#endif
}

QString WideStringToQString(const DAVA::WideString& str)
{
#ifdef __DAVAENGINE_MACOS__
    return QString::fromStdWString(str);
#else
    return QString((const QChar*)str.c_str(), static_cast<int>(str.length()));
#endif
}

DAVA::Vector2 QVector2DToVector2(const QVector2D& vector)
{
    return DAVA::Vector2(vector.x(), vector.y());
}

QVector2D Vector2ToQVector2D(const DAVA::Vector2& vector)
{
    return QVector2D(vector.x, vector.y);
}

QColor HexToQColor(const QString& str)
{
    QColor color;
    if (!str.startsWith("#"))
        return color;
    int len = str.length() - 1;
    switch (len)
    {
    case 6: //RGB
    {
        int r, g, b;
        if (3 == sscanf(str.toStdString().c_str(), "#%02x%02x%02x", &r, &g, &b))
        {
            color.setRgb(qRgb(r, g, b));
        }
    }
    break;
    case 8: //RGBA
    {
        int r, g, b, a;
        if (4 == sscanf(str.toStdString().c_str(), "#%02x%02x%02x%02x", &r, &g, &b, &a))
        {
            color.setRgba(qRgba(r, g, b, a));
        }
    }
    break;
    default:
        break;
    }

    return color;
}

QString QColorToHex(const QColor& color)
{
    QString str;
    str.sprintf("#%02x%02x%02x%02x", color.red(), color.green(), color.blue(), color.alpha());

    return str;
}

QString VariantToQString(const VariantType& val, const InspMember* memberInfo)
{
    switch (val.GetType())
    {
    case VariantType::TYPE_NONE:
        return QString();
    case VariantType::TYPE_BOOLEAN:
        return QString(val.AsBool() ? "true" : "false");
    case VariantType::TYPE_INT8:
        return QVariant(val.AsInt8()).toString();
    case VariantType::TYPE_UINT8:
        return QVariant(val.AsUInt8()).toString();
    case VariantType::TYPE_INT16:
        return QVariant(val.AsInt16()).toString();
    case VariantType::TYPE_UINT16:
        return QVariant(val.AsUInt16()).toString();
    case VariantType::TYPE_INT32:
    {
        const InspDesc& memberInspDesc = memberInfo->Desc();
        if (memberInspDesc.type == InspDesc::T_ENUM)
        {
            int32 e = val.AsInt32();
            return QString::fromStdString(memberInspDesc.enumMap->ToString(e));
        }
        else if (memberInfo->Desc().type == InspDesc::T_FLAGS)
        {
            int32 e = val.AsInt32();
            QString res = "";
            int p = 0;
            while (e)
            {
                if ((e & 0x01) != 0)
                {
                    if (!res.isEmpty())
                        res += " | ";

                    const int32 enumValue = 1 << p;
                    res += QString::fromStdString(memberInspDesc.enumMap->ToString(enumValue));
                }
                p++;
                e >>= 1;
            }
            return res;
        }
        else
        {
            return QVariant(val.AsInt32()).toString();
        }
    }

    case VariantType::TYPE_UINT32:
        return QVariant(val.AsUInt32()).toString();

    case VariantType::TYPE_INT64:
        return QVariant(val.AsInt64()).toString();

    case VariantType::TYPE_UINT64:
        return QVariant(val.AsUInt64()).toString();

    case VariantType::TYPE_FLOAT:
        return QVariant(val.AsFloat()).toString();

    case VariantType::TYPE_FLOAT64:
        return QVariant(val.AsFloat64()).toString();

    case VariantType::TYPE_STRING:
        return StringToQString(val.AsString());

    case VariantType::TYPE_WIDE_STRING:
        return WideStringToQString(val.AsWideString());

    case VariantType::TYPE_FASTNAME:
        return StringToQString(val.AsFastName().c_str());

    case VariantType::TYPE_VECTOR2:
        return StringToQString(Format("%g; %g", val.AsVector2().x, val.AsVector2().y));

    case VariantType::TYPE_COLOR:
        return QColorToHex(ColorToQColor(val.AsColor()));

    case VariantType::TYPE_VECTOR4:
        return StringToQString(Format("%g; %g; %g; %g", val.AsVector4().x, val.AsVector4().y, val.AsVector4().z, val.AsVector4().w));

    case VariantType::TYPE_FILEPATH:
        return StringToQString(val.AsFilePath().GetStringValue());

    case VariantType::TYPE_BYTE_ARRAY:
    case VariantType::TYPE_KEYED_ARCHIVE:
    case VariantType::TYPE_VECTOR3:

    case VariantType::TYPE_MATRIX2:
    case VariantType::TYPE_MATRIX3:
    case VariantType::TYPE_MATRIX4:
    case VariantType::TYPE_AABBOX3:
    default:
        DVASSERT(false);
        break;
    }
    return QString();
}

String AnyToString(const Any& val)
{
    if (val.CanGet<int32>())
    {
        return Format("%d", val.Get<int32>());
    }
    else if (val.CanGet<uint64>())
    {
        return Format("%ld", val.Get<uint64>());
    }
    else if (val.CanGet<int64>())
    {
        return Format("%ldL", val.Get<int64>());
    }
    else if (val.CanGet<uint16>())
    {
        return Format("%d", val.Get<int16>());
    }
    else if (val.CanGet<int16>())
    {
        return Format("%d", val.Get<int16>());
    }
    else if (val.CanGet<uint8>())
    {
        return Format("%d", val.Get<uint8>());
    }
    else if (val.CanGet<int8>())
    {
        return Format("%d", val.Get<int8>());
    }
    else if (val.CanGet<float32>())
    {
        return Format("%f", val.Get<float32>());
    }
    else if (val.CanGet<String>())
    {
        return val.Get<String>();
    }
    else if (val.CanGet<FilePath>())
    {
        return val.Get<FilePath>().GetFrameworkPath();
    }
    else if (val.CanGet<bool>())
    {
        return val.Get<bool>() ? "true" : "false";
    }
    
    return String("");
}
