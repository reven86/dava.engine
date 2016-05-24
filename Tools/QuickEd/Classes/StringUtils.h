#ifndef __UIEditor__StringUtils__
#define __UIEditor__StringUtils__

#include "DAVAEngine.h"
#include <QString>

namespace DAVA
{
// Different string utilities.
// Truncate the file extension.
QString TruncateFileExtension(const QString& fileName, const QString& extension);
bool FindAndReplace(String& str, const String& from, const String& to);
};

#endif /* defined(__UIEditor__Utils__) */
