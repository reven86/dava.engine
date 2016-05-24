#include "MaterialTree.h"
#include "MaterialFilterModel.h"
#include "Main/mainwindow.h"
#include "Scene/SceneSignals.h"
#include "MaterialEditor/MaterialAssignSystem.h"
#include "QtTools/WidgetHelpers/SharedIcon.h"

#include "Classes/Commands2/RemoveComponentCommand.h"
#include "Entity/Component.h"

#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QHeaderView>

MaterialTree::MaterialTree(QWidget* parent /* = 0 */)
    : QTreeView(parent)
{
    treeModel = new MaterialFilteringModel(new MaterialModel(this));
    setModel(treeModel);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setIconSize(QSize(24, 24));

    QObject::connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&)));

    QObject::connect(SceneSignals::Instance(), SIGNAL(CommandExecuted(SceneEditor2*, const Command2*, bool)), this, SLOT(OnCommandExecuted(SceneEditor2*, const Command2*, bool)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(StructureChanged(SceneEditor2*, DAVA::Entity*)), this, SLOT(OnStructureChanged(SceneEditor2*, DAVA::Entity*)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(SelectionChanged(SceneEditor2*, const SelectableGroup*, const SelectableGroup*)), this, SLOT(OnSelectionChanged(SceneEditor2*, const SelectableGroup*, const SelectableGroup*)));

    header()->setSortIndicator(0, Qt::AscendingOrder);
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    header()->setSectionResizeMode(1, QHeaderView::Fixed);
    header()->setSectionResizeMode(2, QHeaderView::Fixed);
    header()->resizeSection(1, 35);
    header()->resizeSection(2, 35);
}

MaterialTree::~MaterialTree()
{
}

void MaterialTree::SetScene(SceneEditor2* sceneEditor)
{
    setSortingEnabled(false);
    treeModel->SetScene(sceneEditor);

    if (nullptr != sceneEditor)
    {
        OnSelectionChanged(sceneEditor, &sceneEditor->selectionSystem->GetSelection(), nullptr);
    }
    else
    {
        treeModel->SetSelection(nullptr);
    }

    sortByColumn(0);
    setSortingEnabled(true);
}

DAVA::NMaterial* MaterialTree::GetMaterial(const QModelIndex& index) const
{
    return treeModel->GetMaterial(index);
}

void MaterialTree::SelectMaterial(DAVA::NMaterial* material)
{
    selectionModel()->clear();

    QModelIndex index = treeModel->GetIndex(material);
    if (index.isValid())
    {
        selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        scrollTo(index, QAbstractItemView::PositionAtCenter);
    }
}

void MaterialTree::SelectEntities(const QList<DAVA::NMaterial*>& materials)
{
    SceneEditor2* curScene = QtMainWindow::Instance()->GetCurrentScene();

    if (nullptr != curScene && materials.size() > 0)
    {
        std::function<void(DAVA::NMaterial*)> fn = [&fn, &curScene](DAVA::NMaterial* material) {
            DAVA::Entity* entity = curScene->materialSystem->GetEntity(material);
            if (nullptr != entity)
            {
                curScene->selectionSystem->AddObjectToSelection(curScene->selectionSystem->GetSelectableEntity(entity));
            }
            const DAVA::Vector<DAVA::NMaterial*>& children = material->GetChildren();
            for (auto child : children)
            {
                fn(child);
            }
        };

        curScene->selectionSystem->Clear();
        for (int i = 0; i < materials.size(); i++)
        {
            DAVA::NMaterial* material = materials.at(i);
            fn(material);
        }

        LookAtSelection(curScene);
    }
}

void MaterialTree::Update()
{
    treeModel->Sync();
    treeModel->invalidate();
    emit Updated();
}

int MaterialTree::getFilterType() const
{
    return treeModel->getFilterType();
}

void MaterialTree::setFilterType(int filterType)
{
    treeModel->setFilterType(filterType);
}

void MaterialTree::ShowContextMenu(const QPoint& pos)
{
    QMenu contextMenu(this);

    contextMenu.addAction(SharedIcon(":/QtIcons/zoom.png"), "Select entities", this, SLOT(OnSelectEntities()));

    emit ContextMenuPrepare(&contextMenu);
    contextMenu.exec(mapToGlobal(pos));
}

void MaterialTree::dragEnterEvent(QDragEnterEvent* event)
{
    QTreeView::dragEnterEvent(event);
    dragTryAccepted(event);
}

void MaterialTree::dragMoveEvent(QDragMoveEvent* event)
{
    QTreeView::dragMoveEvent(event);
    dragTryAccepted(event);
}

void MaterialTree::dropEvent(QDropEvent* event)
{
    QTreeView::dropEvent(event);

    event->setDropAction(Qt::IgnoreAction);
    event->accept();
}

void MaterialTree::dragTryAccepted(QDragMoveEvent* event)
{
    int row, col;
    QModelIndex parent;

    GetDropParams(event->pos(), parent, row, col);
    if (treeModel->dropCanBeAccepted(event->mimeData(), event->dropAction(), row, col, parent))
    {
        event->setDropAction(Qt::MoveAction);
        event->accept();
        treeModel->invalidate();
    }
    else
    {
        event->setDropAction(Qt::IgnoreAction);
        event->accept();
    }
}

void MaterialTree::GetDropParams(const QPoint& pos, QModelIndex& index, int& row, int& col)
{
    row = -1;
    col = -1;
    index = indexAt(pos);

    switch (dropIndicatorPosition())
    {
    case QAbstractItemView::OnItem:
    case QAbstractItemView::AboveItem:
        row = index.row();
        col = index.column();
        index = index.parent();
        break;
    case QAbstractItemView::BelowItem:
        row = index.row() + 1;
        col = index.column();
        index = index.parent();
        break;
    case QAbstractItemView::OnViewport:
        index = QModelIndex();
        break;
    }
}

void MaterialTree::OnCommandExecuted(SceneEditor2* scene, const Command2* command, bool redo)
{
    if (command == nullptr)
    {
        return;
    }

    if (QtMainWindow::Instance()->GetCurrentScene() == scene)
    {
        if (command->MatchCommandID(CMDID_INSP_MEMBER_MODIFY))
        {
            treeModel->invalidate();
        }
        else if (command->MatchCommandIDs({ CMDID_DELETE_RENDER_BATCH, CMDID_CLONE_LAST_BATCH, CMDID_CONVERT_TO_SHADOW, CMDID_MATERIAL_SWITCH_PARENT,
                                            CMDID_MATERIAL_REMOVE_CONFIG, CMDID_MATERIAL_CREATE_CONFIG, CMDID_LOD_DELETE, CMDID_LOD_CREATE_PLANE, CMDID_LOD_COPY_LAST_LOD }))
        {
            Update();
        }
        else if (command->MatchCommandID(CMDID_COMPONENT_REMOVE))
        {
            auto ProcessRemoveCommand = [this](const Command2* command, bool redo)
            {
                const RemoveComponentCommand* removeCommand = static_cast<const RemoveComponentCommand*>(command);
                DVASSERT(removeCommand->GetComponent() != nullptr);
                if (removeCommand->GetComponent()->GetType() == DAVA::Component::RENDER_COMPONENT)
                {
                    Update();
                }
            };

            if (command->GetId() == CMDID_BATCH)
            {
                const CommandBatch* batch = static_cast<const CommandBatch*>(command);
                const DAVA::uint32 count = batch->Size();
                for (DAVA::uint32 i = 0; i < count; ++i)
                {
                    const Command2* cmd = batch->GetCommand(i);
                    if (cmd->GetId() == CMDID_COMPONENT_REMOVE)
                    {
                        ProcessRemoveCommand(cmd, redo);
                    }
                }
            }
            else
            {
                ProcessRemoveCommand(command, redo);
            }
        }
    }
}

void MaterialTree::OnStructureChanged(SceneEditor2* scene, DAVA::Entity* parent)
{
    treeModel->Sync();
}

void MaterialTree::OnSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected)
{
    if (QtMainWindow::Instance()->GetCurrentScene() == scene)
    {
        treeModel->SetSelection(selected);
        treeModel->invalidate();
    }
}

void MaterialTree::OnSelectEntities()
{
    const QModelIndexList selection = selectionModel()->selectedRows();
    QList<DAVA::NMaterial*> materials;

    materials.reserve(selection.size());
    for (int i = 0; i < selection.size(); i++)
    {
        DAVA::NMaterial* material = treeModel->GetMaterial(selection.at(i));
        if (material != nullptr)
            materials << material;
    }

    SelectEntities(materials);
}
