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


#include "configparser.h"
#include "errormessanger.h"
#include <QFile>

AppVersion AppVersion::LoadFromYamlNode(const YAML::Node * node)
{
    AppVersion version;
    version.runPath = GetStringValueFromYamlNode(node->FindValue(CONFIG_APPVERSION_RUNPATH_KEY));
    version.cmd = GetStringValueFromYamlNode(node->FindValue(CONFIG_APPVERSION_CMD_KEY));
    version.url = GetStringValueFromYamlNode(node->FindValue(CONFIG_URL_KEY));

    return version;
}

Application Application::LoadFromYamlNode(const YAML::Node * node)
{
    Application app;
    YAML::Iterator it = node->begin();
    while(it != node->end())
    {
        AppVersion version = AppVersion::LoadFromYamlNode(&it.second());
        version.id = GetStringValueFromYamlNode(&it.first());
        app.versions.push_back(version);
        ++it;
    }

    return app;
}

AppVersion * Application::GetVersion(const QString & versionID)
{
    int versCount = versions.size();
    for(int i = 0; i < versCount; ++i)
        if(versions[i].id == versionID)
            return &versions[i];

    return 0;
}

void Application::RemoveVersion(const QString &versionID)
{
    int index = -1;
    int versCount = versions.size();
    for(int i = 0; i < versCount; ++i)
    {
        if(versions[i].id == versionID)
        {
            index = i;
            break;
        }
    }
    if(index != -1)
        versions.remove(index);
}

Application * Branch::GetApplication(const QString & appID)
{
    int appCount = applications.size();
    for(int i = 0; i < appCount; ++i)
        if(applications[i].id == appID)
            return &applications[i];

    return 0;
}

void Branch::RemoveApplication(const QString &appID)
{
    int index = -1;
    int appCount = applications.size();
    for(int i = 0; i < appCount; ++i)
    {
        if(applications[i].id == appID)
        {
            index = i;
            break;
        }
    }
    if(index != -1)
        applications.remove(index);
}

Branch Branch::LoadFromYamlNode(const YAML::Node * node)
{
    Branch branch;
    YAML::Iterator it = node->begin();
    while(it != node->end())
    {
        Application app = Application::LoadFromYamlNode(&it.second());
        app.id = GetStringValueFromYamlNode(&it.first());
        branch.applications.push_back(app);
        ++it;
    }

    return branch;
}

ConfigParser::ConfigParser(const QByteArray & configData) :
    launcherVersion(LAUNCHER_VER),
    webPageURL(""),
    remoteConfigURL(""),
    newsID("0")
{
    if(configData.size())
    {
        YAML::Parser parser;

        YAML::Node configRoot;
        YAML::Iterator it;
        const YAML::Node * launcherNode;
        const YAML::Node * stringsNode;
        const YAML::Node * branchesNode;

        std::istringstream fileStream(configData.data());
        parser.Load(fileStream);

        try
        {
            if(parser.GetNextDocument(configRoot))
            {
                launcherNode = configRoot.FindValue(CONFIG_LAUNCHER_KEY);
                stringsNode = configRoot.FindValue(CONFIG_STRINGS_KEY);
                branchesNode = configRoot.FindValue(CONFIG_BRANCHES_KEY);

                launcherVersion = GetStringValueFromYamlNode(launcherNode->FindValue(CONFIG_LAUNCHER_VERSION_KEY), LAUNCHER_VER);
                launcherURL = GetStringValueFromYamlNode(launcherNode->FindValue(CONFIG_URL_KEY));
                webPageURL = GetStringValueFromYamlNode(launcherNode->FindValue(CONFIG_LAUNCHER_WEBPAGE_KEY));
                remoteConfigURL = GetStringValueFromYamlNode(launcherNode->FindValue(CONFIG_LAUNCHER_REMOTE_URL_KEY));
                newsID = GetStringValueFromYamlNode(launcherNode->FindValue(CONFIG_LAUNCHER_NEWSID_KEY));
                favorites = GetArrayValueFromYamlNode(launcherNode->FindValue(CONFIG_LAUNCHER_FAVORITES_KEY));

                if(stringsNode)
                {
                    it = stringsNode->begin();
                    while(it != stringsNode->end())
                    {
                        QString key = GetStringValueFromYamlNode(&it.first());
                        QString value = GetStringValueFromYamlNode(&it.second());
                        strings[key] = value;
                        ++it;
                    }
                }

                if(branchesNode)
                {
                    it = branchesNode->begin();
                    while(it != branchesNode->end())
                    {
                        Branch branch = Branch::LoadFromYamlNode(&it.second());
                        branch.id = GetStringValueFromYamlNode(&it.first());
                        branches.push_back(branch);
                        ++it;
                    }
                }
            }
            else
            {
                ErrorMessanger::Instance()->ShowErrorMessage(ErrorMessanger::ERROR_CONFIG);
            }
        }
        catch(YAML::ParserException& e)
        {
            ErrorMessanger::Instance()->ShowErrorMessage(ErrorMessanger::ERROR_CONFIG, -1, QString(e.msg.c_str()));
        }
    }
}

