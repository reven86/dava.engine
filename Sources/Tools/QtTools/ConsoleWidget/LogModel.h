#ifndef __LOGMODEL_H__
#define __LOGMODEL_H__


#include <QObject>
#include <QAbstractListModel>
#include <QMap>
#include <QTimer>
#include <QMutex>
#include <functional>

#include "FileSystem/Logger.h"


class LogModel
    : public QAbstractListModel
    , public DAVA::LoggerOutput
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
    
    void Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text) override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
public slots:
    void AddMessage(DAVA::Logger::eLogLevel ll, const QString &text);
    void Clear();
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
};


#endif // __LOGMODEL_H__
