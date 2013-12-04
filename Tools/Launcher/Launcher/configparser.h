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


#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include "defines.h"
#include <yaml-cpp/yaml.h>
#include <QString>
#include <QMap>
#include <QVector>
#include <QSet>

QString GetStringValueFromYamlNode(const YAML::Node * node, QString defaultValue = "");
QVector<QString> GetArrayValueFromYamlNode(const YAML::Node * node);

class ConfigParser;

struct AppVersion
{
    QString id;
    QString runPath;
    QString cmd;
    QString url;

    static AppVersion LoadFromYamlNode(const YAML::Node * node);
};

struct Application
{
    Application() {}
    Application(const QString & _id) : id(_id) {}

    QString id;

    int GetVerionsCount() {return versions.size();}
    AppVersion * GetVersion(int index) {return &versions[index];}
    AppVersion * GetVersion(const QString & versionID);

    void RemoveVersion(const QString & versionID);

    static Application LoadFromYamlNode(const YAML::Node * node);

private:
    QVector<AppVersion> versions;

    friend class ConfigParser;
};

struct Branch
{
    Branch() {}
    Branch(const QString & _id) : id(_id) {}

    QString id;

    int GetAppCount() {return applications.size();}
    Application * GetApplication(int index) {return &applications[index];}
    Application * GetApplication(const QString & appID);

    void RemoveApplication(const QString & appID);

    static Branch LoadFromYamlNode(const YAML::Node * node);

private:
    QVector<Application> applications;

    friend class ConfigParser;
};

class ConfigParser
{
public:
    ConfigParser(const QByteArray & configData);

    void SaveToYamlFile(const QString & filePath);

    void InsertApplication(const QString & branchID, const QString & appID, const AppVersion & version);
    void RemoveApplication(const QString & branchID, const QString & appID, const QString & version);

    int GetBranchCount();
    QString GetBranchID(int index);

    Branch * GetBranch(int branchIndex);
    Branch * GetBranch(const QString &branch);
    Application * GetApplication(const QString &branch, const QString &appID);
    AppVersion * GetAppVersion(const QString &branch, const QString &appID, const QString &ver);

    void RemoveBranch(const QString & branchID);

    const QString & GetString(const QString & stringID);

    void SetLauncherURL(const QString & url);
    void SetWebpageURL(const QString & url);
    void SetRemoteConfigURL(const QString & url);
    void SetLastNewsID(const QString & id);

    const QString & GetLauncherVersion();
    const QString & GetLauncherURL();
    const QString & GetWebpageURL();
    const QString & GetRemoteConfigURL();
    const QString & GetNewsID();

    const QVector<QString> & GetFavorites();

    void MergeBranchesIDs(QSet<QString> & branches);

    void CopyStringsAndFavsFromConfig(const ConfigParser & parser);

private:
    QString launcherVersion;
    QString launcherURL;
    QString webPageURL;
    QString remoteConfigURL;
    QString newsID;

    QVector<QString> favorites;

    QVector<Branch> branches;
    QMap<QString, QString> strings;
};

#endif // CONFIGPARSER_H
