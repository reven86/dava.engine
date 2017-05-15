#include "ui_PropertiesWidget.h"
#include "UI/Properties/PropertiesWidget.h"
#include "UI/Properties/PropertiesModel.h"
#include "UI/Properties/PropertiesWidgetData.h"

#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/LegacySupportModule/Private/Project.h"

#include "UI/Properties/PropertiesTreeItemDelegate.h"
#include "UI/QtModelPackageCommandExecutor.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include "Model/ControlProperties/StyleSheetProperty.h"
#include "Model/ControlProperties/StyleSheetSelectorProperty.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Utils/QtDavaConvertion.h"

#include <TArc/Core/FieldBinder.h>

#include <UI/Components/UIComponent.h>
#include <UI/UIControl.h>
#include <UI/Styles/UIStyleSheetPropertyDataBase.h>
#include <Engine/Engine.h>
#include <Entity/ComponentManager.h>

#include <QAbstractItemModel>
#include <QItemEditorFactory>
#include <QStyledItemDelegate>
#include <QMenu>
#include <QItemSelection>

using namespace DAVA;

namespace
{
String GetPathFromIndex(QModelIndex index)
{
    QString path = index.data().toString();
    while (index.parent().isValid())
    {
        index = index.parent();
        path += "/" + index.data().toString();
    }
    return path.toStdString();
}
}

PropertiesWidget::PropertiesWidget(QWidget* parent)
    : QDockWidget(parent)
    , nodeUpdater(300)
{
    setupUi(this);

    nodeUpdater.SetUpdater(MakeFunction(this, &PropertiesWidget::UpdateModelInternal));
    nodeUpdater.SetStopper([this]() { return selectedNode == nullptr; });

    propertiesModel = new PropertiesModel(treeView);
    propertiesItemsDelegate = new PropertiesTreeItemDelegate(this);
    treeView->setModel(propertiesModel);
    connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PropertiesWidget::OnSelectionChanged);
    connect(propertiesModel, &PropertiesModel::ComponentAdded, this, &PropertiesWidget::OnComponentAdded);

    treeView->setItemDelegate(propertiesItemsDelegate);

    addComponentAction = CreateAddComponentAction();
    treeView->addAction(addComponentAction);

    addStylePropertyAction = CreateAddStylePropertyAction();
    treeView->addAction(addStylePropertyAction);

    addStyleSelectorAction = CreateAddStyleSelectorAction();
    treeView->addAction(addStyleSelectorAction);

    treeView->addAction(CreateSeparator());

    removeAction = CreateRemoveAction();
    treeView->addAction(removeAction);

    connect(treeView, &QTreeView::expanded, this, &PropertiesWidget::OnExpanded);
    connect(treeView, &QTreeView::collapsed, this, &PropertiesWidget::OnCollapsed);
    DVASSERT(nullptr == selectedNode);
    UpdateModel(nullptr);
}

PropertiesWidget::~PropertiesWidget()
{
    nodeUpdater.Abort();
}

void PropertiesWidget::SetAccessor(DAVA::TArc::ContextAccessor* accessor_)
{
    accessor = accessor_;
    BindFields();

    propertiesModel->SetAccessor(accessor);
}

void PropertiesWidget::SetProject(const Project* project)
{
    propertiesItemsDelegate->SetProject(project);
}

void PropertiesWidget::OnAddComponent(QAction* action)
{
    QtModelPackageCommandExecutor executor(accessor);
    DVASSERT(accessor->GetActiveContext() != nullptr);
    const RootProperty* rootProperty = DAVA::DynamicTypeCheck<const RootProperty*>(propertiesModel->GetRootProperty());

    const Type* componentType = action->data().value<Any>().Cast<const Type*>();
    ComponentPropertiesSection* componentSection = rootProperty->FindComponentPropertiesSection(componentType, 0);
    if (componentSection != nullptr && !UIComponent::IsMultiple(componentType))
    {
        QModelIndex index = propertiesModel->indexByProperty(componentSection);
        OnComponentAdded(index);
    }
    else
    {
        executor.AddComponent(DynamicTypeCheck<ControlNode*>(selectedNode), componentType);
    }
}

