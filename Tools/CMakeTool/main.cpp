#include <QApplication>
#include <QQmlApplicationEngine>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QString configPath;
#ifdef Q_OS_WIN
    configPath = ":/Resources/config_windows.txt";
#elif Q_OS_MAC
    configPath = ":/Resources/config_mac.txt";
#else
    qCritical() << "application started on undefined platform";
    return 1;
#endif //platform
    QFile configFile(fileName);
    if(configFile.op)
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}

