#ifndef __LOGGER_OUTPUT_OBJECT_H__
#define __LOGGER_OUTPUT_OBJECT_H__

#include "Logger/Logger.h"
#include <QObject>

class LoggerOutputObject final
: public QObject
{
    Q_OBJECT
public:
    LoggerOutputObject(QObject* parent = nullptr); //WARNING ! Can be removed at any time by logger
    ~LoggerOutputObject() = default;

signals:
    void OutputReady(DAVA::Logger::eLogLevel ll, const QByteArray& text);

private:
    class LoggerOutputContainer;
    LoggerOutputContainer* outputContainer = nullptr;
};

#endif // __LOGGER_OUTPUT_OBJECT_H__
