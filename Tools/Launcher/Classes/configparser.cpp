#include "configparser.h"
#include "errormessenger.h"
#include "defines.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QDebug>
#include <QJsonParseError>
#include <QRegularExpression>

namespace ConfigParser_local
{

bool GetLauncherVersionAndURL(const QJsonValue& value, QString& version, QString& url, QString& news)
{
    QJsonObject launcherObject = value.toObject();
    QJsonObject platformObject = launcherObject[platformString].toObject();
    version = platformObject["version"].toString();
    url = platformObject["url"].toString();
    news = launcherObject["news"].toString();
    return !version.isEmpty() && !url.isEmpty() && !news.isEmpty();
}

bool GetLauncherStrings(const QJsonValue& value, QMap<QString, QString>& strings)
{
    QJsonArray array = value.toArray();
    bool isValid = !array.isEmpty();
    for (const QJsonValueRef& ref : array)
    {
        QJsonObject entry = ref.toObject();
        if (entry["os"].toString() != platformString)
        {
            continue;
        }
        QString key = entry["build_tag"].toString();
        QString stringValue = entry["build_name"].toString();
        isValid &= !key.isEmpty() && !stringValue.isEmpty();
        strings[key] = stringValue;
    }
    return isValid;
}

bool GetFavorites(const QJsonValue& value, QStringList& favorites)
{
    QJsonArray array = value.toArray();
    bool isValid = !array.isEmpty();
    for (const QJsonValueRef& ref : array)
    {
        QJsonObject entry = ref.toObject();
        if (entry["favourite"].toString() != "1")
        {
            continue;
        }
        QString fave = entry["branch_name"].toString();
        isValid &= !fave.isEmpty();
        //favorites list can not be large
        if (!favorites.contains(fave))
        {
            favorites.append(fave);
        }
    }

    return isValid;
}

QString ProcessID(const QString& id)
{
    QRegularExpressionMatch match;
    QRegularExpression regex("\\[\\d+\\_\\d+\\_\\d+\\]");
    int index = id.indexOf(regex, 0, &match);
    if (index == -1)
    {
        int digitIndex = id.indexOf(QRegularExpression("\\d+"));
        if (digitIndex == -1)
        {
            return id;
        }
        return id.right(id.length() - digitIndex);
    }
    QString version = match.captured();
    int versionLength = version.length();
    QStringList digits = version.split(QRegularExpression("\\D+"), QString::SkipEmptyParts);
    version = digits.join(".");
    QString dateTime = id.right(id.length() - versionLength - index);
    QRegularExpression timeRegex("\\_\\d+\\_\\d+\\_\\d+");
    if (dateTime.indexOf(timeRegex, 0, &match) != -1)
    {
        QString time = match.captured();
        time = time.split(QRegularExpression("\\D+"), QString::SkipEmptyParts).join(".");
        dateTime.replace(timeRegex, "_" + time);
    }
    QString result = version + dateTime;
    return result;
}

bool GetBranches(const QJsonValue& value, QVector<Branch>& branches)
{
    QJsonArray array = value.toArray();
    bool isValid = !array.isEmpty();
    for (const QJsonValueRef& ref : array)
    {
        QJsonObject entry = ref.toObject();
        QString branchNameID = "branchName";
        //now ASK builds without branch name
        if (!entry[branchNameID].isString())
        {
            isValid = false;
            continue;
        }
        QString branchName = entry[branchNameID].toString();

        Branch* branch = nullptr;
        //foreach will cause deeo copy in this case
        int branchCount = branches.size();
        for (int i = 0; i < branchCount; ++i)
        {
            if (branches[i].id == branchName)
            {
                branch = &branches[i];
            }
        }
        if (branch == nullptr)
        {
            branches.append(Branch(branchName));
            branch = &branches.last();
        }

        QString appName = entry["build_name"].toString();
        if (appName.isEmpty())
        {
            isValid = false;
            continue;
        }
        Application* app = branch->GetApplication(appName);
        if (app == nullptr)
        {
            branch->applications.append(Application(appName));
            app = &branch->applications.last();
        }
        QString verID = entry["build_type"].toString();
        if (verID.isEmpty())
        {
            isValid = false;
            continue;
        }
        AppVersion* appVer = app->GetVersion(verID);
        if (appVer == nullptr)
        {
            app->versions.append(AppVersion());
            appVer = &app->versions.last();
        }

        appVer->id = ProcessID(verID);
        appVer->url = entry["artifacts"].toString();
        appVer->runPath = entry["exe_location"].toString();
        appVer->buildNum = entry["build_num"].toString();
        isValid &= (!appVer->url.isEmpty() && !appVer->runPath.isEmpty());
    }
    //hotfix to sort downloaded items without rewriting mainWindow
    for (auto branchIter = branches.begin(); branchIter != branches.end(); ++branchIter)
    {
        QVector<Application>& apps = branchIter->applications;
        qSort(apps.begin(), apps.end(), [](const Application& appLeft, const Application& appRight) { return appLeft.id < appRight.id; });
    }

    return isValid;
}
}

