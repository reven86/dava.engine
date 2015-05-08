#include <QtGui>
#include <array>
#include "MemoryItemStyleDelegate.h"


MemoryItemStyleDelegate::~MemoryItemStyleDelegate()
{
}
void MemoryItemStyleDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    
    if (index.data().canConvert(QVariant::Int)) 
    {
        int memoryData = index.data().toInt(nullptr);
        QString letter = "B";
        static std::array<QString, 8> letters = { "KB", "MB", "GB", "TP", "PB", "EB", "ZB", "YB" };
        size_t counter = 0;
        while (memoryData > 1024 && counter < letters.size())
        {
            memoryData /= 1024;
            letter = letters[counter];
            counter++;
        }
        
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawText(option.rect, QString(std::to_string(memoryData).c_str()) + letter);
        painter->restore();
    }
    else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}
QSize MemoryItemStyleDelegate::sizeHint(const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    return QSize(20,20);
}

void MemoryItemStyleDelegate::commitAndCloseEditor()
{

}