void PropertiesWidget::OnRemove()
{
    QtModelPackageCommandExecutor executor(accessor);
    DVASSERT(accessor->GetActiveContext() != nullptr);

    QModelIndexList indices = treeView->selectionModel()->selectedIndexes();
    if (!indices.empty())
    {
        const QModelIndex& index = indices.first();
        AbstractProperty* property = static_cast<AbstractProperty*>(index.internalPointer());

        if ((property->GetFlags() & AbstractProperty::EF_CAN_REMOVE) != 0)
        {
            ComponentPropertiesSection* section = dynamic_cast<ComponentPropertiesSection*>(property);
            if (section)
            {
                executor.RemoveComponent(DynamicTypeCheck<ControlNode*>(selectedNode), section->GetComponentType(), section->GetComponentIndex());
            }
            else
            {
                StyleSheetProperty* styleProperty = dynamic_cast<StyleSheetProperty*>(property);
                if (styleProperty)
                {
                    executor.RemoveStyleProperty(DynamicTypeCheck<StyleSheetNode*>(selectedNode), styleProperty->GetPropertyIndex());
                }
                else
                {
                    StyleSheetSelectorProperty* selectorProperty = dynamic_cast<StyleSheetSelectorProperty*>(property);
                    if (selectorProperty)
                    {
                        int32 index = property->GetParent()->GetIndex(selectorProperty);
                        if (index != -1)
                        {
                            executor.RemoveStyleSelector(DynamicTypeCheck<StyleSheetNode*>(selectedNode), index);
                        }
                    }
                }
            }
        }
    }
    UpdateActions();
}

void PropertiesWidget::OnAddStyleProperty(QAction* action)
{
    QtModelPackageCommandExecutor executor(accessor);
    DVASSERT(accessor->GetActiveContext() != nullptr);

    uint32 propertyIndex = action->data().toUInt();
    if (propertyIndex < UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT)
    {
        executor.AddStyleProperty(DynamicTypeCheck<StyleSheetNode*>(selectedNode), propertyIndex);
    }
    else
    {
        DVASSERT(propertyIndex < UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT);
    }
}

void PropertiesWidget::OnAddStyleSelector()
{
    QtModelPackageCommandExecutor executor(accessor);
    DVASSERT(accessor->GetActiveContext() != nullptr);
    executor.AddStyleSelector(DynamicTypeCheck<StyleSheetNode*>(selectedNode));
}

void PropertiesWidget::OnSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
    UpdateActions();
}

QAction* PropertiesWidget::CreateAddComponentAction()
{
    QMenu* addComponentMenu = new QMenu(this);
    auto& components = GetEngineContext()->componentManager->GetRegisteredComponents();
    for (auto& c : components)
    {
        if (!ComponentPropertiesSection::IsHiddenComponent(c))
        {
            const String& name = ReflectedTypeDB::GetByType(c)->GetPermanentName();
            QAction* componentAction = new QAction(name.c_str(), this); // TODO: Localize name
            componentAction->setData(QVariant::fromValue(Any(c)));
            addComponentMenu->addAction(componentAction);
        }
    }
    connect(addComponentMenu, &QMenu::triggered, this, &PropertiesWidget::OnAddComponent);

    QAction* action = new QAction(tr("Add Component"), this);
    action->setMenu(addComponentMenu);
    addComponentMenu->setEnabled(false);
    return action;
}

QAction* PropertiesWidget::CreateAddStyleSelectorAction()
{
    QAction* action = new QAction(tr("Add Style Selector"), this);
    connect(action, &QAction::triggered, this, &PropertiesWidget::OnAddStyleSelector);
    action->setEnabled(false);
    return action;
}

