#include "TArc/DataProcessing/TArcAnyCasts.h"

#include <QtTools/Utils/Utils.h>

#include <Base/Any.h>
#include <Base/FastName.h>

#include <QString>
#include <Qt>
#include <QIcon>

namespace DAVA
{
namespace TArc
{
QString StringToQString(const Any& value)
{
    return QString::fromStdString(value.Get<String>());
}

String QStringToString(const Any& value)
{
    return value.Get<QString>().toStdString();
}

QString FastNameToQString(const Any& value)
{
    return QString(value.Get<FastName>().c_str());
}

FastName QStringToFastName(const Any& value)
{
    return FastName(value.Get<QString>().toStdString());
}

QString CharPointerToQString(const Any& value)
{
    return QString(value.Get<const char*>());
}

template <typename T>
QString IntegralToQString(const Any& value)
{
    return QString::number(value.Get<T>());
}

// Don't create QString -> const char* cast function. It is impossible

Qt::CheckState BoolToCheckState(const Any& value)
{
    return value.Get<bool>() ? Qt::Checked : Qt::Unchecked;
}

bool CheckStateToBool(const Any& value)
{
    return value.Get<Qt::CheckState>() == Qt::Checked;
}

Color QColorToColorAny(const Any& value)
{
    return QColorToColor(value.Get<QColor>());
}

QColor ColorToQColorAny(const Any& value)
{
    return ColorToQColor(value.Get<Color>());
}

QIcon ColorToQIcon(const Any& value)
{
    return CreateIconFromColor(ColorToQColorAny(value));
}

QIcon QColorToQIcon(const Any& value)
{
    return CreateIconFromColor(value.Get<QColor>());
}

String ColorToString(const Any& value)
{
    return ColorToQColorAny(value.Get<Color>()).name().toStdString();
}

void RegisterAnyCasts()
{
    AnyCast<String, QString>::Register(&StringToQString);
    AnyCast<QString, String>::Register(&QStringToString);
    AnyCast<FastName, QString>::Register(&FastNameToQString);
    AnyCast<QString, FastName>::Register(&QStringToFastName);
    AnyCast<const char*, QString>::Register(&CharPointerToQString);
    AnyCast<int32, QString>::Register(&IntegralToQString<int32>);
    AnyCast<uint32, QString>::Register(&IntegralToQString<uint32>);
    AnyCast<int16, QString>::Register(&IntegralToQString<int16>);
    AnyCast<uint16, QString>::Register(&IntegralToQString<uint16>);
    AnyCast<int8, QString>::Register(&IntegralToQString<int8>);
    AnyCast<uint8, QString>::Register(&IntegralToQString<uint8>);
    AnyCast<int64, QString>::Register(&IntegralToQString<int64>);
    AnyCast<uint64, QString>::Register(&IntegralToQString<uint64>);
    AnyCast<size_t, QString>::Register(&IntegralToQString<size_t>);
    AnyCast<float32, QString>::Register(&IntegralToQString<float32>);
    AnyCast<float64, QString>::Register(&IntegralToQString<float64>);
    AnyCast<bool, Qt::CheckState>::Register(&BoolToCheckState);
    AnyCast<Qt::CheckState, bool>::Register(&CheckStateToBool);

    AnyCast<QColor, Color>::Register(&QColorToColorAny);
    AnyCast<Color, QColor>::Register(&ColorToQColorAny);
    AnyCast<Color, QIcon>::Register(&ColorToQIcon);
    AnyCast<QColor, QIcon>::Register(&QColorToQIcon);
    AnyCast<Color, String>::Register(&ColorToString);
}

} // namespace TArc
} // namespace DAVA
