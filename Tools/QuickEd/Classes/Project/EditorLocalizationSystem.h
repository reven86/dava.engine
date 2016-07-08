#ifndef __EDITOR_LOCALIZATION_SYSTEM_H__
#define __EDITOR_LOCALIZATION_SYSTEM_H__

#include <QObject>
#include <QMap>
#include <QStringList>

class QDir;

class EditorLocalizationSystem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentLocale READ GetCurrentLocale WRITE SetCurrentLocale NOTIFY CurrentLocaleChanged)

public:
    explicit EditorLocalizationSystem(QObject* parent = nullptr);

    QStringList GetAvailableLocaleNames() const;
    QStringList GetAvailableLocaleValues() const;

    void SetDirectory(const QDir& dir);
    void SetCurrentLocaleValue(const QString& localeStr);
    void Cleanup();

private:
    static QString GetLocaleNameFromStr(const QString& localeStr);
    QMap<QString, QString> availableLocales;

    //properties section
public:
    QString GetCurrentLocale() const;

public slots:
    void SetCurrentLocale(const QString& locale);

signals:
    void CurrentLocaleChanged(const QString& locale);

private:
    QString currentLocale;
};


#endif //__EDITOR_LOCALIZATION_SYSTEM_H__
