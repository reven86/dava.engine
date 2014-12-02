//
//  Project.h
//  UIEditor
//
//  Created by Dmitry Belsky on 11.9.14.
//
//

#ifndef __UI_EDITOR_PROJECT_H__
#define __UI_EDITOR_PROJECT_H__

#include <QObject>
#include "Base/BaseTypes.h"

class PackageNode;
class LegacyControlData;

class Project : QObject
{
    Q_OBJECT
public:
    Project();
    virtual ~Project();

    bool Open(const QString &path);

    PackageNode *NewPackage(const QString &path){return NULL;}
    PackageNode *OpenPackage(const QString &path);
    bool SavePackage(PackageNode *package);
    bool SaveAsPackage(PackageNode *package){return false;}

signals:
    void ProjectOpened();

private:
    QString projectFile;
    QString projectDir;
    
    LegacyControlData *legacyData;
};

#endif // __UI_EDITOR_PROJECT_H__
