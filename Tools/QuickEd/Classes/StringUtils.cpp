#include "StringUtils.h"

namespace DAVA
{
// Truncate the file extension.
QString TruncateFileExtension(const QString& fileName, const QString& extension)
{
    // Just wrap around the particular DAVA engine functions.

    String truncatedName = fileName.toStdString();

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

bool FindAndReplace(String& str, const String& from, const String& to)
{
    size_t startPos = str.find(from);
    if (startPos == String::npos)
        return false;
    str.replace(startPos, from.length(), to);
    return true;
}
}
