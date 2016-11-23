#include "QtHelpers/ProcessCommunication.h"
#include "QtHelpers/ProcessHelper.h"

#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QApplication>
#include <QThread>
#include <QFile>
#include <QDebug>

namespace ProcessCommunicationDetails
{
    //this is the unique key. Do not copy it please.
    //It used as an unique ID of this module.
    QString uniqueKey = "MA6#_1YB`zQSfXu4WWHj;C8o41,1c!+ETuav6(b>K=t3+Ru)9p&Q)`W/ZN9[Ei!";
    const qint64 maxDataSize = 10 * 1024; // i think that enough to send application path and console request
    const QString keyClientMessageID = "messageCode";
    const QString keyTargetApp = "targetApp";
    const QString keySenderApp = "senderApp";
    const QString keyTransportMessageID = "messageID";

    const qint64 pollingTime = 50; //100ms;
    const qint64 maximumTimeout = 5 * 60 * 1000; // 5 minutes
    static qint64 lastMessageID = 0;
}

ProcessCommunication::ProcessCommunication(QObject* parent)
    : QObject(parent)
    , sharedMemory(ProcessCommunicationDetails::uniqueKey)
{
    processRequest = [](eMessage) {
        return eReply::NOT_INITIALIZED;
    };
    elapsedTimer.start();
    QSharedMemory::AccessMode accessMode = QSharedMemory::ReadWrite;
    if (sharedMemory.create(ProcessCommunicationDetails::maxDataSize, accessMode) == false && sharedMemory.attach(accessMode) == false)
    {
        qCritical() << "Process communication module can not attach to shared memory, reason is" << sharedMemory.errorString();
    }
    else
    {
        InitPollTimer();
        initialized = true;
    }
}

ProcessCommunication::~ProcessCommunication()
{
    if (sharedMemory.detach())
    {
        qWarning() << "Process communication module can not detach from shared memory, reason is" << sharedMemory.errorString();
    }
}

QString ProcessCommunication::GetReplyString(eReply reply)
{
    switch (reply)
    {
    case eReply::ACCEPT:
        return tr("message accepted");
    case eReply::REJECT:
        return tr("message rejected");
    case eReply::UNKNOWN_MESSAGE:
        return tr("this message is unknown for client");
    case eReply::NOT_INITIALIZED:
        return tr("communication module was not initialized");
    case eReply::SEND_ERROR:
        return tr("can not send message");
    case eReply::TIMEOUT_ERROR:
        return tr("required application not responding");
    case eReply::NOT_EXISTS:
        return tr("required application is not exist");
    case eReply::NOT_RUNNING:
        return tr("required application is not running");
    default:
        return tr("unknown reply");
    }
}

void ProcessCommunication::SendAsync(const eMessage messageCode, const QString &targetAppPath, CallbackFunction callBack /* = CallbackFunction() */)
{
    if (IsInitialized() == false)
    {
        callBack(eReply::NOT_INITIALIZED);
        return;
    }
    if (QFile::exists(targetAppPath))
    {
        callBack(eReply::NOT_EXISTS);
        return;
    }
    if (ProcessHelper::IsProcessRuning(targetAppPath))
    {
        callBack(eReply::NOT_RUNNING);
        return;
    }
    QJsonObject obj;
    MessageDetails messageDetails(callBack);
    obj[ProcessCommunicationDetails::keyClientMessageID] = static_cast<int>(messageCode);
    obj[ProcessCommunicationDetails::keyTargetApp] = targetAppPath;
    obj[ProcessCommunicationDetails::keySenderApp] = qApp->applicationFilePath();
    obj[ProcessCommunicationDetails::keyTransportMessageID] = messageDetails.transportMessageID;

    QJsonDocument document(obj);
    QByteArray jsonData = document.toJson();
    jsonData.append('\0');
    if (Write(jsonData))
    {
        messageDetails.creationTime = elapsedTimer.elapsed();
        sentMessages.append(messageDetails);
    }
    else
    {
        callBack(eReply::SEND_ERROR);
    }
}

ProcessCommunication::eReply ProcessCommunication::SendSync(const eMessage messagCode, const QString &targetAppPath)
{
    bool haveAnswer = false;
    eReply reply;
    CallbackFunction callBack = [&haveAnswer, &reply](eReply replyFromClient){
        reply = replyFromClient;
        haveAnswer = true;
    };
    SendAsync(messagCode, targetAppPath, callBack);
    //whithout an answer this loop will be stopped by timeout
    while (haveAnswer == false)
    {
        QThread::msleep(100);
    }
    return reply;
}

void ProcessCommunication::SetProcessRequestFunction(ProcessRequestFunction function)
{
    processRequest = function;
}

bool ProcessCommunication::IsInitialized() const
{
    return initialized;
}

