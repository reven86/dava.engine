#ifndef PROCESSWRAPPER_H
#define PROCESSWRAPPER_H

#include <QObject>
#include <QProcess>
#include <QQueue>
#include <QVariant>

class ProcessWrapper : public QObject
{
    Q_OBJECT
public:
    explicit ProcessWrapper(QObject *parent = 0);
    ~ProcessWrapper();

signals:
    void processStateChanged(QVariant text);
    void processErrorChanged(QVariant text);
    void processStandardOutput(QVariant text);
    void processStandardError(QVariant text);
    void testSignal();

    void processOutput(QString text);

public slots:
    void LaunchCmake(QString command);

private slots:
    void OnReadyReadStandardOutput();
    void OnReadyReadStandardError();
    void OnProcessStateChanged(QProcess::ProcessState newState);
    void OnProcessError(QProcess::ProcessError error);

private:
    Q_INVOKABLE void StartNextCommand();

    QProcess process;
    QQueue<QString> taskQueue;
};

#endif // PROCESSWRAPPER_H
