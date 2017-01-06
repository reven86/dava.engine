#include "QtMimeData.h"
#include <QUrl>

bool QtMimeData::ContainsFilepathWithExtension(const QMimeData* data, const DAVA::String& extension)
{
    if (!data || !data->hasUrls())
        return false;

    QList<QUrl> urls = data->urls();
    for (int i = 0; i < urls.size(); ++i)
    {
        if (IsURLEqualToExtension(urls.at(i), extension))
        {
            return true;
        }
    }

    return false;
}

bool QtMimeData::IsURLEqualToExtension(const QUrl& url, const DAVA::String& extension)
{
    DAVA::FilePath path = url.toLocalFile().toStdString();
    return (path.IsEqualToExtension(extension));
}