QAction* PropertiesWidget::CreateAddStylePropertyAction()
{
    QMenu* propertiesMenu = new QMenu(this);
    QMenu* groupMenu = nullptr;
    UIStyleSheetPropertyGroup* prevGroup = nullptr;
    UIStyleSheetPropertyDataBase* db = UIStyleSheetPropertyDataBase::Instance();
    for (int32 i = 0; i < UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT; i++)
    {
        const UIStyleSheetPropertyDescriptor& descr = db->GetStyleSheetPropertyByIndex(i);
        if (descr.group != prevGroup)
        {
            prevGroup = descr.group;
            if (descr.group->prefix.empty())
            {
                groupMenu = propertiesMenu;
            }
            else
            {
                groupMenu = new QMenu(QString::fromStdString(descr.group->prefix), this);
                propertiesMenu->addMenu(groupMenu);
            }
        }
        QAction* componentAction = new QAction(descr.name.c_str(), this);
        componentAction->setData(i);

        groupMenu->addAction(componentAction);
    }
    connect(propertiesMenu, &QMenu::triggered, this, &PropertiesWidget::OnAddStyleProperty);

    QAction* action = new QAction(tr("Add Style Property"), this);
    action->setMenu(propertiesMenu);
    propertiesMenu->setEnabled(false);
    return action;
}

QAction* PropertiesWidget::CreateRemoveAction()
{
    QAction* action = new QAction(tr("Remove"), this);
    connect(action, &QAction::triggered, this, &PropertiesWidget::OnRemove);
    action->setEnabled(false);
    return action;
}

QAction* PropertiesWidget::CreateSeparator()
{
    QAction* separator = new QAction(this);
    separator->setSeparator(true);
    return separator;
}

void PropertiesWidget::OnModelUpdated()
{
    bool blocked = treeView->blockSignals(true);
    treeView->expandToDepth(0);
    treeView->blockSignals(blocked);
    treeView->resizeColumnToContents(0);
    ApplyExpanding();
}

void PropertiesWidget::OnExpanded(const QModelIndex& index)
{
    itemsState[GetPathFromIndex(index)] = true;
}

void PropertiesWidget::OnCollapsed(const QModelIndex& index)
{
    itemsState[GetPathFromIndex(index)] = false;
}

void PropertiesWidget::OnComponentAdded(const QModelIndex& index)
{
    treeView->expand(index);
    treeView->setCurrentIndex(index);
    int rowCount = propertiesModel->rowCount(index);
    if (rowCount > 0)
    {
        QModelIndex lastChildIndex = index.child(rowCount - 1, 0);
        treeView->scrollTo(lastChildIndex, QAbstractItemView::EnsureVisible);
    }
    treeView->scrollTo(index, QAbstractItemView::EnsureVisible);
}

void PropertiesWidget::UpdateModel(PackageBaseNode* node)
{
    if (node == selectedNode)
    {
        return;
    }
    selectedNode = node;
    nodeUpdater.Update();
}

void PropertiesWidget::UpdateModelInternal()
{
    if (nullptr != selectedNode)
    {
        auto index = treeView->indexAt(QPoint(0, 0));
        lastTopIndexPath = GetPathFromIndex(index);
    }
    propertiesModel->Reset(selectedNode);
    bool isControl = dynamic_cast<ControlNode*>(selectedNode) != nullptr;
    bool isStyle = dynamic_cast<StyleSheetNode*>(selectedNode) != nullptr;
    addComponentAction->menu()->setEnabled(isControl);
    addStylePropertyAction->menu()->setEnabled(isStyle);
    addStyleSelectorAction->setEnabled(isStyle);
    removeAction->setEnabled(false);

    OnModelUpdated();
}

void PropertiesWidget::UpdateActions()
{
    QModelIndexList indices = treeView->selectionModel()->selectedIndexes();
    if (!indices.empty())
    {
        AbstractProperty* property = static_cast<AbstractProperty*>(indices.first().internalPointer());
        removeAction->setEnabled((property->GetFlags() & AbstractProperty::EF_CAN_REMOVE) != 0);
    }
}

