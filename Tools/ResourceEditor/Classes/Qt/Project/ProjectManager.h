#pragma once

#include "Base/Singleton.h"
#include "FileSystem/FilePath.h"

#include <QObject>
#include <QVector>

#include <memory>

class SpritesPackerModule;
class ProjectStructure;
class ProjectManager : public QObject, public DAVA::Singleton<ProjectManager>
{
    Q_OBJECT

public:
    ProjectManager();
    ~ProjectManager();
};