AppVersion AppVersion::LoadFromYamlNode(const YAML::Node* node)
{
    AppVersion version;
    version.runPath = GetStringValueFromYamlNode(node->FindValue(CONFIG_APPVERSION_RUNPATH_KEY));
    version.cmd = GetStringValueFromYamlNode(node->FindValue(CONFIG_APPVERSION_CMD_KEY));
    version.url = GetStringValueFromYamlNode(node->FindValue(CONFIG_URL_KEY));

    return version;
}

Application Application::LoadFromYamlNode(const YAML::Node* node)
{
    Application app;
    YAML::Iterator it = node->begin();
    while (it != node->end())
    {
        AppVersion version = AppVersion::LoadFromYamlNode(&it.second());
        version.id = GetStringValueFromYamlNode(&it.first());
        app.versions.push_back(version);
        ++it;
    }

    return app;
}

AppVersion* Application::GetVersion(const QString& versionID)
{
    int versCount = versions.size();
    for (int i = 0; i < versCount; ++i)
        if (versions[i].id == versionID)
            return &versions[i];

    return 0;
}

void Application::RemoveVersion(const QString& versionID)
{
    int index = -1;
    int versCount = versions.size();
    for (int i = 0; i < versCount; ++i)
    {
        if (versions[i].id == versionID)
        {
            index = i;
            break;
        }
    }
    if (index != -1)
        versions.remove(index);
}

Application* Branch::GetApplication(const QString& appID)
{
    int appCount = applications.size();
    for (int i = 0; i < appCount; ++i)
        if (applications[i].id == appID)
            return &applications[i];

    return 0;
}

void Branch::RemoveApplication(const QString& appID)
{
    int index = -1;
    int appCount = applications.size();
    for (int i = 0; i < appCount; ++i)
    {
        if (applications[i].id == appID)
        {
            index = i;
            break;
        }
    }
    if (index != -1)
        applications.remove(index);
}

Branch Branch::LoadFromYamlNode(const YAML::Node* node)
{
    Branch branch;
    YAML::Iterator it = node->begin();
    while (it != node->end())
    {
        Application app = Application::LoadFromYamlNode(&it.second());
        app.id = GetStringValueFromYamlNode(&it.first());
        branch.applications.push_back(app);
        ++it;
    }

    return branch;
}

ConfigParser::ConfigParser()
    : launcherVersion(LAUNCHER_VER)
    , webPageURL("")
    , remoteConfigURL("")
    , newsID("0")
{
}

void ConfigParser::Clear()
{
    launcherVersion.clear();
    launcherURL.clear();
    webPageURL.clear();
    remoteConfigURL.clear();
    newsID.clear();

    favorites.clear();

    branches.clear();
    strings.clear();
}

bool ConfigParser::ParseJSON(const QByteArray& configData)
{
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(configData, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        //this is not JSON
        return false;
    }
    QJsonObject rootObj = document.object();
    for (const QString& key : rootObj.keys())
    {
        QJsonValue value = rootObj.value(key);
        if (key == "launcher")
        {
            if (!ConfigParser_local::GetLauncherVersionAndURL(value, launcherVersion, launcherURL, webPageURL))
            {
                ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_CONFIG, QObject::tr("wrong launcher version object"));
                continue;
            }
        }
        else if (key == "seo_list")
        {
            if (!ConfigParser_local::GetLauncherStrings(value, strings))
            {
                ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_CONFIG, QObject::tr("wrong seo strings object"));
                continue;
            }
        }
        else if (key == "branches")
        {
            if (!ConfigParser_local::GetFavorites(value, favorites))
            {
                ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_CONFIG, QObject::tr("error while reading favorites list"));
            }
        }
        else if (key == "builds")
        {
            if (!ConfigParser_local::GetBranches(value, branches))
            {
                ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_CONFIG, QObject::tr("error while reading branches list"));
            }
        }
    }
    return true;
}

