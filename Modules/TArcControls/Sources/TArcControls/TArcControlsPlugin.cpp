#include <QQmlExtensionPlugin>
#include <QtQml>

class QExampleQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "dava.com.TArcControlsExtension")

public:
    void registerTypes(const char *uri)
    {
        Q_ASSERT(uri == QLatin1String("TArcControls"));
    }
};

#include "TArcControlsPlugin.moc"