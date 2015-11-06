#ifndef __LOGMODEL_H__
#define __LOGMODEL_H__


#include <QObject>
#include <QAbstractListModel>
#include <functional>
#include <QPointer>
#include "FileSystem/Logger.h"

class QMutex;
class QTimer;

class LoggerOutputObject
: public QObject,
  public DAVA::LoggerOutput
{
    Q_OBJECT
public:
    LoggerOutputObject(QObject* parent = nullptr);
    ~LoggerOutputObject() override = default; //this object deletes by logger
    void Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text) override;
};

class LogModel
    : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles
    {
        LEVEL_ROLE = Qt::UserRole,
        INTERNAL_DATA_ROLE
    };
    using ConvertFunc = std::function < DAVA::String(const DAVA::String &) >;

    explicit LogModel(QObject* parent = nullptr);
    ~LogModel();
    void SetConvertFunction(ConvertFunc func); //provide mechanism to convert data string to string to be displayed

    const QPixmap &GetIcon(int ll) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

public slots:
    void AddMessage(DAVA::Logger::eLogLevel ll, const QByteArray &text);
    void AddMessageAsync(DAVA::Logger::eLogLevel ll, const QByteArray &msg);
    void Clear();

private slots:
    void Sync();

private:
    void createIcons();
    struct LogItem
    {
        LogItem(DAVA::Logger::eLogLevel ll_ = DAVA::Logger::LEVEL_FRAMEWORK, const QString &text_ = QString(), const QString &data_ = QString());
        DAVA::Logger::eLogLevel ll;
        QString text;
        QString data;
    };
    QVector<LogItem> items;

    QVector<QPixmap> icons;
    ConvertFunc func;
    QPointer<LoggerOutputObject> loggerOutputObject = nullptr;

    QVector<LogItem> itemsToAdd;
    std::unique_ptr<QMutex> mutex = nullptr;
    QTimer *syncTimer = nullptr;
};

#endif // __LOGMODEL_H__
