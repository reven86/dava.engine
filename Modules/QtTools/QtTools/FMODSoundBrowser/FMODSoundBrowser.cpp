#include "QtTools/FMODSoundBrowser/FMODSoundBrowser.h"

#include "QtTools/WidgetHelpers/SharedIcon.h"
#include "ui_FMODSoundBrowser.h"

#include <QTreeWidget>
#include <QMessageBox>
#include <QLabel>
#include <QSlider>
#include <QToolTip>

FMODSoundBrowser::FMODSoundBrowser(QWidget* parent)
    : QDialog(parent)
    , selectedItem(0)
    , ui(new Ui::FMODSoundBrowser)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint);

    QObject::connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnEventDoubleClicked(QTreeWidgetItem*, int)));
    QObject::connect(ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(OnEventSelected(QTreeWidgetItem*, QTreeWidgetItem*)));

    QObject::connect(ui->selectButton, SIGNAL(clicked()), this, SLOT(accept()));
    QObject::connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    QObject::connect(this, SIGNAL(accepted()), this, SLOT(OnAccepted()));
    QObject::connect(this, SIGNAL(rejected()), this, SLOT(OnRejected()));

    SetSelectedItem(0);

    setModal(true);

    UpdateEventTree();
}

FMODSoundBrowser::~FMODSoundBrowser()
{
    delete ui;
}

DAVA::String FMODSoundBrowser::GetSelectSoundEvent()
{
    if (selectedItem)
    {
        QVariant data = selectedItem->data(0, Qt::UserRole);
        if (!data.isNull())
            return data.toString().toStdString();
    }

    return "";
}

void FMODSoundBrowser::UpdateEventTree()
{
    DAVA::Vector<DAVA::String> names;
    DAVA::SoundSystem::Instance()->GetAllEventsNames(names);

    FillEventsTree(names);
}

void FMODSoundBrowser::OnEventSelected(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    if (!current->childCount())
        SetSelectedItem(current);
    else
        SetSelectedItem(0);
}

void FMODSoundBrowser::OnEventDoubleClicked(QTreeWidgetItem* item, int column)
{
    if (!item->childCount())
    {
        SetSelectedItem(item);
        accept();
    }
    else
    {
        SetSelectedItem(0);
    }
}

void FMODSoundBrowser::OnAccepted()
{
}

void FMODSoundBrowser::OnRejected()
{
    SetSelectedItem(0);
}

void FMODSoundBrowser::SetSelectedItem(QTreeWidgetItem* item)
{
    selectedItem = item;
    if (selectedItem)
        ui->selectButton->setDisabled(false);
    else
        ui->selectButton->setDisabled(true);
}

void FMODSoundBrowser::SelectItemAndExpandTreeByEventName(const DAVA::String& eventName)
{
    DAVA::Vector<DAVA::String> tokens;
    DAVA::Split(eventName, "/", tokens);
    DAVA::int32 tokensCount = static_cast<DAVA::int32>(tokens.size());
    QTreeWidgetItem* currentItem = ui->treeWidget->invisibleRootItem();
    for (DAVA::int32 i = 0; i < tokensCount; i++)
    {
        QString currentToken = QString(tokens[i].c_str());
        DAVA::int32 childrenCount = currentItem->childCount();
        QTreeWidgetItem* findedItem = 0;
        for (DAVA::int32 k = 0; k < childrenCount; k++)
        {
            QTreeWidgetItem* currentChild = currentItem->child(k);
            if (currentChild->text(0) == currentToken)
            {
                findedItem = currentChild;
                findedItem->setExpanded(true);
                break;
            }
        }
        if (!findedItem)
            return;

        currentItem = findedItem;
    }
    currentItem->setSelected(true);
}

void FMODSoundBrowser::FillEventsTree(const DAVA::Vector<DAVA::String>& names)
{
    ui->treeWidget->clear();

    DAVA::int32 eventsCount = static_cast<DAVA::int32>(names.size());
    for (DAVA::int32 i = 0; i < eventsCount; i++)
    {
        const DAVA::String& eventPath = names[i];

        DAVA::Vector<DAVA::String> tokens;
        DAVA::Split(eventPath, "/", tokens);

        DAVA::int32 tokensCount = static_cast<DAVA::int32>(tokens.size());
        QTreeWidgetItem* currentItem = ui->treeWidget->invisibleRootItem();
        for (DAVA::int32 j = 0; j < tokensCount; j++)
        {
            QString currentToken = QString(tokens[j].c_str());
            DAVA::int32 childrenCount = currentItem->childCount();
            QTreeWidgetItem* findedItem = 0;
            for (DAVA::int32 k = 0; k < childrenCount; k++)
            {
                QTreeWidgetItem* currentChild = currentItem->child(k);
                if (currentChild->text(0) == currentToken)
                {
                    findedItem = currentChild;
                    break;
                }
            }

            bool isEvent = (j == tokensCount - 1);

            if (findedItem == 0)
            {
                findedItem = new QTreeWidgetItem(currentItem);
                currentItem->addChild(findedItem);
                findedItem->setText(0, currentToken);

                if (isEvent)
                {
                    findedItem->setIcon(0, SharedIcon(":/QtTools/Icons/sound.png"));
                    findedItem->setData(0, Qt::UserRole, QString(eventPath.c_str()));
                }
                else
                {
                    findedItem->setIcon(0, SharedIcon(":/QtTools/Icons/sound_group.png"));
                }
            }
            currentItem = findedItem;
        }
    }
}

void FMODSoundBrowser::SetCurrentEvent(const DAVA::String& eventPath)
{
    ui->treeWidget->collapseAll();
    SelectItemAndExpandTreeByEventName(eventPath);
}
