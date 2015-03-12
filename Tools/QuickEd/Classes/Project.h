#ifndef __UI_EDITOR_PROJECT_H__
#define __UI_EDITOR_PROJECT_H__

#include <QObject>
#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"

class PackageNode;
class LegacyControlData;

class Project : public QObject
{
    Q_OBJECT
public:
    Project();
    virtual ~Project();

    bool Open(const QString &path);

    DAVA::RefPtr<PackageNode> NewPackage(const QString &path);
    DAVA::RefPtr<PackageNode> OpenPackage(const QString &path);
    bool SavePackage(PackageNode *package);
    bool SaveAsPackage(PackageNode *package){return false;}
    QString GetProjectDir() const { return projectDir; };

signals:
    void ProjectOpened();

private:
    QString projectFile;
    QString projectDir;
    
    LegacyControlData *legacyData;
};

#endif // __UI_EDITOR_PROJECT_H__
