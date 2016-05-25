#ifndef __QTMIMEDATA_H__
#define __QTMIMEDATA_H__

#include "DAVAEngine.h"
#include <QMimeData>

class QtMimeData
{
public:
    //Utility methods
    static bool ContainsFilepathWithExtension(const QMimeData* data, const DAVA::String& extension);
    static bool IsURLEqualToExtension(const QUrl& url, const DAVA::String& extension);
};


#endif //#ifndef __QTMIMEDATA_H__