#pragma once

#include <QString>
#include <QObject>

#include <functional>
#include <memory>

class QLocalServer;

class LauncherListener final : public QObject
{
    Q_OBJECT
public:
    LauncherListener();
    ~LauncherListener();

    QString GetErrorMessage() const;
    //this message will be send to a target application
    //user must know about this messages to enumerate them on the client side
    //this enums used on data level of the OSI model
    enum eMessage
    {
        QUIT
    };

    //this is reply to launcher
    enum eReply
    {
        ACCEPT,
        REJECT,
        UNKNOWN_MESSAGE
    };

    //client application receive messages by ProcessRequestFunction;
    //usage example:
    //SetProcessRequestFunction([](eMessage message) { return UNKNOWN_MESSAGE; };
    using ProcessRequestFunction = std::function<eReply(eMessage)>;
    bool Init(ProcessRequestFunction function);

private slots:
    void OnNewConnection();
    void ProcessTransportLevel();

private:
    int ProcessChannelLevel(const QByteArray& data);
    int ProcessDataLevel(const QByteArray& data);

    QString lastError;
    std::unique_ptr<QLocalServer> server;
    ProcessRequestFunction processRequest;
};