void PropertiesWidget::ApplyExpanding()
{
    QModelIndex index = propertiesModel->index(0, 0);
    while (index.isValid())
    {
        const auto& path = GetPathFromIndex(index);
        if (path == lastTopIndexPath)
        {
            treeView->scrollTo(index, QTreeView::PositionAtTop);
        }
        auto iter = itemsState.find(path);
        if (iter != itemsState.end())
        {
            treeView->setExpanded(index, iter->second);
        }

        index = treeView->indexBelow(index);
    }
}

void PropertiesWidget::BindFields()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    fieldBinder.reset(new FieldBinder(accessor));
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::packagePropertyName);
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &PropertiesWidget::OnPackageDataChanged));
    }
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::selectionPropertyName);
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &PropertiesWidget::OnSelectionDataChanged));
    }
}

void PropertiesWidget::OnPackageDataChanged(const DAVA::Any& package)
{
    treeView->setEnabled(package.CanGet<PackageNode*>() && (package.Get<PackageNode*>() != nullptr));
}

void PropertiesWidget::OnSelectionDataChanged(const DAVA::Any& selectionData)
{
    using namespace DAVA;
    using namespace DAVA::TArc;
    DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext == nullptr)
    {
        UpdateModel(nullptr);
        return;
    }

    //prepare selection
    SelectedNodes selection = selectionData.Cast<SelectedNodes>(SelectedNodes());
    for (auto iter = selection.begin(); iter != selection.end();)
    {
        PackageBaseNode* node = *iter;
        PackageBaseNode* parent = node->GetParent();
        if (parent == nullptr || parent->GetParent() == nullptr)
        {
            iter = selection.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    PropertiesWidgetData* widgetData = activeContext->GetData<PropertiesWidgetData>();

    SelectedNodes& cachedSelection = widgetData->cachedSelection;
    List<PackageBaseNode*>& selectionHistory = widgetData->selectionHistory;

    if (selection.empty())
    {
        selectionHistory.clear();
        cachedSelection.clear();
        UpdateModel(nullptr);
        return;
    }

    SortedPackageBaseNodeSet newSelection(CompareByLCA);
    std::set_difference(selection.begin(),
                        selection.end(),
                        cachedSelection.begin(),
                        cachedSelection.end(),
                        std::inserter(newSelection, newSelection.end()));

    if (newSelection.empty())
    {
        SelectedNodes removedSelection;
        std::set_difference(cachedSelection.begin(),
                            cachedSelection.end(),
                            selection.begin(),
                            selection.end(),
                            std::inserter(removedSelection, removedSelection.end()));

        for (PackageBaseNode* node : removedSelection)
        {
            selectionHistory.remove(node);
        }
        UpdateModel(selectionHistory.back());
    }
    else
    {
        //take any node from new selection. If this node is higher than cached selection top node, display properties for most top node from new selection
        //otherwise if this node is lower than cached selection bottom node, display properties for most bottom node from new selection
        PackageBaseNode* newSelectedNode = *newSelection.begin();
        DVASSERT(newSelectedNode != nullptr);

        bool selectionAddedToTop = true;
        if (cachedSelection.empty() == false)
        {
            PackageBaseNode* cachedTopNode = *cachedSelection.begin();
            selectionAddedToTop = CompareByLCA(newSelectedNode, cachedTopNode);
        }

        if (selectionAddedToTop)
        {
            for (auto reverseIter = newSelection.rbegin(); reverseIter != newSelection.rend(); ++reverseIter)
            {
                selectionHistory.push_back(*reverseIter);
            }
            UpdateModel(newSelectedNode);
        }
        else
        {
            for (PackageBaseNode* node : newSelection)
            {
                selectionHistory.push_back(node);
            }
            UpdateModel(*newSelection.rbegin());
        }
    }
    cachedSelection = selection;
}
