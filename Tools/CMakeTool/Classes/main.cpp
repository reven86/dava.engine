#include <QApplication>
#include <QString>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QProcessEnvironment>
#include <QMessageBox>
#include "configstorage.h"
#include "processwrapper.h"
#include "filesystemhelper.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QString configPath;
#ifdef Q_OS_WIN
    configPath = "../CMakeTool/Resources/config_windows.txt";
#elif defined Q_OS_MAC
    configPath = app.applicationDirPath() + "/../Resources/Data/config_mac.txt";
#else
    qCritical() << "application started on undefined platform";
    return 1;
#endif //platform
    ConfigStorage configStorage(configPath);
    QString configuration = configStorage.GetJSONTextFromConfig();
    if(configuration.isEmpty())
    {
        return 0;
    }
    FileSystemHelper fileSystemHelper;
    ProcessWrapper processWrapper;
    QQmlApplicationEngine engine;
    auto rootContext = engine.rootContext();
    rootContext->setContextProperty("applicationDirPath", app.applicationDirPath());
    rootContext->setContextProperty("configuration", configuration);
    rootContext->setContextProperty("fileSystemHelper", &fileSystemHelper);
    rootContext->setContextProperty("processWrapper", &processWrapper);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if(!engine.rootObjects().empty())
    {
        auto rootObject = engine.rootObjects().at(0);
        QObject::connect(rootObject, SIGNAL(dataReadyToSave(QString)), &configStorage, SLOT(SaveJSONTestToConfig(QString)));
        QObject::connect(rootObject, SIGNAL(invokeCmake(QString)), &processWrapper, SLOT(LaunchCmake(QString)));
        return app.exec();
    }
    QMessageBox::critical(nullptr, QObject::tr("Failed to load QML file"), QObject::tr("QML file not loaded!"));
    return 0;
}

