/*==================================================================================
 Copyright (c) 2008, binaryzebra
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the binaryzebra nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/


#include "configstorage.h"
#include <QFile>
#include <QMessageBox>
#include <QApplication>

ConfigStorage::ConfigStorage(QObject* parent)
    : QObject(parent)
{
#ifdef Q_OS_WIN
    configFilePath = "../CMakeTool/Data/config_windows.txt";
#elif defined Q_OS_MAC
    configFilePath = qApp->applicationDirPath() + "/../Resources/Data/config_mac.txt";
#else
    qCritical() << "application started on undefined platform";
    return 1;
#endif //platform
}

QString ConfigStorage::GetJSONTextFromConfigFile() const
{
    if (!QFile::exists(configFilePath))
    {
        QMessageBox::critical(nullptr, QObject::tr("Config file not available!"), QObject::tr("Can not find config file %1").arg(configFilePath));
        exit(0);
    }
    QFile configFile(configFilePath);
    if (configFile.open(QIODevice::ReadOnly))
    {
        return configFile.readAll();
    }
    else
    {
        QMessageBox::critical(nullptr, QObject::tr("Failed to open config file!"), QObject::tr("Failed to open config file %1").arg(configFilePath));
        exit(0);
    }
    return QString();
}

void ConfigStorage::SaveToConfigFile(const QString& config)
{
    QFile configFile(configFilePath);
    if (configFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        configFile.write(config.toUtf8());
    }
    else
    {
        auto button = QMessageBox::question(nullptr, tr("Can't open file"), tr("Can not open config file %1\n. do you want to repeat?").arg(configFilePath));
        if (button == QMessageBox::Yes)
        {
            SaveToConfigFile(config);
        }
    }
}