bool ConfigParser::Parse(const QByteArray& configData)
{
    if (configData.isEmpty())
    {
        return false;
    }
    if (ParseJSON(configData))
    {
        return true;
    }
    YAML::Parser parser;

    YAML::Node configRoot;
    YAML::Iterator it;
    const YAML::Node* launcherNode;
    const YAML::Node* stringsNode;
    const YAML::Node* branchesNode;

    std::istringstream fileStream(configData.data());
    parser.Load(fileStream);

    try
    {
        if (parser.GetNextDocument(configRoot))
        {
            launcherNode = configRoot.FindValue(CONFIG_LAUNCHER_KEY);
            stringsNode = configRoot.FindValue(CONFIG_STRINGS_KEY);
            branchesNode = configRoot.FindValue(CONFIG_BRANCHES_KEY);

            launcherVersion = GetStringValueFromYamlNode(launcherNode->FindValue(CONFIG_LAUNCHER_VERSION_KEY), LAUNCHER_VER);
            launcherURL = GetStringValueFromYamlNode(launcherNode->FindValue(CONFIG_URL_KEY));
            webPageURL = GetStringValueFromYamlNode(launcherNode->FindValue(CONFIG_LAUNCHER_WEBPAGE_KEY));
            newsID = GetStringValueFromYamlNode(launcherNode->FindValue(CONFIG_LAUNCHER_NEWSID_KEY));
            favorites = GetArrayValueFromYamlNode(launcherNode->FindValue(CONFIG_LAUNCHER_FAVORITES_KEY));

            //hotfix to remove duplicates
            favorites = favorites.toSet().toList();
            if (stringsNode)
            {
                it = stringsNode->begin();
                while (it != stringsNode->end())
                {
                    QString key = GetStringValueFromYamlNode(&it.first());
                    QString value = GetStringValueFromYamlNode(&it.second());
                    strings[key] = value;
                    ++it;
                }
            }

            if (branchesNode)
            {
                it = branchesNode->begin();
                while (it != branchesNode->end())
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
            ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_CONFIG);
            return false;
        }
    }
    catch (YAML::ParserException& e)
    {
        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_CONFIG, -1, QString(e.msg.c_str()));
        return false;
    }
    return true;
}

void ConfigParser::CopyStringsAndFavsFromConfig(const ConfigParser& parser)
{
    QMap<QString, QString>::ConstIterator it = parser.strings.begin();
    QMap<QString, QString>::ConstIterator itEnd = parser.strings.end();
    for (; it != itEnd; ++it)
        strings[it.key()] = it.value();
    if (!parser.favorites.isEmpty())
    {
        favorites = parser.favorites;
    }
}

void ConfigParser::UpdateApplicationsNames()
{
    for (auto branchIter = branches.begin(); branchIter != branches.end(); ++branchIter)
    {
        for (auto appIter = branchIter->applications.begin(); appIter != branchIter->applications.end(); ++appIter)
        {
            auto stringsIter = strings.find(appIter->id);
            if (stringsIter != strings.end())
            {
                appIter->id = *stringsIter;
            }
        }
    }
}

