#include "Core/Tasks/LoadLocalConfigTask.h"
#include "Core/ApplicationManager.h"

#include "Gui/MainWindow.h"

#include "Data/ConfigParser.h"

#include <QFile>

LoadLocalConfigTask::LoadLocalConfigTask(ApplicationManager* appManager, const QString& localConfigPath_)
    : RunTask(appManager)
    , localConfigPath(localConfigPath_)
{
}

QString LoadLocalConfigTask::GetDescription() const
{
    return QObject::tr("Loading local config");
}

void LoadLocalConfigTask::Run()
{
    QFile configFile(localConfigPath);
    if (!configFile.exists())
    {
        return;
    }
    ConfigParser* localConfig = appManager->GetLocalConfig();
    localConfig->Clear();
    if (configFile.open(QFile::ReadOnly))
    {
        QByteArray data = configFile.readAll();
        configFile.close();
        if (data.isEmpty() == false)
        {
            localConfig->ParseJSON(data, this);
            localConfig->UpdateApplicationsNames();

            //kostil
            Branch* branch = localConfig->GetBranch("ST");
            if (branch != nullptr)
            {
                branch->id = "Stable";
            }
            branch = localConfig->GetBranch("BLITZTOSTABLE");
            if (branch != nullptr)
            {
                branch->id = "Blitz To Stable";
            }

            appManager->GetMainWindow()->RefreshApps();
        }
    }
    else
    {
        SetError(QObject::tr("Can not load local config: file %1 is inacessible").arg(localConfigPath));
    }
}
