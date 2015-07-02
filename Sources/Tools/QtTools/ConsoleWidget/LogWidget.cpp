#include "LogWidget.h"
#include <QDebug>
#include <QClipboard>
#include <QKeyEvent>
#include <QScrollBar>
#include <QScrollBar>
#include <QBuffer>

#include "LogModel.h"
#include "LogFilterModel.h"
#include "LogDelegate.h"

#include "Base/GlobalEnum.h"
#include "Utils/PointerSerializer.h"
#include "Debug/DVAssert.h"

LogWidget::LogWidget(QWidget* parent)
    : QWidget(parent)
    , onBottom(true)
{
    qRegisterMetaType<DAVA::PointerSerializer>("DAVA::PointerSerializer");
    setupUi(this);
    connect(log, &QListView::clicked, this, &LogWidget::OnClicked);
    time.start();

    //LogDelegate* delegate = new LogDelegate(log, this);
    logModel = new LogModel(this);
    logFilterModel = new LogFilterModel(this);

    logFilterModel->setSourceModel(logModel);
    log->setModel(logFilterModel);
    //connect(delegate, &LogDelegate::copyRequest, this, &LogWidget::OnCopy);
    //connect(delegate, &LogDelegate::clearRequest, this, &LogWidget::OnClear);
    log->installEventFilter(this);

    FillFiltersCombo();

    connect(filter, &CheckableComboBox::selectedUserDataChanged, logFilterModel, &LogFilterModel::SetFilters);
    connect(search, &LineEditEx::textUpdated, this, &LogWidget::OnTextFilterChanged);
    connect(log->model(), &QAbstractItemModel::rowsAboutToBeInserted, this, &LogWidget::OnBeforeAdded);
    connect(log->model(), &QAbstractItemModel::rowsInserted, this, &LogWidget::OnRowAdded);
    filter->selectUserData(logFilterModel->GetFilters());
}

LogModel* LogWidget::Model()
{
    return logModel;
}

QByteArray LogWidget::Serialize() const
{
    QByteArray retData;
    QDataStream stream(&retData, QIODevice::WriteOnly);
    stream << logFilterModel->GetFilterString();
    stream << logFilterModel->GetFilters();
    return retData;
}

void LogWidget::Deserialize(const QByteArray& data)
{
    QDataStream stream(data);
    QString filterString;
    stream >> filterString;
    if (stream.status() == QDataStream::ReadCorruptData)
    {
        return;
    }
    QVariantList logLevels;
    stream >> logLevels;
    if (stream.status() == QDataStream::ReadCorruptData)
    {
        return;
    }
    logFilterModel->SetFilterString(filterString);
    logFilterModel->SetFilters(logLevels);
    filter->selectUserData(logLevels);
}

void LogWidget::AddResultList(const DAVA::ResultList &resultList)
{
    for(const auto &result : resultList.GetResults())
    {
        DAVA::Logger::eLogLevel level;
        switch(result.type)
        {
            case DAVA::Result::RESULT_SUCCESS:
                level = DAVA::Logger::LEVEL_INFO;
            break;
            case DAVA::Result::RESULT_FAILURE:
                level = DAVA::Logger::LEVEL_WARNING;
            break;
            case DAVA::Result::RESULT_ERROR:
                level = DAVA::Logger::LEVEL_ERROR;
            break;
        }
        logModel->AddMessage(level, QString::fromStdString(result.message));
    }
}

void LogWidget::OnTextFilterChanged(const QString& text)
{
    logFilterModel->SetFilterString(text);
}

void LogWidget::FillFiltersCombo()
{
    const auto &logMap = GlobalEnumMap<DAVA::Logger::eLogLevel>::Instance();
    for (size_t i = 0; i < logMap->GetCount(); ++i)
    {
        int value;
        bool ok = logMap->GetValue(i, value);
        if (!ok)
        {
            DVASSERT_MSG(ok, "wrong enum used to create GPU list");
            break;
        }
        filter->addItem(logMap->ToString(value), value);
    }

    QAbstractItemModel* m = filter->model();
    const int n = m->rowCount();
    for (int i = 0; i < n; i++)
    {
        QModelIndex index = m->index(i, 0, QModelIndex());
        const int ll = index.data(LogModel::LEVEL_ROLE).toInt();
        const QPixmap& pix = logModel->GetIcon(ll);
        m->setData(index, pix, Qt::DecorationRole);
        m->setData(index, Qt::Checked, Qt::CheckStateRole);
    }
}

bool LogWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == log)
    {
        switch (event->type())
        {
        case QEvent::KeyPress:
            {
                QKeyEvent* ke = static_cast<QKeyEvent *>(event);
                if (ke->matches(QKeySequence::Copy))
                {
                    OnCopy();
                    return true;
                }
            }
            break;

        default:
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void LogWidget::OnCopy()
{
    const QModelIndexList& selection = log->selectionModel()->selectedIndexes();
    const int n = selection.size();
    if (n == 0)
        return ;

    QMap< int, QModelIndex > sortedSelection;
    for (int i = 0; i < n; i++)
    {
        const QModelIndex& index = selection[i];
        const int realIdx = index.row();
        sortedSelection[realIdx] = index;
    }

    QString text;
    QTextStream ss(&text);
    for (auto it = sortedSelection.constBegin(); it != sortedSelection.constEnd(); ++it)
    {
        ss << it.value().data(Qt::DisplayRole).toString() << "\n";
    }
    ss.flush();

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

void LogWidget::OnClear()
{
    logModel->removeRows(0, logModel->rowCount());
}

void LogWidget::OnClicked(const QModelIndex &index)
{
    auto pIndex = logFilterModel->mapToSource(index);
    QString text = logModel->data(pIndex, LogModel::ORIGINAL_TEXT_ROLE).toString();
    emit ItemClicked(DAVA::PointerSerializer(text.toStdString()));
}

void LogWidget::OnBeforeAdded()
{
    onBottom = log->verticalScrollBar()->value() == log->verticalScrollBar()->maximum();
}


void LogWidget::OnRowAdded()
{
    if (onBottom)
    {
        log->scrollToBottom();
    }
}