void ConfigParser::CopyStringsAndFavsFromConfig(const ConfigParser & parser)
{
    QMap<QString, QString>::ConstIterator it = parser.strings.begin();
    QMap<QString, QString>::ConstIterator itEnd = parser.strings.end();
    for(; it != itEnd; ++it)
        strings[it.key()] = it.value();

    favorites = parser.favorites;
}

void ConfigParser::SaveToYamlFile(const QString & filePath)
{
    YAML::Emitter emitter;
    emitter.SetIndent(4);
    emitter << YAML::BeginMap;

    //Launcher info
    emitter << YAML::Key << CONFIG_LAUNCHER_KEY << YAML::Value << YAML::BeginMap;
    emitter << YAML::Key << CONFIG_LAUNCHER_WEBPAGE_KEY << YAML::Value << webPageURL.toStdString();
    emitter << YAML::Key << CONFIG_LAUNCHER_REMOTE_URL_KEY << YAML::Value << remoteConfigURL.toStdString();
    emitter << YAML::Key << CONFIG_LAUNCHER_NEWSID_KEY << YAML::Value << newsID.toStdString();

    int favCount = favorites.size();
    if(favCount)
    {
        emitter << YAML::Key << CONFIG_LAUNCHER_FAVORITES_KEY << YAML::Value;
        emitter << YAML::BeginSeq;
        for(int i = 0; i < favCount; i++)
            emitter << favorites[i].toStdString();
        emitter << YAML::EndSeq;
    }

    emitter << YAML::EndMap;

    //Strings
    emitter << YAML::Key << CONFIG_STRINGS_KEY << YAML::Value << YAML::BeginMap;
    QMap<QString, QString>::Iterator it = strings.begin();
    QMap<QString, QString>::Iterator itEnd = strings.end();
    for(; it != itEnd; ++it)
        emitter << YAML::Key << it.key().toStdString() << YAML::Value << it.value().toStdString();
    emitter << YAML::EndMap;

    //Applications
    emitter << YAML::Key << CONFIG_BRANCHES_KEY << YAML::Value << YAML::BeginMap;
    for(int i = 0; i < branches.size(); ++i)
    {
        Branch * branch = GetBranch(i);
        emitter << YAML::Key << branch->id.toStdString() << YAML::Value << YAML::BeginMap;
        for(int j = 0; j < branch->GetAppCount(); ++j)
        {
            Application * app = branch->GetApplication(j);
            emitter << YAML::Key << app->id.toStdString() << YAML::Value << YAML::BeginMap;
            for(int k = 0; k < app->GetVerionsCount(); ++k)
            {
                AppVersion * ver = app->GetVersion(k);
                emitter << YAML::Key << ver->id.toStdString() << YAML::Value << YAML::BeginMap;
                emitter << YAML::Key << CONFIG_APPVERSION_RUNPATH_KEY << YAML::Value << ver->runPath.toStdString();
                emitter << YAML::Key << CONFIG_APPVERSION_CMD_KEY << YAML::Value << ver->cmd.toStdString();
                emitter << YAML::Key << CONFIG_URL_KEY << YAML::Value << ver->url.toStdString();
                emitter << YAML::EndMap;
            }
            emitter << YAML::EndMap;
        }
        emitter << YAML::EndMap;
    }
    emitter << YAML::EndMap;

    emitter << YAML::EndMap;

    QFile file(filePath);
    file.open(QFile::WriteOnly);
    file.write(emitter.c_str());
    file.close();
}

void ConfigParser::RemoveBranch(const QString & branchID)
{
    int index = -1;
    int branchesCount = branches.size();
    for(int i = 0; i < branchesCount; ++i)
    {
        if(branches[i].id == branchID)
        {
            index = i;
            break;
        }
    }
    if(index != -1)
        branches.remove(index);
}

