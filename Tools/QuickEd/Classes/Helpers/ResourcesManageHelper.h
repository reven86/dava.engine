#pragma once

class QString;
class QStringList;

class ResourcesManageHelper
{
public:
    static void InitInternalResources();
    // Helper to work with resources
    static QString GetFontRelativePath(const QString& resourceFileName, bool graphicsFont = false);
    static QStringList GetFontsList();
};
