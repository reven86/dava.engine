#include "DockProperties.h"
#include "PropertyEditor.h"
#include "Main/mainwindow.h"

#include <QComboBox>
#include <QToolBar>
#include <QMenu>

DockProperties::DockProperties(QWidget* parent /* = NULL */)
    : QDockWidget(parent)
    , addComponentMenu(new QMenu(this))
{
}

DockProperties::~DockProperties()
{
}

void DockProperties::Init(Ui::MainWindow* mainwindowUi, const std::shared_ptr<GlobalOperations>& globalOperations)
{
    DVASSERT(mainwindowUi != nullptr);
    toolbar = mainwindowUi->propertiesToolBar;
    propertiesEditor = mainwindowUi->propertyEditor;

    // toggle propertyEditor view mode
    QComboBox* viewModes = new QComboBox();
    QObject::connect(viewModes, SIGNAL(activated(int)), this, SLOT(ViewModeSelected(int)));
    viewModes->addItem("Basic", (int)PropertyEditor::VIEW_NORMAL);
    viewModes->addItem("Advanced", (int)PropertyEditor::VIEW_ADVANCED);
    viewModes->addItem("Favorites only", (int)PropertyEditor::VIEW_FAVORITES_ONLY);
    viewModes->setCurrentIndex(1);

    toolbar->addSeparator();
    toolbar->addWidget(viewModes);

    propertiesEditor->SetViewMode(PropertyEditor::VIEW_ADVANCED);

    // toggle favorites edit mode
    QObject::connect(mainwindowUi->actionFavoritesEdit, SIGNAL(triggered()), this, SLOT(ActionFavoritesEdit()));
    propertiesEditor->SetFavoritesEditMode(mainwindowUi->actionFavoritesEdit->isChecked());

    // Add components
    addComponentMenu->addAction(mainwindowUi->actionAddActionComponent);
    addComponentMenu->addAction(mainwindowUi->actionAddQualitySettingsComponent);
    addComponentMenu->addAction(mainwindowUi->actionAddStaticOcclusionComponent);
    addComponentMenu->addAction(mainwindowUi->actionAddSoundComponent);
    addComponentMenu->addAction(mainwindowUi->actionAddWaveComponent);
    addComponentMenu->addAction(mainwindowUi->actionAddSkeletonComponent);
    addComponentMenu->addAction(mainwindowUi->actionAddPathComponent);
    addComponentMenu->addAction(mainwindowUi->actionAddRotationComponent);
    addComponentMenu->addAction(mainwindowUi->actionAddSnapToLandscapeComponent);
    addComponentMenu->addAction(mainwindowUi->actionAddWASDComponent);
    addComponentMenu->addAction(mainwindowUi->actionAddVisibilityComponent);
    addComponentMenu->addAction(mainwindowUi->actionAddLodComponent);

    connect(mainwindowUi->actionAddNewComponent, SIGNAL(triggered()), SLOT(OnAddAction()));
    propertiesEditor->Init(mainwindowUi, globalOperations);
}

void DockProperties::ActionFavoritesEdit()
{
    QAction* favoritesEditAction = qobject_cast<QAction*>(QObject::sender());
    DVASSERT(favoritesEditAction);
    propertiesEditor->SetFavoritesEditMode(favoritesEditAction->isChecked());
}

void DockProperties::ViewModeSelected(int index)
{
    QComboBox* viewModes = qobject_cast<QComboBox*>(QObject::sender());
    DVASSERT(viewModes);
    propertiesEditor->SetViewMode(static_cast<PropertyEditor::eViewMode>(viewModes->itemData(index).toInt()));
}

void DockProperties::OnAddAction()
{
    QAction* actionAddNewComponent = qobject_cast<QAction*>(QObject::sender());
    DVASSERT(actionAddNewComponent != nullptr);
    QWidget* w = toolbar->widgetForAction(actionAddNewComponent);
    QPoint pos = QCursor::pos();

    if (w != NULL)
    {
        pos = w->mapToGlobal(QPoint(0, w->height()));
    }

    addComponentMenu->exec(pos, actionAddNewComponent);
}
