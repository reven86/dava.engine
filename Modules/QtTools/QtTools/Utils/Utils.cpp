#include "Utils.h"
#include "FileSystem/FileSystem.h"
#include "Render/TextureDescriptor.h"
#include <QPainter>
#include <QProcess>
#include <QDir>
#include <QApplication>

// Truncate the file extension.
QString TruncateFileExtension(const QString& fileName, const QString& extension)
{
    // Just wrap around the particular DAVA engine functions.

    DAVA::String truncatedName = fileName.toStdString();

    size_t truncatedStringLen = truncatedName.length() - extension.length();
    bool endsWithExtension = false;
    if (fileName.length() >= extension.length())
    {
        endsWithExtension = (truncatedName.compare(truncatedStringLen, extension.length(), extension.toStdString()) == 0);
    }

    if (endsWithExtension)
    {
        truncatedName.resize(truncatedStringLen);
    }

    return QString::fromStdString(truncatedName);
}

bool FindAndReplace(DAVA::String& str, const DAVA::String& from, const DAVA::String& to)
{
    size_t startPos = str.find(from);
    if (startPos == DAVA::String::npos)
        return false;
    str.replace(startPos, from.length(), to);
    return true;
}

QPixmap CreateIconFromColor(const QColor& color)
{
    QPixmap pix(16, 16);
    QPainter p(&pix);

    if (color.alpha() < 255)
    {
        // QtDocumentation QPainter::drawRect : A filled rectangle has a size of rectangle.size()
        p.fillRect(QRect(0, 0, 16, 16), QColor(250, 250, 250));
        p.fillRect(QRect(0, 0, 8, 8), QColor(150, 150, 150));
        p.fillRect(QRect(8, 8, 16, 16), QColor(150, 150, 150));
    }

    p.fillRect(QRect(0, 0, 16, 16), color);
    return pix;
}

DAVA::Color QColorToColor(const QColor& qtColor)
{
    return DAVA::Color(qtColor.redF(), qtColor.greenF(), qtColor.blueF(), qtColor.alphaF());
}

QColor ColorToQColor(const DAVA::Color& davaColor)
{
    DAVA::float32 maxC = std::max({ 1.0f, davaColor.r, davaColor.g, davaColor.b });

    return QColor::fromRgbF(davaColor.r / maxC, davaColor.g / maxC, davaColor.b / maxC, DAVA::Clamp(davaColor.a, 0.0f, 1.0f));
}

#if !defined(__DAVAENGINE_MACOS__)
void MakeAppForeground()
{
}

void RestoreMenuBar()
{
}
#endif

namespace StringPropertyDelegateDetails
{
//we need to store sequence in order
DAVA::Vector<std::pair<QChar, QString>> escapeSequences = {
    { '\\', QStringLiteral("\\\\") },
    { '\n', QStringLiteral("\\n") },
    { '\r', QStringLiteral("\\r") },
    { '\t', QStringLiteral("\\t") },
};
}

//replace strings with escape characters
QString EscapeString(const QString& str)
{
    QString stringToReplace(str);
    for (const auto& pair : StringPropertyDelegateDetails::escapeSequences)
    {
        stringToReplace.replace(pair.second, pair.first);
    }
    return stringToReplace;
}

//replace escape characters with their string form
QString UnescapeString(const QString& str)
{
    QString stringToReplace(str);
    for (const auto& pair : StringPropertyDelegateDetails::escapeSequences)
    {
        stringToReplace.replace(pair.first, pair.second);
    }
    return stringToReplace;
}
