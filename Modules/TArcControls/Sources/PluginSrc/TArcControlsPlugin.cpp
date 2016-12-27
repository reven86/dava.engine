#include "ValueSetHelper.h"

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
        qmlRegisterSingletonType<TArcControls::ValueSetHelper>(uri, 1, 0, "ValueSetHelper", &TArcControls::ValueSetHelperSingletonProvider);
    }
};

#include "TArcControlsPlugin.moc"