void ProcessCommunication::Poll()
{
    QJsonObject object = Read();
    if (object.isEmpty())
    {
        bool gotReply = false;

        int messageIDValue = object[ProcessCommunicationDetails::keyClientMessageID].toInt();

        qint64 transportLevelID = object[ProcessCommunicationDetails::keyTransportMessageID].toInt();
        QMutableListIterator<MessageDetails> iterator(sentMessages);
        while (iterator.hasNext())
        {
            const MessageDetails &details = iterator.next();
            if (details.transportMessageID == transportLevelID)
            {
                details.callBack(static_cast<eReply>(messageIDValue));
                iterator.remove();
                //we got reply for sent message
                gotReply = true;
            }
        }

        //this is a new direct message
        if (gotReply == false)
        {
            eMessage clientMessage = static_cast<eMessage>(messageIDValue);
            eReply reply = processRequest(clientMessage);
            QString targetApp = object[ProcessCommunicationDetails::keySenderApp].toString();
            Reply(transportLevelID, reply, targetApp);
        }
    }
    QMutableListIterator<MessageDetails> iterator(sentMessages);
    qint64 elapsedTime = elapsedTimer.elapsed();
    while (iterator.hasNext())
    {
        const MessageDetails &details = iterator.next();
        if (elapsedTime - details.creationTime > ProcessCommunicationDetails::maximumTimeout)
        {
            details.callBack(eReply::TIMEOUT_ERROR);
            iterator.remove();
        }
    }
}

void ProcessCommunication::Reply(qint64 transportLevelID, const eReply replyCode, const QString &targetAppPath)
{
    QJsonObject obj;
    obj[ProcessCommunicationDetails::keyClientMessageID] = static_cast<int>(replyCode);
    obj[ProcessCommunicationDetails::keyTargetApp] = targetAppPath;
    obj[ProcessCommunicationDetails::keySenderApp] = qApp->applicationFilePath();
    obj[ProcessCommunicationDetails::keyTransportMessageID] = transportLevelID;

    QJsonDocument document(obj);
    QByteArray jsonData = document.toJson();
    jsonData.append('\0');
    Write(jsonData);
}

void ProcessCommunication::InitPollTimer()
{
    pollTimer = new QTimer(this);
    pollTimer->setInterval(ProcessCommunicationDetails::pollingTime);
    pollTimer->setSingleShot(false);
    connect(pollTimer, &QTimer::timeout, this, &ProcessCommunication::Poll);
    pollTimer->start();
}

bool ProcessCommunication::Lock()
{
    bool locked = sharedMemory.lock();
    if (locked == false)
    {
        qWarning() << "Process communication module can not lock the shared memory. Last error is" << sharedMemory.errorString();
    }
    return locked;
}

bool ProcessCommunication::Unlock()
{
    bool unlocked = sharedMemory.unlock();
    if (unlocked == false)
    {
        qWarning() << "Process communication module can not unlock the shared memory. Last error is" << sharedMemory.errorString();
    }
    return unlocked;
}

bool ProcessCommunication::Write(const QByteArray &data)
{
    bool success = false;
    if (Lock() != false)
    {
        void* pureData = sharedMemory.data();
        if (pureData != nullptr)
        {

            if (data.size() > ProcessCommunicationDetails::maxDataSize)
            {
                qWarning() << "Process communication module can not send data biggest than max size";
            }
            else
            {
                memcpy(pureData, data.data(), data.size());
                success = true;
            }
        }
        else
        {
            qWarning() << "Process communication module can not write shared memory. Last error is" << sharedMemory.errorString();
        }
        Unlock();
    }
    return success;
}

QJsonObject ProcessCommunication::Read()
{
    QByteArray data;
    if (Lock() != false)
    {
        const void* pureData = sharedMemory.data();
        if (pureData != nullptr)
        {
            data += QByteArray(reinterpret_cast<const char*>(pureData), ProcessCommunicationDetails::maxDataSize);
            memset(const_cast<void*>(pureData), '\0', ProcessCommunicationDetails::maxDataSize);
        }
        else
        {
            qWarning() << "Process communication module can not read shared memory. Last error is" << sharedMemory.errorString();
        }
        Unlock();
    }
    if (data.indexOf('\0') == 0)
    {
        return QJsonObject();
    }
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(data.left(data.indexOf('\0')), &parseError);
    if (parseError.error == QJsonParseError::NoError && document.isObject())
    {
        QJsonObject obj = document.object();
        if (obj[ProcessCommunicationDetails::keyClientMessageID].isDouble() &&
            obj[ProcessCommunicationDetails::keySenderApp].isString() &&
            obj[ProcessCommunicationDetails::keyTargetApp].isString() &&
            obj[ProcessCommunicationDetails::keyTransportMessageID].isDouble() &&
            obj[ProcessCommunicationDetails::keyTargetApp] == qApp->applicationFilePath())
        {
            return obj;
        }
    }
    else
    {
        qWarning() << "Process communication module can not write shared memory. Last error is" << sharedMemory.errorString();
    }
    return QJsonObject();
}

ProcessCommunication::MessageDetails::MessageDetails(CallbackFunction callBack_)
    : transportMessageID(ProcessCommunicationDetails::lastMessageID)
    , callBack(callBack_)
{
    ProcessCommunicationDetails::lastMessageID++;
}
