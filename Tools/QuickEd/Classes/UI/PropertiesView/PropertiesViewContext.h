#ifndef __PROPERTIESVIEWCONTEXT_H__
#define __PROPERTIESVIEWCONTEXT_H__

class Document;

class PropertiesViewContext
{
public:
    PropertiesViewContext(Document *doc);
    ~PropertiesViewContext();

    Document *GetDocument() const;
    //QPoint scrollPosition;
private:
    Document *document;
};

#endif // __PROPERTIESVIEWCONTEXT_H__
