#ifndef __QTDAVACONVERTION_H__
#define __QTDAVACONVERTION_H__
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "FileSystem/VariantType.h"
#include "qmetatype.h"

#include <QString>
#include <QRegularExpression>

class QString;
class QColor;
class QVector2D;

Q_DECLARE_METATYPE(DAVA::VariantType);

DAVA::String QStringToString(const QString& str);
QString StringToQString(const DAVA::String& str);

DAVA::WideString QStringToWideString(const QString& str);
QString WideStringToQString(const DAVA::WideString& str);

QColor HexToQColor(const QString& str);
QString QColorToHex(const QColor& color);

DAVA::Vector2 QVector2DToVector2(const QVector2D& vector);
QVector2D Vector2ToQVector2D(const DAVA::Vector2& vector);

template <typename T>
T StringToVector(const QString& str);

template <typename T>
T StringToVector(const QString& str)
{
    static_assert(std::is_same<T, DAVA::Vector2>::value ||
                  std::is_same<T, DAVA::Vector3>::value ||
                  std::is_same<T, DAVA::Vector4>::value,
                  "this function works only for types Vector2, Vector3 and Vector4"
                  );

    QRegularExpression expr("[\\d*]*[.]?[\\d*]+");
    QRegularExpressionMatchIterator iter = expr.globalMatch(str);
    QList<float> floatList;
    while (iter.hasNext())
    {
        QRegularExpressionMatch match(iter.next());
        if (match.hasMatch())
        {
            QString matchedStr = match.captured(0);
            bool ok;
            floatList << matchedStr.toFloat(&ok);
            DVASSERT(ok);
        }
    }
    T vector;
    int count = Min(floatList.size(), static_cast<int32>(T::AXIS_COUNT));
    for (int i = 0; i < count; i++)
    {
        vector.data[i] = floatList[i];
    }
    return vector;
}

#endif // __QTDAVACONVERTION_H__
