#include "Core/Tasks/RemoveApplicationTask.h"

#include "Core/ApplicationManager.h"

#include "Utils/AppsCommandsSender.h"
#include "Utils/FileManager.h"

#include <QtHelpers/ProcessHelper.h>
#include <QtHelpers/HelperFunctions.h>

#include <QEventLoop>
#include <QElapsedTimer>

#include <atomic>
#include <thread>

RemoveApplicationTask::RemoveApplicationTask(ApplicationManager* appManager, const QString& branch_, const QString& app_)
    : RunTask(appManager)
    , branch(branch_)
    , app(app_)
{
}

QString RemoveApplicationTask::GetDescription() const
{
    ConfigParser* localConfig = appManager->GetLocalConfig();
    Application* localApp = localConfig->GetApplication(branch, app);
    if (localApp != nullptr)
    {
        AppVersion* localVersion = localApp->GetVersion(0);
        if (localVersion != nullptr)
        {
            return QObject::tr("Removing application %1").arg(appManager->GetAppName(app, localVersion->isToolSet));
        }
    }
    return QObject::tr("Removing application %1").arg(app);
}

void RemoveApplicationTask::Run()
{
    ConfigParser* localConfig = appManager->GetLocalConfig();
    Application* localApp = localConfig->GetApplication(branch, app);
    if (localApp == nullptr)
    {
        return;
    }
    AppVersion* localVersion = localApp->GetVersion(0);
    if (localVersion == nullptr)
    {
        return;
    }
    QStringList appNames;
    bool isToolSet = localVersion->isToolSet;
    if (isToolSet)
    {
        //check that we can remove folder "toolset" first
        for (const QString& fakeapp : localConfig->GetTranslatedToolsetApplications())
        {
            appNames << fakeapp;
        }
    }
    else
    {
        appNames << app;
    }

    using appWorkerFn = std::function<bool(const QString&, const QString&, AppVersion*)>;
    auto forEachApp = [appNames, localConfig, this](appWorkerFn fn) -> bool {
        for (const QString& fakeName : appNames)
        {
            Application* fakeApp = localConfig->GetApplication(branch, fakeName);
            if (fakeApp != nullptr)
            {
                AppVersion* fakeVersion = fakeApp->GetVersion(0);
                if (fakeVersion != nullptr)
                {
                    if (fn(branch, fakeName, fakeVersion) == false)
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    };

    using namespace std::placeholders;
    if (forEachApp(std::bind(&RemoveApplicationTask::CanRemoveApp, this, _1, _2, _3)))
    {
        forEachApp(std::bind(&RemoveApplicationTask::RemoveApplicationImpl, this, _1, _2, _3));
    }
}

bool RemoveApplicationTask::CanRemoveApp(const QString& branchID, const QString& appID, AppVersion* localVersion) const
{
    QString appDirPath = appManager->GetApplicationDirectory(branchID, appID, localVersion->isToolSet, false);
    if (appDirPath.isEmpty())
    {
        SetError(QObject::tr("Can not find application %1 in branch %2").arg(appID).arg(branchID));
        return true;
    }
    QString runPath = appDirPath + appManager->GetLocalAppPath(localVersion, appID);
    if (appManager->GetAppsCommandsSender()->HostIsAvailable(runPath))
    {
        if (appManager->CanTryStopApplication(appID))
        {
            if (TryStopApp(runPath))
            {
                return true;
            }
        }
    }

    if (ProcessHelper::IsProcessRuning(runPath))
    {
        SetError(QObject::tr("Can not remove application %1 in branch %2 because it still running").arg(appID, branchID));
        return false;
    }
    return true;
}

bool RemoveApplicationTask::TryStopApp(const QString& runPath) const
{
    using namespace std;
    if (appManager->GetAppsCommandsSender()->RequestQuit(runPath))
    {
        QEventLoop eventLoop;
        atomic<bool> isStillRunning(true);
        auto ensureAppIsNotRunning = [&eventLoop, &isStillRunning, runPath]() {
            QElapsedTimer timer;
            timer.start();
            const int maxWaitTimeMs = 10 * 1000;
            while (timer.elapsed() < maxWaitTimeMs && isStillRunning)
            {
                const int waitTimeMs = 100;
                this_thread::sleep_for(std::chrono::milliseconds(waitTimeMs));
                isStillRunning = ProcessHelper::IsProcessRuning(runPath);
            }
            eventLoop.quit();
        };
        std::thread workerThread(QtHelpers::InvokeInAutoreleasePool, ensureAppIsNotRunning);
        eventLoop.exec();
        workerThread.join();
        return isStillRunning == false;
    }
    return false;
}

bool RemoveApplicationTask::RemoveApplicationImpl(const QString& branchID, const QString& appID, AppVersion* localVersion)
{
    QString appDirPath = appManager->GetApplicationDirectory(branchID, appID, localVersion->isToolSet, false);
    bool success = FileManager::DeleteDirectory(appDirPath);
    if (success)
    {
        appManager->GetLocalConfig()->RemoveApplication(branchID, appID, localVersion->id);
        appManager->SaveLocalConfig();
    }
    else
    {
        SetError(QObject::tr("Can not remove directory %1\nApplication need to be reinstalled!\n"
                             "If this error occurs again - remove directory by yourself, please")
                 .arg(appDirPath));
    }
    return success;
}
