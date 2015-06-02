#include "LogModel.h"


#include <QDebug>
#include <QPainter>

#include "Utils/UTF8Utils.h"


LogModel::LogModel(QObject* parent)
    : QStandardItemModel(parent)
{
    connect(this, &LogModel::logged, this, &LogModel::AddMessage);
}

LogModel::~LogModel()
{
}

void LogModel::Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text)
{
    emit const_cast<LogModel *>(this)->logged(ll, QString::fromStdString(std::string(text)));
}

void LogModel::Output(DAVA::Logger::eLogLevel ll, const DAVA::char16* text)
{
    const DAVA::WideString wide(text);
    const DAVA::String utf8 = DAVA::UTF8Utils::EncodeToUTF8(wide);
    const QString& str = QString::fromUtf8(utf8.data(), utf8.size());
    emit const_cast<LogModel *>(this)->logged(ll, str);
}

void LogModel::AddMessage(int ll, const QString& text)
{
    const QList<QStandardItem *>& row = CreateItem(ll, normalize(text));
    appendRow(row);
}

QList<QStandardItem *> LogModel::CreateItem(int ll, const QString& text) const
{
    QList<QStandardItem *> row;

    QStandardItem* textItem = new QStandardItem();
    textItem->setText(text);
    textItem->setToolTip(text);
    textItem->setIcon(GetIcon(ll));
    row << textItem;

    for (int i = 0; i < row.size(); i++)
    {
        row[i]->setData(ll, LEVEL_ROLE);
    }

    return row;
}

QString LogModel::normalize(const QString& text) const
{
    return text.split('\n', QString::SkipEmptyParts).join("\n");
}

QPixmap LogModel::GetIcon(int ll) const
{
    const auto it = icons.constFind(ll);
    if (it != icons.constEnd())
    {
        return it.value();
    }

    QPixmap pix(16, 16);
    pix.fill(Qt::transparent);
    QPainter p(&pix);

    QColor bg = Qt::transparent;
    QColor fg1 = Qt::gray;

    switch (ll)
    {
    case DAVA::Logger::LEVEL_FRAMEWORK:
        bg = Qt::lightGray;
        break;
    case DAVA::Logger::LEVEL_DEBUG:
        bg = Qt::blue;
        break;
    case DAVA::Logger::LEVEL_INFO:
        bg = Qt::green;
        break;
    case DAVA::Logger::LEVEL_WARNING:
        bg = Qt::yellow;
        break;
    case DAVA::Logger::LEVEL_ERROR:
        bg = Qt::red;
        break;
    default:
        break;
    }

    const int ofs = 3;

    p.setBrush(bg);
    QRect rc = QRect(QPoint(0, 0), pix.size()).adjusted(ofs, ofs, -ofs, -ofs);
    p.setPen(fg1);
    p.drawEllipse(rc);

    icons[ll] = pix;
    return pix;
}