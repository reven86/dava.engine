#include "LogWidget.h"
#include <QDebug>
#include <QClipboard>
#include <QKeyEvent>
#include <QScrollBar>

#include "LogModel.h"
#include "LogFilterModel.h"
#include "LogDelegate.h"

#include "Base/GlobalEnum.h"
#include "Debug/DVAssert.h"

LogWidget::LogWidget(QWidget* parent)
    : QWidget(parent)
    , onBottom(true)
{
    setupUi(this);
    time.start();

    logModel = new LogModel(this);
    logFilterModel = new LogFilterModel(this);

    logFilterModel->setSourceModel(logModel);
    log->setModel(logFilterModel);
    log->installEventFilter(this);

    FillFiltersCombo();

    connect(filter, &CheckableComboBox::selectedUserDataChanged, logFilterModel, &LogFilterModel::SetFilters);
    connect(search, &LineEditEx::textUpdated, this, &LogWidget::OnTextFilterChanged);
    connect(logFilterModel, &LogFilterModel::filterStringChanged, search, &LineEditEx::setText);
    connect(log->model(), &QAbstractItemModel::rowsAboutToBeInserted, this, &LogWidget::OnBeforeAdded);
    connect(log->model(), &QAbstractItemModel::rowsInserted, this, &LogWidget::OnRowAdded);
    connect(log, &QListView::clicked, [this](const QModelIndex &index)
    {
        emit ItemClicked(logModel->data(index, LogModel::INTERNAL_DATA_ROLE).toString());
    });
    filter->selectUserData(logFilterModel->GetFilters());
}

void LogWidget::SetConvertFunction(LogModel::ConvertFunc func)
{
    logModel->SetConvertFunction(func);
}

LogModel* LogWidget::Model() const
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
    if (stream.status() == QDataStream::ReadCorruptData || stream.status() == QDataStream::ReadPastEnd)
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
            DVASSERT_MSG(ok, "wrong enum used to create eLogLevel list");
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

