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
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawText(option.rect, formatMemoryData(index.data().toInt(nullptr)));
        painter->restore();
    }
    else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}
QString MemoryItemStyleDelegate::formatMemoryData(DAVA::uint32 mem)
{
    float memoryData = static_cast<float>(mem);
    QString letter = "B";
    static std::array<QString, 8> letters = { "KB", "MB", "GB", "TP", "PB", "EB", "ZB", "YB" };
    size_t counter = 0;
    while (memoryData > 1024.0f && counter < letters.size())
    {
        memoryData /= 1024.0f;
        letter = letters[counter];
        counter++;
    }
    return QString(std::to_string(memoryData).c_str())+" " + letter;
}
QSize MemoryItemStyleDelegate::sizeHint(const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    return QSize(20,20);
}

void MemoryItemStyleDelegate::commitAndCloseEditor()
{

}