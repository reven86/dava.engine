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
    
    enum class eMessage
    {
        //messages
        QUIT,

        USER_MESSAGE = 0xFFFF,
    };

    enum class eReply
    {
        DONE = 0xF0000,
        NOT_INITIALIZED,
        USER_REPLY = 0xFFFFF
    };

    //async method
    using CallbackFunction = std::function<void(eReply)>;
    void Send(const eMessage messageCode, const QString &targetAppPath, CallbackFunction callBack = CallbackFunction());

    //this class require client function to process requests
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

    QSharedMemory sharedMemory;
    QTimer *pollTimer;
    struct MessageDetails
    {
        //c-tor for send
        MessageDetails(CallbackFunction callBack);
        qint64 transportMessageID = -1;
        qint64 creationTime = -1;
        CallbackFunction callBack;
    };
    QList<MessageDetails> sentMessages;
    QElapsedTimer elapsedTimer;
    ProcessRequestFunction processRequest;
};


