#ifndef __QTDAVACONVERTION_H__
#define __QTDAVACONVERTION_H__
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "FileSystem/VariantType.h"
#include "qmetatype.h"

class QString;
class QColor;
class QVector2D;

Q_DECLARE_METATYPE(DAVA::VariantType);

DAVA::String QStringToString(const QString &str);
QString StringToQString(const DAVA::String &str);

DAVA::WideString QStringToWideString(const QString &str);
QString WideStringToQString(const DAVA::WideString &str);

DAVA::Color QColorToColor(const QColor &color);
QColor ColorToQColor(const DAVA::Color &color);

QColor HexToQColor(const QString &str);
QString QColorToHex(const QColor &color);

DAVA::Vector2 QVector2DToVector2(const QVector2D &vector);
QVector2D Vector2ToQVector2D(const DAVA::Vector2 &vector);

#endif // __QTDAVACONVERTION_H__
