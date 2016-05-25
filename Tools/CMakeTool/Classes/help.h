#ifndef HELP_H
#define HELP_H

#include <QObject>
#include <QString>

class Help : public QObject
{
    Q_OBJECT

public:
    explicit Help(QObject* parent = 0);
    Q_INVOKABLE void Show() const;

private:
    QString helpPath;
};

#endif // HELP_H