QByteArray ConfigParser::Serialize()
{
    YAML::Emitter emitter;
    emitter.SetIndent(4);
    emitter << YAML::BeginMap;

    //Launcher info
    emitter << YAML::Key << CONFIG_LAUNCHER_KEY << YAML::Value << YAML::BeginMap;
    emitter << YAML::Key << CONFIG_LAUNCHER_WEBPAGE_KEY << YAML::Value << webPageURL.toStdString();
    emitter << YAML::Key << CONFIG_LAUNCHER_NEWSID_KEY << YAML::Value << newsID.toStdString();

    int favCount = favorites.size();
    if (favCount)
    {
        emitter << YAML::Key << CONFIG_LAUNCHER_FAVORITES_KEY << YAML::Value;
        emitter << YAML::BeginSeq;
        for (int i = 0; i < favCount; i++)
            emitter << favorites[i].toStdString();
        emitter << YAML::EndSeq;
    }

    emitter << YAML::EndMap;

    //Strings
    emitter << YAML::Key << CONFIG_STRINGS_KEY << YAML::Value << YAML::BeginMap;
    QMap<QString, QString>::Iterator it = strings.begin();
    QMap<QString, QString>::Iterator itEnd = strings.end();
    for (; it != itEnd; ++it)
        emitter << YAML::Key << it.key().toStdString() << YAML::Value << it.value().toStdString();
    emitter << YAML::EndMap;

    //Applications
    emitter << YAML::Key << CONFIG_BRANCHES_KEY << YAML::Value << YAML::BeginMap;
    for (int i = 0; i < branches.size(); ++i)
    {
        Branch* branch = GetBranch(i);
        emitter << YAML::Key << branch->id.toStdString() << YAML::Value << YAML::BeginMap;
        for (int j = 0; j < branch->GetAppCount(); ++j)
        {
            Application* app = branch->GetApplication(j);
            emitter << YAML::Key << app->id.toStdString() << YAML::Value << YAML::BeginMap;
            for (int k = 0; k < app->GetVerionsCount(); ++k)
            {
                AppVersion* ver = app->GetVersion(k);
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
    return emitter.c_str();
}

void ConfigParser::SaveToFile(const QString& filePath)
{
    QFile file(filePath);
    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QByteArray data = Serialize();
        file.write(data);
    }
    file.close();
}

void ConfigParser::RemoveBranch(const QString& branchID)
{
    int index = -1;
    int branchesCount = branches.size();
    for (int i = 0; i < branchesCount; ++i)
    {
        if (branches[i].id == branchID)
        {
            index = i;
            break;
        }
    }
    if (index != -1)
        branches.remove(index);
}

void ConfigParser::InsertApplication(const QString& branchID, const QString& appID, const AppVersion& version)
{
    Branch* branch = GetBranch(branchID);
    if (!branch)
    {
        branches.push_back(Branch(branchID));
        branch = GetBranch(branchID);
    }

    Application* app = branch->GetApplication(appID);
    if (!app)
    {
        branch->applications.push_back(Application(appID));
        app = branch->GetApplication(appID);
    }

    app->versions.clear();
    app->versions.push_back(version);
}

void ConfigParser::RemoveApplication(const QString& branchID, const QString& appID, const QString& versionID)
{
    Branch* branch = GetBranch(branchID);
    if (!branch)
        return;

    Application* app = branch->GetApplication(appID);
    if (!app)
        return;

    AppVersion* appVersion = app->GetVersion(versionID);
    if (!appVersion)
        return;

    app->RemoveVersion(versionID);
    if (!app->GetVerionsCount())
        branch->RemoveApplication(appID);

    if (!branch->GetAppCount())
        RemoveBranch(branchID);
}

int ConfigParser::GetBranchCount()
{
    return branches.size();
}

QString ConfigParser::GetBranchID(int branchIndex)
{
    if (branchIndex >= 0 && branchIndex < branches.size())
        return branches[branchIndex].id;

    return QString();
}

Branch* ConfigParser::GetBranch(int branchIndex)
{
    if (branchIndex >= 0 && branchIndex < branches.size())
        return &branches[branchIndex];

    return 0;
}

Branch* ConfigParser::GetBranch(const QString& branch)
{
    int branchCount = branches.size();
    for (int i = 0; i < branchCount; ++i)
        if (branches[i].id == branch)
            return &branches[i];

    return 0;
}

Application* ConfigParser::GetApplication(const QString& branchID, const QString& appID)
{
    Branch* branch = GetBranch(branchID);
    if (branch)
    {
        int appCount = branch->applications.size();
        for (int i = 0; i < appCount; ++i)
            if (branch->applications[i].id == appID)
                return &branch->applications[i];
    }

    return 0;
}

AppVersion* ConfigParser::GetAppVersion(const QString& branchID, const QString& appID, const QString& ver)
{
    Application* app = GetApplication(branchID, appID);
    if (app)
    {
        int versCount = app->versions.size();
        for (int i = 0; i < versCount; ++i)
            if (app->versions[i].id == ver)
                return &app->versions[i];
    }

    return 0;
}

QString ConfigParser::GetString(const QString& stringID) const
{
    if (strings.contains(stringID))
        return strings[stringID];

    return stringID;
}

const QString& ConfigParser::GetLauncherVersion() const
{
    return launcherVersion;
}

const QString& ConfigParser::GetLauncherURL() const
{
    return launcherURL;
}

const QString& ConfigParser::GetWebpageURL() const
{
    return webPageURL;
}

const QString& ConfigParser::GetNewsID() const
{
    return newsID;
}

void ConfigParser::SetLauncherURL(const QString& url)
{
    launcherURL = url;
}

void ConfigParser::SetWebpageURL(const QString& url)
{
    webPageURL = url;
}

void ConfigParser::SetRemoteConfigURL(const QString& url)
{
    remoteConfigURL = url;
}

void ConfigParser::MergeBranchesIDs(QSet<QString>& branchIDs)
{
    int branchCount = branches.size();
    for (int i = 0; i < branchCount; ++i)
        branchIDs.insert(branches[i].id);
}

void ConfigParser::SetLastNewsID(const QString& id)
{
    newsID = id;
}

const QStringList& ConfigParser::GetFavorites()
{
    return favorites;
}

QString GetStringValueFromYamlNode(const YAML::Node* node, QString defaultValue /* "" */)
{
    if (!node)
        return defaultValue;

    std::string stdStr;
    node->GetScalar(stdStr);
    return QString(stdStr.c_str());
}

QStringList GetArrayValueFromYamlNode(const YAML::Node* node)
{
    QStringList array;

    if (!node)
        return array;

    if (node->Type() == YAML::NodeType::Scalar)
        array.push_back(GetStringValueFromYamlNode(node));

    if (node->Type() == YAML::NodeType::Sequence)
    {
        YAML::Iterator it = node->begin();
        while (it != node->end())
        {
            array.push_back(GetStringValueFromYamlNode(&(*it)));
            ++it;
        }
    }
    return array;
}
