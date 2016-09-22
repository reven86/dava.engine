#pragma once

#include "Base/BaseTypes.h"
#include "Math/Color.h"
#include "FileSystem/FilePath.h"

#include <QString>
#include <QPixmap>
#include <QColor>

// Different string utilities.
// Truncate the file extension.
QString TruncateFileExtension(const QString& fileName, const QString& extension);
bool FindAndReplace(DAVA::String& str, const DAVA::String& from, const DAVA::String& to);

QPixmap CreateIconFromColor(const QColor& color);

DAVA::Color QColorToColor(const QColor& qtColor);

QColor ColorToQColor(const DAVA::Color& davaColor);

void ShowFileInExplorer(const QString& path);

void ConnectApplicationFocus();

QString EscapeString(const QString& str);
QString UnescapeString(const QString& str);
