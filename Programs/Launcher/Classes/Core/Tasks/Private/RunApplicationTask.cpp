#include "Core/Tasks/RunApplicationTask.h"
#include "Core/ApplicationManager.h"

#include "Data/ConfigParser.h"

#include <QtHelpers/ProcessHelper.h>

#include <QObject>
#include <QFile>

namespace RunApplicationTaskDetails
{
#ifdef Q_OS_WIN
void FixLocalAppPath_kostil(QString& runPath)
{
    if (QFile::exists(runPath) == false)
    {
        //Standalone ResourceEditor kostil
        //BA-manager not support different executable paths
        QString newString = "Programs/ResourceEditor/ResourceEditor.exe";
        QString oldString = "Tools/ResourceEditor/ResourceEditor.exe";
        if (runPath.endsWith(newString))
        {
            runPath.replace(newString, oldString);
        }
    }
}
#endif //Q_OS_WIN
}

RunApplicationTask::RunApplicationTask(ApplicationManager* appManager, const QString& branch_, const QString& app_, const QString& version_)
    : RunTask(appManager)
    , branch(branch_)
    , app(app_)
    , version(version_)
{
}

QString RunApplicationTask::GetDescription() const
{
    return QObject::tr("Launching application %1").arg(app);
}

void RunApplicationTask::Run()
{
    AppVersion* localVersion = appManager->GetLocalConfig()->GetAppVersion(branch, app, version);
    if (localVersion == nullptr)
    {
        SetError(QObject::tr("Version %1 of application %2 is not installed").arg(version).arg(app));
        return;
    }
    QString runPath = appManager->GetApplicationDirectory(branch, app, localVersion->isToolSet, false);
    if (runPath.isEmpty())
    {
        SetError(QObject::tr("Application %1 in branch %2 not exists!").arg(app).arg(branch));
        return;
    }
    QString localAppPath = appManager->GetLocalAppPath(localVersion, app);
    runPath += localAppPath;
#ifdef Q_OS_WIN
    RunApplicationTaskDetails::FixLocalAppPath_kostil(runPath);
#endif //Q_OS_WIN
    if (!QFile::exists(runPath))
    {
        SetError(QObject::tr("Application not found: %1").arg(runPath));
        return;
    }

    QString appName = runPath.right(runPath.size() - runPath.lastIndexOf("/") - 1);
    if (!ProcessHelper::IsProcessRuning(runPath))
    {
        ProcessHelper::RunProcess(runPath);
    }
    else
    {
        SetError(QObject::tr("Application %1 is already launched").arg(appName));
    }
}
