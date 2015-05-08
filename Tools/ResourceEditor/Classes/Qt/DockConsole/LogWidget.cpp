#include "LogWidget.h"
#include "ui_LogWidget.h"


#include <QAbstractItemModel>
#include <QDebug>
#include <QClipboard>
#include <QApplication>
#include <QTextStream>
#include <QEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include <QTimer>

#include "Settings/SettingsManager.h"

#include "LogModel.h"
#include "LogFilterModel.h"
#include "LogDelegate.h"


namespace
{
    const int scrollDelay = 50;
}


LogWidget::LogWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::LogWidget())
    , eventSkipper(new QTimer(this))
    , doAutoScroll(true)
    , scrollStateDetected(false)
    , registerLoggerAsLocal( true )
{
    ui->setupUi(this);

    eventSkipper->setInterval(scrollDelay);
    eventSkipper->setSingleShot(true);

    LogDelegate* delegate = new LogDelegate(ui->log, this);
    logModel = new LogModel(this);
    logFilterModel = new LogFilterModel(this);

    logFilterModel->setSourceModel(logModel);
    ui->log->setModel(logFilterModel);
    connect(delegate, SIGNAL( copyRequest() ), SLOT( OnCopy() ));
    ui->log->installEventFilter(this);

    FillFiltersCombo();

    connect(ui->clear, SIGNAL( clicked() ), SLOT( OnClear() ));
    connect(ui->filter, SIGNAL( done() ), SLOT( OnFilterChanged() ));
    connect(ui->search, SIGNAL( textUpdated( const QString& ) ), SLOT( OnTextFilterChanged( const QString& ) ));

    // Auto scroll feature
    connect(ui->log->model(), SIGNAL( rowsAboutToBeInserted(const QModelIndex&, int, int) ), SLOT( DetectAutoScroll() ));
    connect(ui->log->model(), SIGNAL( rowsAboutToBeRemoved(const QModelIndex&, int, int) ), SLOT( DetectAutoScroll() ));
    connect(ui->log->model(), SIGNAL( modelAboutToBeReset() ), SLOT( DetectAutoScroll() ));

    connect(ui->log->model(), SIGNAL( rowsInserted(const QModelIndex&, int, int) ), eventSkipper, SLOT( start() ));
    connect(ui->log->model(), SIGNAL( modelReset() ), eventSkipper, SLOT( start() ));
    connect(eventSkipper, SIGNAL( timeout() ), SLOT( DoAutoScroll() ) );

    QMetaObject::invokeMethod( this, "LoadSettings", Qt::QueuedConnection );
    QMetaObject::invokeMethod( this, "OnFilterChanged", Qt::QueuedConnection );
}

LogWidget::~LogWidget()
{
    SaveSettings();
}

void LogWidget::SetRegisterLoggerAsLocal( bool isLocal )
{
    registerLoggerAsLocal = isLocal;
}

LogModel* LogWidget::Model()
{
    return logModel;
}

void LogWidget::OnFilterChanged()
{
    QList<QVariant> selection = ui->filter->selectedUserData();
    QSet<int> levels;

    for (int i = 0; i < selection.size(); i++)
    {
        bool isValid = false;
        const int level = selection[i].toInt(&isValid);
        if (isValid)
        {
            levels.insert(level);
        }
    }

    logFilterModel->SetFilters(levels);
}

void LogWidget::OnTextFilterChanged(const QString& text)
{
    logFilterModel->SetFilterString(text);
}

void LogWidget::FillFiltersCombo()
{
    ui->filter->addItem("LEVEL_FRAMEWORK", DAVA::Logger::LEVEL_FRAMEWORK);
    ui->filter->addItem("LEVEL_DEBUG", DAVA::Logger::LEVEL_DEBUG);
    ui->filter->addItem("LEVEL_INFO", DAVA::Logger::LEVEL_INFO);
    ui->filter->addItem("LEVEL_WARNING", DAVA::Logger::LEVEL_WARNING);
    ui->filter->addItem("LEVEL_ERROR", DAVA::Logger::LEVEL_ERROR);

    QAbstractItemModel* m = ui->filter->model();
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

void LogWidget::LoadSettings()
{
    if (!registerLoggerAsLocal)
        return;

    DAVA::VariantType v = SettingsManager::Instance()->GetValue(Settings::Internal_LogLevelFilter);
    const DAVA::uint32* a = (DAVA::uint32*)v.AsByteArray();
    const DAVA::int32 n = SettingsManager::Instance()->GetValue(Settings::Internal_LogLevelFilter).AsByteArraySize() / sizeof(DAVA::uint32);
    QList<QVariant> levels;
    for (int i = 0; i < n; i++)
    {
        levels << a[i];
    }

    ui->filter->selectUserData(levels);

    const DAVA::String text = SettingsManager::Instance()->GetValue(Settings::Internal_LogTextFilter).AsString();
    ui->search->setText(QString::fromStdString(text));
}

void LogWidget::SaveSettings()
{
    if (!registerLoggerAsLocal)
        return;

    DAVA::Logger::RemoveCustomOutput(logModel);

    const QList<QVariant>& selection = ui->filter->selectedUserData();
    const int n = selection.size();
    DAVA::Vector<DAVA::uint32> v;
    v.reserve(selection.size());
    for (int i = 0; i < n; i++)
    {
        v.push_back(selection[i].toInt());
    }
    SettingsManager::Instance()->SetValue(Settings::Internal_LogLevelFilter, DAVA::VariantType((DAVA::uint8 *)&v[0], v.size() * sizeof( v[0] )));

    const DAVA::String text = ui->search->text().toStdString();
    SettingsManager::Instance()->SetValue(Settings::Internal_LogTextFilter, DAVA::VariantType(text));
}

bool LogWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == ui->log)
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
    const QModelIndexList& selection = ui->log->selectionModel()->selectedIndexes();
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
    logModel->clear();
}

void LogWidget::DetectAutoScroll()
{
    if (scrollStateDetected)
        return;

    scrollStateDetected = true;
    QScrollBar *scroll = ui->log->verticalScrollBar();
    doAutoScroll = (scroll->value() == scroll->maximum());
}

void LogWidget::DoAutoScroll()
{
    if (!doAutoScroll)
        return ;

    ui->log->scrollToBottom();
    scrollStateDetected = false;
}
