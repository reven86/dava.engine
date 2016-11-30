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

    const qint64 pollingTime = 200; //ms;
    const qint64 maximumTimeout = 5 * 60 * 1000; // 5 minutes
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
    if (sharedMemory.detach() == false)
    {
        qWarning() << "Process communication module can not detach from shared memory, reason is" << sharedMemory.errorString();
    }
}

void ProcessCommunication::InitPollTimer()
{
    pollTimer = new QTimer(this);
    pollTimer->setInterval(ProcessCommunicationDetails::pollingTime);
    pollTimer->setSingleShot(false);
    connect(pollTimer, &QTimer::timeout, this, &ProcessCommunication::Poll);
    pollTimer->start();
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
        return tr("communication module was not initialized, restart appication please");
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
    if (QFile::exists(targetAppPath) == false)
    {
        callBack(eReply::NOT_EXISTS);
        return;
    }
    if (ProcessHelper::IsProcessRuning(targetAppPath) == false)
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
    QEventLoop loop;

    CallbackFunction callBack = [&haveAnswer, &reply, &loop](eReply replyFromClient){
        reply = replyFromClient;
        haveAnswer = true;
        loop.quit();
    };
    SendAsync(messagCode, targetAppPath, callBack);
    if (haveAnswer == false)
    {
        loop.exec();
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
    if (object.isEmpty() == false)
    {
        bool gotReply = false;

        int messageIDValue = object[ProcessCommunicationDetails::keyClientMessageID].toInt();

        qint64 transportLevelID = object[ProcessCommunicationDetails::keyTransportMessageID].toInt();
        QMutableListIterator<MessageDetails> iterator(sentMessages);
        while (iterator.hasNext())
        {
            const MessageDetails &details = iterator.next();
            //we got reply for sent message
            if (details.transportMessageID == transportLevelID)
            {
                details.callBack(static_cast<eReply>(messageIDValue));
                iterator.remove();
                gotReply = true;
                Flush();
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

    //check messages for timeout error
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
    Write(jsonData);
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
    if (data.size() > ProcessCommunicationDetails::maxDataSize)
    {
        qWarning() << "Process communication module can not send data biggest then max size";
    }
    else if (Lock() != false)
    {
        void* pureData = sharedMemory.data();
        if (pureData != nullptr)
        {
            int dataToWriteSize = data.size();
            memcpy(pureData, data.data(), dataToWriteSize);
            memset(static_cast<char*>(pureData) + dataToWriteSize, '\0', ProcessCommunicationDetails::maxDataSize - dataToWriteSize);
            success = true;
        }
        else
        {
            qWarning() << "Process communication module can not write to the shared memory. Last error is" << sharedMemory.errorString();
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
        //check that message is valid and it delivered for the current application
        //because this detail need to be checked on the protocol level
        QJsonObject obj = document.object();
        if (obj[ProcessCommunicationDetails::keyClientMessageID].isDouble() &&
            obj[ProcessCommunicationDetails::keySenderApp].isString() &&
            obj[ProcessCommunicationDetails::keyTargetApp].isString() &&
            obj[ProcessCommunicationDetails::keyTransportMessageID].isDouble() &&
            obj[ProcessCommunicationDetails::keyTargetApp] == qApp->applicationFilePath())
        {
            return obj;
        }
        else
        {
            return QJsonObject();
        }
    }
    else
    {
        qWarning() << "Process communication module can not parse shared memory data, last error is " << parseError.errorString();
    }
    return QJsonObject();
}

bool ProcessCommunication::Flush()
{
    bool success = Write(QByteArray());
    if (success == false)
    {
        qWarning() << "Can not flush data buffer, possible to loss data";
    }
    return success;
}

qint64 ProcessCommunication::MessageDetails::lastMessageID = 0;

ProcessCommunication::MessageDetails::MessageDetails(CallbackFunction callBack_)
    : transportMessageID(lastMessageID)
    , callBack(callBack_)
{
    lastMessageID++;
}
