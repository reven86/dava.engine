#ifndef __PROPERTIESVIEWCONTEXT_H__
#define __PROPERTIESVIEWCONTEXT_H__

#include <QPointer>
#include "UI/PackageDocument.h"

class PropertiesViewContext
{
public:
    PropertiesViewContext(PackageDocument *doc);
    ~PropertiesViewContext();

    PackageDocument *Document() const;
    //QPoint scrollPosition;
private:
    QPointer<PackageDocument> document;
};

#endif // __PROPERTIESVIEWCONTEXT_H__
