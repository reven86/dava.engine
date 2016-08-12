#pragma once

// #include "defines.h"
// #include <yaml-cpp/yaml.h>
// #include <QString>
// #include <QMap>
// #include <QVector>
// #include <QSet>
//
// QString GetStringValueFromYamlNode(const YAML::Node* node, QString defaultValue = "");
// QStringList GetArrayValueFromYamlNode(const YAML::Node* node);

// class SharedDataParser
// {
// public:
//
//     SharedDataParser();
//     void Clear();
//     bool Parse(const QByteArray& data);
//     QByteArray Serialize();
//     void SaveToFile(const QString& filePath);
//
//     void InsertApplication(const QString& branchID, const QString& appID, const AppVersion& version);
//     void RemoveApplication(const QString& branchID, const QString& appID, const QString& version);
//
//     int GetBranchCount();
//     QString GetBranchID(int index);
//
//     Branch* GetBranch(int branchIndex);
//     Branch* GetBranch(const QString& branch);
//     Application* GetApplication(const QString& branch, const QString& appID);
//     AppVersion* GetAppVersion(const QString& branch, const QString& appID, const QString& ver);
//
//     void RemoveBranch(const QString& branchID);
//
//     QString GetString(const QString& stringID) const;
//     const QMap<QString, QString>& GetStrings() const;
//
//     void SetLauncherURL(const QString& url);
//     void SetWebpageURL(const QString& url);
//     void SetRemoteConfigURL(const QString& url);
//     void SetLastNewsID(const QString& id);
//
//     const QString& GetLauncherVersion() const;
//     const QString& GetLauncherURL() const;
//     const QString& GetWebpageURL() const;
//     const QString& GetNewsID() const;
//
//     const QStringList& GetFavorites();
//
//     void MergeBranchesIDs(QSet<QString>& branches);
//
//     void CopyStringsAndFavsFromConfig(const ConfigParser& parser);
//
//     void UpdateApplicationsNames();
//
// private:
//     bool ParseJSON(const QByteArray& configData);
//
//     QString launcherVersion;
//     QString launcherURL;
//     QString webPageURL;
//     QString remoteConfigURL;
//     QString newsID;
//
//     QStringList favorites;
//
//     QVector<Branch> branches;
//     QMap<QString, QString> strings;
// };

#include "ApplicationSettings.h"

namespace SharedDataParser
{
DAVA::List<SharedPoolParams> ParsePoolsReply(const QByteArray& data);
DAVA::List<SharedServerParams> ParseServersReply(const QByteArray& data);
}
