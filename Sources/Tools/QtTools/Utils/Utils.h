#pragma once

#include "Base/BaseTypes.h"
#include "Math/Color.h"
#include "FileSystem/FilePath.h"

#include <QString>
#include <QPixmap>
#include <QColor>

// Different string utilities.
// Truncate the file extension.
extern QString TruncateFileExtension(const QString& fileName, const QString& extension);
extern bool FindAndReplace(DAVA::String& str, const DAVA::String& from, const DAVA::String& to);

extern QPixmap CreateIconFromColor(const QColor& color);

extern DAVA::Color QColorToColor(const QColor& qtColor);

extern QColor ColorToQColor(const DAVA::Color& davaColor);
