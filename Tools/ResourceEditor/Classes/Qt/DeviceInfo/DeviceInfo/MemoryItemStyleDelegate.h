#pragma once
#include "Base/BaseTypes.h"
#include <QStyledItemDelegate>
class MemoryItemStyleDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    MemoryItemStyleDelegate(QWidget *parent = 0) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
    
    static QString formatMemoryData(DAVA::uint32 mem);
    ~MemoryItemStyleDelegate();

public slots:
    void commitAndCloseEditor();



};

