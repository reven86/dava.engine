#include "QtHelpers/ProcessCommunication.h"

#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QApplication>
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
    const qint64 maximumTimeout = pollingTime * 10;
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
    }
}

ProcessCommunication::~ProcessCommunication()
{
    if (sharedMemory.detach())
    {
        qWarning() << "Process communication module can not detach from shared memory, reason is" << sharedMemory.errorString();
    }
}

void ProcessCommunication::Send(const eMessage messageCode, const QString &targetAppPath, CallbackFunction callBack /* = CallbackFunction() */)
{
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
}

void ProcessCommunication::Poll()
{
    QJsonObject object = Read();
    if (object.isEmpty())
    {
        return;
    }
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
            return;
        }
    }

    eMessage clientMessage = static_cast<eMessage>(messageIDValue);
    eReply reply = processRequest(clientMessage);
    QString targetApp = object[ProcessCommunicationDetails::keySenderApp].toString();
    Reply(transportLevelID, reply, targetApp);
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
