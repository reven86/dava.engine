#ifndef __PROPERTIESVIEWCONTEXT_H__
#define __PROPERTIESVIEWCONTEXT_H__

#include <QObject>

class Document;

class PropertiesContext : public QObject
{
    
    Q_OBJECT
public:
    PropertiesContext(Document *doc);
    virtual ~PropertiesContext();

    Document *GetDocument() const;
    //QPoint scrollPosition;
private:
    Document *document;
};

#endif // __PROPERTIESVIEWCONTEXT_H__
