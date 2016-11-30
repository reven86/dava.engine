#pragma once

#include <QObject>
#include <QSharedMemory>
#include <QElapsedTimer>
#include <functional>

class QTimer;

class ProcessCommunication : public QObject
{
    Q_OBJECT

public:
    //init communication transport layer from QApplication, using application runPath and application name
    ProcessCommunication(QObject* parent = nullptr);
    ~ProcessCommunication();
    
    //this message will be send to a target application
    enum class eMessage
    {
        //messages
        QUIT,
        USER_MESSAGE = 0xFFFF,
    };

    //reply to a sender
    //can be sent from ProcessCommunication module
    enum class eReply
    {
        ACCEPT = 0xF0000,
        REJECT,
        UNKNOWN_MESSAGE,
        
        //this replies used by ProcessCommunication module
        //don't use it, please
        NOT_INITIALIZED,
        NOT_RUNNING,
        NOT_EXISTS,
        SEND_ERROR,
        TIMEOUT_ERROR,
        USER_REPLY = 0xFFFFF
    };

    bool IsInitialized() const;

    //converts enum value to a string value
    static QString GetReplyString(eReply reply);

    using CallbackFunction = std::function<void(eReply)>;
    void SendAsync(const eMessage messageCode, const QString &targetAppPath, CallbackFunction callBack = CallbackFunction());
    eReply SendSync(const eMessage messagCode, const QString &targetAppPath);

    //client application receive messages by ProcessRequestFunction;
    //usage example:
    //SetProcessRequestFunction([](eMessage message) { return UNKNOWN_MESSAGE; };
    using ProcessRequestFunction = std::function<eReply(eMessage)>;
    void SetProcessRequestFunction(ProcessRequestFunction function);

private slots:
    void Poll();

private:
    void Reply(qint64 transportLevelID, const eReply replyCode, const QString &targetAppPath);

    void InitPollTimer();
    bool Lock();
    bool Unlock();

    bool Write(const QByteArray &data);
    QJsonObject Read();
    bool Flush();

    QSharedMemory sharedMemory;
    QTimer *pollTimer;
    struct MessageDetails
    {
        MessageDetails(CallbackFunction callBack);
        qint64 transportMessageID = -1;
        qint64 creationTime = -1;
        CallbackFunction callBack;
        static qint64 lastMessageID;
    };
    QList<MessageDetails> sentMessages;
    QElapsedTimer elapsedTimer;
    ProcessRequestFunction processRequest;
    bool initialized = false;
};
