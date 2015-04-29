#ifndef QUICKED__PROJECT_H__
#define QUICKED__PROJECT_H__

#include <QObject>
#include "Model/LegacyEditorUIPackageLoader.h"

class PackageNode;
class EditorFontSystem;

class Project : public QObject, public DAVA::Singleton<Project>
{
    Q_OBJECT
    Q_PROPERTY(bool isOpen READ IsOpen NOTIFY IsOpenChanged)

public:
    Project(QObject *parent = nullptr);
    virtual ~Project();

    bool Open(const QString &path);
    bool CheckAndUnlockProject(const QString& projectPath);

    DAVA::RefPtr<PackageNode> NewPackage(const QString &path);
    DAVA::RefPtr<PackageNode> OpenPackage(const QString &path);
    bool SavePackage(PackageNode *package);
    EditorFontSystem *GetEditorFontSystem() const;
signals:
    void ProjectOpened();

private:
    bool OpenInternal(const QString &path);
    QString projectDir;
    
    LegacyControlData *legacyData;
    EditorFontSystem *editorFontSystem;
    //properties
public:
    bool IsOpen() const;
signals:
    void IsOpenChanged(bool arg);
private:
    void SetIsOpen(bool arg);
    bool isOpen;
};

#endif // QUICKED__PROJECT_H__
