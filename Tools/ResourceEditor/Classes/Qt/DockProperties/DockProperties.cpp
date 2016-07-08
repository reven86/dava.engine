#include "DockProperties.h"
#include "Main/mainwindow.h"
#include <QComboBox>

DockProperties::DockProperties(QWidget* parent /* = NULL */)
    : QDockWidget(parent)
    , addComponentMenu(new QMenu(this))
{
}

DockProperties::~DockProperties()
{
}

void DockProperties::Init()
{
    Ui::MainWindow* ui = QtMainWindow::Instance()->GetUI();

    // toggle propertyEditor view mode
    QComboBox* viewModes = new QComboBox();
    viewModes->addItem("Basic", (int)PropertyEditor::VIEW_NORMAL);
    viewModes->addItem("Advanced", (int)PropertyEditor::VIEW_ADVANCED);
    viewModes->addItem("Favorites only", (int)PropertyEditor::VIEW_FAVORITES_ONLY);

    ui->propertiesToolBar->addSeparator();
    ui->propertiesToolBar->addWidget(viewModes);
    QObject::connect(viewModes, SIGNAL(activated(int)), this, SLOT(ViewModeSelected(int)));

    ui->propertyEditor->SetViewMode(PropertyEditor::VIEW_ADVANCED);
    viewModes->setCurrentIndex(1);

    // toggle favorites edit mode
    QObject::connect(ui->actionFavoritesEdit, SIGNAL(triggered()), this, SLOT(ActionFavoritesEdit()));
    ui->propertyEditor->SetFavoritesEditMode(ui->actionFavoritesEdit->isChecked());

    // Add components
    addComponentMenu->addAction(ui->actionAddActionComponent);
    addComponentMenu->addAction(ui->actionAddQualitySettingsComponent);
    addComponentMenu->addAction(ui->actionAddStaticOcclusionComponent);
    addComponentMenu->addAction(ui->actionAddSoundComponent);
    addComponentMenu->addAction(ui->actionAddWaveComponent);
    addComponentMenu->addAction(ui->actionAddSkeletonComponent);
    addComponentMenu->addAction(ui->actionAddPathComponent);
    addComponentMenu->addAction(ui->actionAddRotationComponent);
    addComponentMenu->addAction(ui->actionAddSnapToLandscapeComponent);
    addComponentMenu->addAction(ui->actionAddWASDComponent);
    addComponentMenu->addAction(ui->actionAddVisibilityComponent);

    connect(ui->actionAddNewComponent, SIGNAL(triggered()), SLOT(OnAddAction()));
}

void DockProperties::ActionFavoritesEdit()
{
    QAction* favoritesEditAction = dynamic_cast<QAction*>(QObject::sender());
    if (NULL != favoritesEditAction)
    {
        QtMainWindow::Instance()->GetUI()->propertyEditor->SetFavoritesEditMode(favoritesEditAction->isChecked());
    }
}

void DockProperties::ViewModeSelected(int index)
{
    QComboBox* viewModes = dynamic_cast<QComboBox*>(QObject::sender());

    if (NULL != viewModes)
    {
        PropertyEditor::eViewMode mode = (PropertyEditor::eViewMode)viewModes->itemData(index).toInt();
        QtMainWindow::Instance()->GetUI()->propertyEditor->SetViewMode(mode);
    }
}

void DockProperties::OnAddAction()
{
    Ui::MainWindow* ui = QtMainWindow::Instance()->GetUI();
    QWidget* w = ui->propertiesToolBar->widgetForAction(ui->actionAddNewComponent);
    QPoint pos = QCursor::pos();

    if (w != NULL)
    {
        pos = w->mapToGlobal(QPoint(0, w->height()));
    }

    addComponentMenu->exec(pos, ui->actionAddNewComponent);
}
