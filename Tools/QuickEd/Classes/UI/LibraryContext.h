#ifndef __QUICKED_LIBRARY_CONTEXT_H__
#define __QUICKED_LIBRARY_CONTEXT_H__

#include <QObject>
#include <QPoint>

class Document;
class QAbstractItemModel;

class LibraryContext : public QObject
{
    Q_OBJECT
public:
    LibraryContext(Document *_document);
    virtual ~LibraryContext();
    
    Document *GetDocument() const;
    QAbstractItemModel *GetModel() const;
    
private:
    Document *document;
    QPoint scrollPosition;
    //QModelIndexList expandedItems;
    QAbstractItemModel *model;
};

#endif // __QUICKED_LIBRARY_CONTEXT_H__
