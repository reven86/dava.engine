#include "RunActionEventWidget.h"

#include <QStringList>
#include <QSet>
#include <QCompleter>

#include "ui_RunActionEventWidget.h"

#include "DAVAEngine.h"
#include "Scene3D/Components/ActionComponent.h"

#include "Classes/Qt/Main/mainwindow.h"
#include "Scene/SceneEditor2.h"
#include "Settings/SettingsManager.h"


namespace
{
    const String settingsType("Internal/RunActionEventWidget/CurrentType");
}


RunActionEventWidget::RunActionEventWidget(QWidget *parent)
    : QWidget(parent)
    , ui( new Ui::RunActionEventWidget() )
    , scene(NULL)
{
    ui->setupUi(this);
    setObjectName("RunActionEventWidget");

    ui->eventType->addItem("Switch", ActionComponent::Action::EVENT_SWITCH_CHANGED);
    ui->eventType->addItem("Added", ActionComponent::Action::EVENT_ADDED_TO_SCENE);
    ui->eventType->addItem("User", ActionComponent::Action::EVENT_CUSTOM);

    editorIdMap[ActionComponent::Action::EVENT_SWITCH_CHANGED] = 0;
    editorIdMap[ActionComponent::Action::EVENT_ADDED_TO_SCENE] = 1;
    editorIdMap[ActionComponent::Action::EVENT_CUSTOM] = 2;

    autocompleteModel = new QStringListModel(this);
    ui->name->setModel(autocompleteModel);
    if (ui->name->completer())
    {
        ui->name->completer()->setCompletionMode(QCompleter::PopupCompletion);
    }

    connect( ui->eventType, SIGNAL( currentIndexChanged( int ) ), SLOT( OnTypeChanged() ) );
    connect( ui->run, SIGNAL( clicked() ), SLOT( OnInvoke() ) );
    connect(SceneSignals::Instance(), SIGNAL(SelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)), this, SLOT(sceneSelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)));
    connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(sceneActivated(SceneEditor2 *)));

    const ActionComponent::Action::eEvent eventType = static_cast<ActionComponent::Action::eEvent>(SettingsManager::Instance()->GetValue(settingsType).AsUInt32());
    ui->eventType->setCurrentIndex(editorIdMap[eventType]);
}

RunActionEventWidget::~RunActionEventWidget()
{
}

void RunActionEventWidget::OnTypeChanged()
{
    const uint32 eventTypeId = ui->eventType->itemData(ui->eventType->currentIndex()).toUInt();
    const int editorindex = editorIdMap[eventTypeId];
    DVASSERT(editorindex < ui->stackedWidget->count());

    ui->stackedWidget->setCurrentIndex(editorindex);
    SettingsManager::Instance()->SetValue(settingsType,VariantType(eventTypeId));
}

void RunActionEventWidget::OnInvoke()
{
    const uint eventTypeId = ui->eventType->itemData(ui->eventType->currentIndex()).toUInt();
    SceneEditor2 *editor = QtMainWindow::Instance()->GetCurrentScene();
    if (editor == NULL)
        return ;

    const uint32 switchIndex = ui->switchIndex->value();
    const FastName name(ui->name->currentText().toStdString().c_str());

    const EntityGroup& selection = editor->selectionSystem->GetSelection();
    for (size_t i = 0; i < selection.Size(); i++)
    {
        Entity* entity = selection.GetEntity(i);
        ActionComponent *component = static_cast<ActionComponent *>(entity->GetComponent(Component::ACTION_COMPONENT));
        if (!component)
            continue;
        const uint32 nEvents = component->GetCount();
        for (uint32 componentIdx = 0; componentIdx < nEvents; componentIdx++)
        {
            ActionComponent::Action& act = component->Get(componentIdx);
            if (act.eventType == eventTypeId)
            {
                switch ( eventTypeId )
                {
                case ActionComponent::Action::EVENT_SWITCH_CHANGED:
                    if (act.switchIndex == switchIndex)
                    {
                        component->StartSwitch(switchIndex);
                    }
                    break;
                case ActionComponent::Action::EVENT_ADDED_TO_SCENE:
                    component->StartAdd();
                    break;
                case ActionComponent::Action::EVENT_CUSTOM:
                    if (act.userEventId == name)
                    {
                        component->StartUser(name);
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }

}

void RunActionEventWidget::sceneActivated(SceneEditor2 *_scene)
{
    scene = _scene;
    sceneSelectionChanged(scene, NULL, NULL);
}

void RunActionEventWidget::sceneSelectionChanged(SceneEditor2 *_scene, const EntityGroup *selected, const EntityGroup *deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    if (scene == NULL)
    {
        autocompleteModel->setStringList(QStringList());
        return ;
    }

    if (scene != _scene)
    {
        return ;
    }

    QSet< QString > nameSet;

    const EntityGroup& selection = scene->selectionSystem->GetSelection();
    for (size_t i = 0; i < selection.Size(); i++)
    {
        Entity* entity = selection.GetEntity(i);
        ActionComponent *component = static_cast<ActionComponent *>(entity->GetComponent(Component::ACTION_COMPONENT));
        if (!component)
            continue;
        const uint32 nEvents = component->GetCount();
        for (uint32 componentIdx = 0; componentIdx < nEvents; componentIdx++)
        {
            ActionComponent::Action& act = component->Get(componentIdx);
            if (act.eventType == ActionComponent::Action::EVENT_CUSTOM)
            {
                nameSet.insert(QString(act.userEventId.c_str()));
            }
        }
    }

    const QStringList& nameList = nameSet.toList();
    autocompleteModel->setStringList(nameList);

    const bool enable = selection.Size() > 0;
    setEnabled(enable);
}
