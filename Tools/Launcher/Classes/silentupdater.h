#pragma once

#include <QObject>

class ApplicationManager;
class QTimer;

class SilentUpdater : public QObject
{
    Q_OBJECT

public:
    SilentUpdater(ApplicationManager* appManager, QObject* parent = nullptr);

private slots:
    void Process();

private:
    QTimer* updateTimer = nullptr;
    ApplicationManager* applicationManager = nullptr;
};
