#pragma once

#include <QStringList>
#include <memory>

class ProjectStructure
{
public:
    ProjectStructure(const QStringList& supportedExtensions);
    ~ProjectStructure();

    void AddProjectDirectory(const QString& directory);
    void RemoveProjectDirectory(const QString& directory);
    void RemoveAllProjectDirectories();
    QStringList GetProjectDirectories() const;

    QStringList GetFiles(const QString& extension) const;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
