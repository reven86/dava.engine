#include "Utils.h"
#include "FileSystem/FileSystem.h"
#include "Render/TextureDescriptor.h"
#include <QPainter>
#include <QProcess>

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
    p.setPen(QColor(0, 0, 0, 0));

    if (color.alpha() < 255)
    {
        p.setBrush(QColor(250, 250, 250));
        p.drawRect(QRect(0, 0, 15, 15));
        p.setPen(QColor(200, 200, 200));
        p.setBrush(QColor(150, 150, 150));
        p.drawRect(QRect(0, 0, 7, 7));
        p.drawRect(QRect(8, 8, 15, 15));
    }

    p.setBrush(QBrush(color));
    p.drawRect(QRect(0, 0, 15, 15));
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

void ShowFileInExplorer(const QString& path)
{
#if defined(Q_OS_MAC)
    QStringList args;
    args << "-e";
    args << "tell application \"Finder\"";
    args << "-e";
    args << "activate";
    args << "-e";
    args << "select POSIX file \"" + path + "\"";
    args << "-e";
    args << "end tell";
    QProcess::startDetached("osascript", args);
#elif defined(Q_OS_WIN)
    QStringList args;
    args << "/select," << QDir::toNativeSeparators(path);
    QProcess::startDetached("explorer", args);
#endif //
}
