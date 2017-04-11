#pragma once

#include "defines.h"
#include <QString>
#include <QMap>
#include <QVector>
#include <QSet>

struct AppVersion;
class QJsonObject;

bool FillAppFields(AppVersion* appVer, const QJsonObject& entry, bool toolset);
bool IsToolset(const QString& appName);

class ConfigParser;

struct AppVersion
{
    QString id;
    //can be empty
    QString runPath;
    QString cmd;
    QString url;
    QString buildNum;
    bool isToolSet = false;
};

struct Application
{
    Application()
    {
    }
    Application(const QString& id_)
        : id(id_)
    {
    }

    //in a fact it is an ID to be displayed in UI
    // old name was not changed to save capability with other code
    QString id;

    int GetVerionsCount() const
    {
        return versions.size();
    }
    AppVersion* GetVersion(int index)
    {
        return &versions[index];
    }

    const AppVersion* GetVersion(int index) const
    {
        return &versions[index];
    }

    AppVersion* GetVersion(const QString& versionID);
    AppVersion* GetVersionByNum(const QString& num);
    void RemoveVersion(const QString& versionID);

    QVector<AppVersion> versions;
};

struct Branch
{
    Branch()
    {
    }
    Branch(const QString& _id)
        : id(_id)
    {
    }

    QString id;

    int GetAppCount() const
    {
        return applications.size();
    }
    Application* GetApplication(int index)
    {
        return &applications[index];
    }

    const Application* GetApplication(int index) const
    {
        return &applications[index];
    }
    Application* GetApplication(const QString& appID);

    void RemoveApplication(const QString& appID);

    QVector<Application> applications;
};

class ConfigParser
{
public:
    ConfigParser();
    void Clear();
    bool Parse(const QByteArray& data);
    QByteArray Serialize() const;
    void SaveToFile(const QString& filePath) const;

    void InsertApplication(const QString& branchID, const QString& appID, const AppVersion& version);

    void RemoveApplication(const QString& branchID, const QString& appID, const QString& version);

    static QStringList GetToolsetApplications();
    QStringList GetTranslatedToolsetApplications() const;
    int GetBranchCount();
    QString GetBranchID(int index);

    Branch* GetBranch(int branchIndex);
    const Branch* GetBranch(int branchIndex) const;

    Branch* GetBranch(const QString& branch);
    const Branch* GetBranch(const QString& branch) const;

    const Application* GetApplication(const QString& branch, const QString& appID) const;
    Application* GetApplication(const QString& branch, const QString& appID);

    AppVersion* GetAppVersion(const QString& branch, const QString& appID, const QString& ver);

    void RemoveBranch(const QString& branchID);

    QString GetString(const QString& stringID) const;
    const QMap<QString, QString>& GetStrings() const;

    void SetLauncherURL(const QString& url);
    void SetWebpageURL(const QString& url);
    void SetRemoteConfigURL(const QString& url);
    void SetLastNewsID(const QString& id);

    const QString& GetLauncherVersion() const;
    const QString& GetLauncherURL() const;
    const QString& GetWebpageURL() const;
    const QString& GetNewsID() const;

    const QStringList& GetFavorites();

    void MergeBranchesIDs(QSet<QString>& branches);

    void CopyStringsAndFavsFromConfig(const ConfigParser& parser);

    void UpdateApplicationsNames();

private:
    void InsertApplicationImpl(const QString& branchID, const QString& appID, const AppVersion& version);

    bool ParseJSON(const QByteArray& configData);

    QString launcherVersion;
    QString launcherURL;
    QString webPageURL;
    QString remoteConfigURL;
    QString newsID;

    QStringList favorites;

    QVector<Branch> branches;
    QMap<QString, QString> strings;
};
