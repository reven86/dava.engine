#pragma once

#include "Base/BaseTypes.h"
#include "Math/Color.h"
#include "FileSystem/FilePath.h"
#include "Debug/DVAssert.h"

#include <QString>
#include <QPixmap>
#include <QColor>
#include <QRegularExpression>

// Different string utilities.
// Truncate the file extension.
extern QString TruncateFileExtension(const QString& fileName, const QString& extension);
extern bool FindAndReplace(DAVA::String& str, const DAVA::String& from, const DAVA::String& to);

extern QPixmap CreateIconFromColor(const QColor& color);

extern DAVA::Color QColorToColor(const QColor& qtColor);

extern QColor ColorToQColor(const DAVA::Color& davaColor);

void MakeAppForeground();
void RestoreMenuBar();

template <typename T>
T StringToVector(const QString& str)
{
    static_assert(std::is_same<T, DAVA::Vector2>::value ||
                  std::is_same<T, DAVA::Vector3>::value ||
                  std::is_same<T, DAVA::Vector4>::value,
                  "this function works only for types Vector2, Vector3 and Vector4"
                  );

    QRegularExpression expr("[+-]?[\\d*]*[.]?[\\d*]+");
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
    int count = DAVA::Min(floatList.size(), static_cast<int>(T::AXIS_COUNT));
    for (int i = 0; i < count; i++)
    {
        vector.data[i] = floatList[i];
    }
    return vector;
}
