#include "Core/Tasks/UpdateConfigTask.h"
#include "Core/Tasks/DownloadTask.h"

#include "Core/ApplicationManager.h"

#include "Gui/MainWindow.h"

UpdateConfigTask::UpdateConfigTask(ApplicationManager* appManager, const std::vector<QUrl>& urls_)
    : AsyncChainTask(appManager)
    , urls(urls_)
{
}

QString UpdateConfigTask::GetDescription() const
{
    return QObject::tr("Updating configuration");
}

void UpdateConfigTask::Run()
{
    appManager->GetRemoteConfig()->Clear();
    QString description = QObject::tr("Downloading configuration");
    std::unique_ptr<BaseTask> task = appManager->CreateTask<DownloadTask>(description, urls);
    task->SetOnFinishCallback(std::bind(&UpdateConfigTask::OnConfigLoaded, this, std::placeholders::_1));
    appManager->AddTaskWithBaseReceivers(std::move(task));
}

void UpdateConfigTask::OnConfigLoaded(const BaseTask* task)
{
    if (task->HasError())
    {
        appManager->GetMainWindow()->RefreshApps();
    }
    else
    {
        ConfigParser* remoteConfig = appManager->GetRemoteConfig();
        ConfigParser* localConfig = appManager->GetLocalConfig();
        Q_ASSERT(task->GetTaskType() == BaseTask::DOWNLOAD_TASK);

        const DownloadTask* downloadTask = static_cast<const DownloadTask*>(task);
        for (const QByteArray& data : downloadTask->GetLoadedData())
        {
            //old code, need to be refactored
            if (remoteConfig->ParseJSON(data, this))
            {
                QString webPageUrl = remoteConfig->GetWebpageURL();
                if (webPageUrl.isEmpty() == false)
                {
                    localConfig->SetWebpageURL(webPageUrl);
                }
                localConfig->CopyStringsAndFavsFromConfig(remoteConfig);
                appManager->SaveLocalConfig();
            }
        }
        //old code, need to be refactored
        appManager->GetRemoteConfig()->UpdateApplicationsNames();
        appManager->GetLocalConfig()->UpdateApplicationsNames();
        appManager->SaveLocalConfig();
        appManager->GetMainWindow()->RefreshApps();
        appManager->CheckUpdates();
    }

    emit Finished();
}