void ConfigParser::InsertApplication(const QString & branchID, const QString & appID, const AppVersion & version)
{
    Branch * branch = GetBranch(branchID);
    if(!branch)
    {
        branches.push_back(Branch(branchID));
        branch = GetBranch(branchID);
    }

    Application * app = branch->GetApplication(appID);
    if(!app)
    {
        branch->applications.push_back(Application(appID));
        app = branch->GetApplication(appID);
    }

    app->versions.clear();
    app->versions.push_back(version);
}

void ConfigParser::RemoveApplication(const QString & branchID, const QString & appID, const QString & versionID)
{
    Branch * branch = GetBranch(branchID);
    if(!branch)
        return;

    Application * app = branch->GetApplication(appID);
    if(!app)
        return;

    AppVersion * appVersion = app->GetVersion(versionID);
    if(!appVersion)
        return;

    app->RemoveVersion(versionID);
    if(!app->GetVerionsCount())
        branch->RemoveApplication(appID);

    if(!branch->GetAppCount())
        RemoveBranch(branchID);
}

int ConfigParser::GetBranchCount()
{
    return branches.size();
}

QString ConfigParser::GetBranchID(int branchIndex)
{
    if(branchIndex >= 0 && branchIndex < branches.size())
        return branches[branchIndex].id;

    return QString();
}

Branch * ConfigParser::GetBranch(int branchIndex)
{
    if(branchIndex >= 0 && branchIndex < branches.size())
        return &branches[branchIndex];

    return 0;
}

Branch * ConfigParser::GetBranch(const QString &branch)
{
    int branchCount = branches.size();
    for(int i = 0; i < branchCount; ++i)
        if(branches[i].id == branch)
            return &branches[i];

    return 0;
}

Application * ConfigParser::GetApplication(const QString &branchID, const QString &appID)
{
    Branch * branch = GetBranch(branchID);
    if(branch)
    {
        int appCount = branch->applications.size();
        for(int i = 0; i < appCount; ++i)
            if(branch->applications[i].id == appID)
                return &branch->applications[i];
    }

    return 0;
}

AppVersion * ConfigParser::GetAppVersion(const QString &branchID, const QString &appID, const QString &ver)
{
    Application * app = GetApplication(branchID, appID);
    if(app)
    {
        int versCount = app->versions.size();
        for(int i = 0; i < versCount; ++i)
            if(app->versions[i].id == ver)
                return &app->versions[i];
    }

    return 0;
}

const QString & ConfigParser::GetString(const QString & stringID)
{
    if(strings.contains(stringID))
        return strings[stringID];

    return stringID;
}

const QString & ConfigParser::GetLauncherVersion()
{
    return launcherVersion;
}

const QString & ConfigParser::GetLauncherURL()
{
    return launcherURL;
}

const QString & ConfigParser::GetWebpageURL()
{
    return webPageURL;
}

const QString & ConfigParser::GetRemoteConfigURL()
{
    return remoteConfigURL;
}

const QString & ConfigParser::GetNewsID()
{
    return newsID;
}

void ConfigParser::SetLauncherURL(const QString & url)
{
    launcherURL = url;
}

void ConfigParser::SetWebpageURL(const QString & url)
{
    webPageURL = url;
}

void ConfigParser::SetRemoteConfigURL(const QString & url)
{
    remoteConfigURL = url;
}

void ConfigParser::MergeBranchesIDs(QSet<QString> & branchIDs)
{
    int branchCount = branches.size();
    for(int i = 0; i < branchCount; ++i)
        branchIDs.insert(branches[i].id);
}

void ConfigParser::SetLastNewsID(const QString & id)
{
    newsID = id;
}

const QVector<QString> & ConfigParser::GetFavorites()
{
    return favorites;
}

QString GetStringValueFromYamlNode(const YAML::Node * node, QString defaultValue /* "" */)
{
    if(!node)
        return defaultValue;

    std::string stdStr;
    node->GetScalar(stdStr);
    return QString(stdStr.c_str());
}

QVector<QString> GetArrayValueFromYamlNode(const YAML::Node * node)
{
    QVector<QString> array;

    if(!node)
        return array;

    if(node->Type() == YAML::NodeType::Scalar)
        array.push_back(GetStringValueFromYamlNode(node));

    if(node->Type() == YAML::NodeType::Sequence)
    {
        YAML::Iterator it = node->begin();
        while(it != node->end())
        {
            array.push_back(GetStringValueFromYamlNode(&(*it)));
            ++it;
        }
    }
    return array;
}
