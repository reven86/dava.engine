#ifndef __LOGMODEL_H__
#define __LOGMODEL_H__


#include <QObject>
#include <QAbstractListModel>
#include <QMap>
#include <QTimer>
#include <QMutex>

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
        ORIGINAL_TEXT_ROLE
    };

    explicit LogModel(QObject* parent = nullptr);
    ~LogModel();

    const QPixmap &GetIcon(int ll) const;

    void Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text) override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
public slots:
    void AddMessage(DAVA::Logger::eLogLevel ll, const QString &text);
private slots:
    void OnTimeout();
private:
    QString normalize(const QString& text) const;
    void createIcons();
    struct LogItem
    {
        LogItem(DAVA::Logger::eLogLevel ll_ = DAVA::Logger::LEVEL_FRAMEWORK, const QString &text_ = QString());
        DAVA::Logger::eLogLevel ll;
        QString text;
        QString displayText;
    };
    QVector<LogItem> items;

    QVector<QPixmap> icons;
    mutable QMutex m_mutex;
    size_t registerCount = 0;
    QTimer *timer;

};


#endif // __LOGMODEL_H__
