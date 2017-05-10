#include "TArc/Controls/PropertyPanel/PropertiesView.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/Private/PropertiesViewDelegate.h"
#include "TArc/Controls/ComboBox.h"
#include "TArc/Controls/CheckBox.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/Utils/ScopedValueGuard.h"

#include <QtTools/WidgetHelpers/SharedIcon.h>

#include <Reflection/Reflection.h>
#include <Base/BaseTypes.h>

#include <QVBoxLayout>
#include <QTreeView>
#include <QScrollBar>
#include <QHeaderView>
#include <QTimer>
#include <QPainter>
#include <QToolBar>
#include <QPersistentModelIndex>
#include <QtWidgets/private/qheaderview_p.h>

namespace DAVA
{
namespace TArc
{
namespace PropertiesViewDetail
{
const char* SeparatorPositionKey = "SeparatorPosition";
const char* isFavoritesViewOnlyKey = "isFavoritesViewOnly";
const int ToolBarHeight = 34;
const int FavoritesStarSpaceWidth = 20;

class PropertiesHeaderView : public QHeaderView
{
public:
    PropertiesHeaderView(Qt::Orientation orientation, QWidget* parent)
        : QHeaderView(orientation, parent)
    {
    }

    void SetFavoritesEditMode(bool isActive)
    {
        isInFavoritesEdit = isActive;
        setOffset(isInFavoritesEdit == true ? -FavoritesStarSpaceWidth : 0);
    }

protected:
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override
    {
        QRect r = rect;
        if (isInFavoritesEdit == true && logicalIndex == 0)
        {
            r.setX(0);
        }

        QHeaderView::paintSection(painter, r, logicalIndex);
    }

private:
    bool isInFavoritesEdit = false;
};

String DeveloperModeDescription(const Any& v)
{
    if (v.Get<bool>() == true)
        return "Developer mode on";
    else
        return "Developer mode off";
}

} // namespace PropertiesViewDetail

class PropertiesView::PropertiesTreeView : public QTreeView
{
public:
    PropertiesTreeView(QWidget* parent)
        : QTreeView(parent)
    {
        headerView = new PropertiesViewDetail::PropertiesHeaderView(Qt::Horizontal, this);
        setHeader(headerView);
        headerView->setStretchLastSection(true);
        setMouseTracking(true);
    }

    void setModel(QAbstractItemModel* model) override
    {
        QTreeView::setModel(model);
        propertiesModel = qobject_cast<ReflectedPropertyModel*>(model);
    }

    void SetFavoritesEditMode(bool inEditMode)
    {
        isInFavoritesEdit = inEditMode;
        headerView->SetFavoritesEditMode(inEditMode);
        viewport()->update();
        headerView->update();
    }

protected:
    void drawRow(QPainter* painter, const QStyleOptionViewItem& options, const QModelIndex& index) const override
    {
        QTreeView::drawRow(painter, options, index);

        QColor gridColor = options.palette.color(QPalette::Normal, QPalette::Window);

        painter->save();
        // draw horizontal bottom line
        painter->setPen(gridColor);
        painter->drawLine(options.rect.bottomLeft(), options.rect.bottomRight());

        // draw vertical line
        bool isSelected = options.state & QStyle::State_Selected;
        bool isSpanned = isFirstColumnSpanned(index.row(), index.parent());
        if (isSelected == false && isSpanned == false)
        {
            if (headerView != nullptr && headerView->count() > 1)
            {
                int sz = headerView->sectionSize(0) - headerView->offset();
                QScrollBar* scroll = horizontalScrollBar();
                if (scroll != nullptr)
                {
                    sz -= scroll->value();
                }

                QPoint p1 = options.rect.topLeft();
                QPoint p2 = options.rect.bottomLeft();

                p1.setX(p1.x() + sz - 1);
                p2.setX(p2.x() + sz - 1);

                painter->setPen(gridColor);
                painter->drawLine(p1, p2);
            }
        }

        painter->restore();

        if (isInFavoritesEdit == true)
        {
            bool isFavotire = propertiesModel->IsFavorite(index);
            QRect iconRect = QRect(options.rect.x(), options.rect.y(), PropertiesViewDetail::FavoritesStarSpaceWidth, options.rect.height());

            if (isFavotire)
            {
                SharedIcon(":/QtIcons/star.png").paint(painter, iconRect);
            }
            else if (propertiesModel->IsInFavoriteHierarchy(index) == false)
            {
                SharedIcon(":/QtIcons/star_empty.png").paint(painter, iconRect);
            }
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (isInFavoritesEdit)
        {
            QPoint normalizedEventPos = event->pos();
            normalizedEventPos.rx() += -headerView->offset();
            QModelIndex index = indexAt(normalizedEventPos);

            if (index.isValid() && index.column() == 0)
            {
                QRect rect = visualRect(index);
                rect.setX(0);
                rect.setWidth(-headerView->offset());

                if (rect.contains(event->pos()))
                {
                    if (propertiesModel->IsFavorite(index))
                    {
                        propertiesModel->RemoveFavorite(index);
                    }
                    else if (propertiesModel->IsInFavoriteHierarchy(index) == false)
                    {
                        propertiesModel->AddFavorite(index);
                    }

                    viewport()->update();
                }
            }
        }
        QTreeView::mouseReleaseEvent(event);
    }

    bool edit(const QModelIndex& index, EditTrigger trigger, QEvent* event) override
    {
        if (trigger == SelectedClicked)
        {
            return QTreeView::edit(index, QAbstractItemView::EditKeyPressed, event);
        }

        return QTreeView::edit(index, trigger, event);
    }

private:
    PropertiesViewDetail::PropertiesHeaderView* headerView = nullptr;
    bool isInFavoritesEdit = false;
    ReflectedPropertyModel* propertiesModel = nullptr;
};

DAVA_REFLECTION_IMPL(PropertiesView)
{
    ReflectionRegistrator<PropertiesView>::Begin()
    .Field("viewMode", &PropertiesView::GetViewMode, &PropertiesView::SetViewMode)[M::EnumT<eViewMode>()]
    .Field("devMode", &PropertiesView::IsInDeveloperMode, &PropertiesView::SetDeveloperMode)[M::ValueDescription(&PropertiesViewDetail::DeveloperModeDescription)]
    .End();
}

PropertiesView::PropertiesView(const Params& params_)
    : binder(params_.accessor)
    , params(params_)
{
    binder.BindField(params.objectsField, MakeFunction(this, &PropertiesView::OnObjectsChanged));
    model.reset(new ReflectedPropertyModel(params.wndKey, params.accessor, params.invoker, params.ui));

    SetupUI();

    std::shared_ptr<Updater> lockedUpdater = params.updater.lock();
    if (lockedUpdater != nullptr)
    {
        lockedUpdater->update.Connect(this, &PropertiesView::Update);
    }

    PropertiesItem viewItem = params.accessor->CreatePropertiesNode(params.settingsNodeName);
    int columnWidth = viewItem.Get(PropertiesViewDetail::SeparatorPositionKey, view->columnWidth(0));
    view->setColumnWidth(0, columnWidth);

    model->LoadState(viewItem);
    viewMode = static_cast<eViewMode>(viewItem.Get(PropertiesViewDetail::isFavoritesViewOnlyKey, static_cast<int32>(VIEW_MODE_NORMAL)));

    QObject::connect(view, &QTreeView::expanded, this, &PropertiesView::OnExpanded);
    QObject::connect(view, &QTreeView::collapsed, this, &PropertiesView::OnCollapsed);
    QObject::connect(view->selectionModel(), &QItemSelectionModel::currentChanged, this, &PropertiesView::OnCurrentChanged);

    model->SetDeveloperMode(params.isInDevMode);
}

PropertiesView::~PropertiesView()
{
    std::shared_ptr<Updater> lockedUpdater = params.updater.lock();
    if (lockedUpdater != nullptr)
    {
        lockedUpdater->update.Disconnect(this);
    }

    PropertiesItem viewSettings = params.accessor->CreatePropertiesNode(params.settingsNodeName);
    viewSettings.Set(PropertiesViewDetail::SeparatorPositionKey, view->columnWidth(0));
    viewSettings.Set(PropertiesViewDetail::isFavoritesViewOnlyKey, static_cast<int32>(viewMode));

    model->SaveState(viewSettings);
}

void PropertiesView::RegisterExtension(const std::shared_ptr<ExtensionChain>& extension)
{
    model->RegisterExtension(extension);
}

void PropertiesView::UnregisterExtension(const std::shared_ptr<ExtensionChain>& extension)
{
    model->UnregisterExtension(extension);
}

void PropertiesView::SetupUI()
{
    // Main layout setup.
    // Toolbar + TreeView
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    QToolBar* toolBar = new QToolBar(this);
    toolBar->setFixedHeight(PropertiesViewDetail::ToolBarHeight);
    layout->addWidget(toolBar);

    view = new PropertiesTreeView(this);
    view->setObjectName(QString("%1_propertiesview").arg(QString::fromStdString(params.settingsNodeName)));
    view->setEditTriggers(QAbstractItemView::CurrentChanged | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);
    layout->addWidget(view);

    view->setModel(model.get());
    view->setRootIndex(QModelIndex());
    view->setItemDelegate(new PropertiesViewDelegate(view, model.get(), this));

    // Toolbar setup
    QAction* favoriteModeAction = new QAction(toolBar);
    favoriteModeAction->setCheckable(true);
    QIcon icon;
    icon.addFile(QStringLiteral(":/QtIcons/star.png"), QSize(), QIcon::Normal, QIcon::Off);
    favoriteModeAction->setIcon(icon);
    toolBar->addAction(favoriteModeAction);
    connections.AddConnection(favoriteModeAction, &QAction::toggled, MakeFunction(this, &PropertiesView::OnFavoritesEditChanged));

    Reflection thisModel = Reflection::Create(ReflectedObject(this));
    {
        ComboBox::Params controlParams(params.accessor, params.ui, params.wndKey);
        controlParams.fields[ComboBox::Fields::Value] = "viewMode";
        ComboBox* comboBox = new ComboBox(controlParams, params.accessor, thisModel, toolBar);
        toolBar->addWidget(comboBox->ToWidgetCast());
    }

    {
        CheckBox::Params controlParams(params.accessor, params.ui, params.wndKey);
        controlParams.fields[CheckBox::Fields::Checked] = "devMode";
        CheckBox* checkBox = new CheckBox(controlParams, params.accessor, thisModel, toolBar);
        toolBar->addWidget(checkBox->ToWidgetCast());
    }

    QHeaderView* headerView = view->header();
    connections.AddConnection(headerView, &QHeaderView::sectionResized, MakeFunction(this, &PropertiesView::OnColumnResized));
}

void PropertiesView::OnObjectsChanged(const Any& objects)
{
    view->setRootIndex(QModelIndex());
    if (objects.IsEmpty())
    {
        model->SetObjects(Vector<Reflection>());
    }
    else
    {
        DVASSERT(objects.CanCast<Vector<Reflection>>());
        model->SetObjects(objects.Cast<Vector<Reflection>>());
    }
    UpdateViewRootIndex();
    UpdateExpanded();
}

void PropertiesView::OnColumnResized(int columnIndex, int oldSize, int newSize)
{
    PropertiesViewDelegate* d = qobject_cast<PropertiesViewDelegate*>(view->itemDelegate());
    DVASSERT(d != nullptr);
    d->UpdateSizeHints(columnIndex, newSize);
}

void PropertiesView::Update(UpdatePolicy policy)
{
    ScopedValueGuard<bool> guard(isModelUpdate, true);

    switch (policy)
    {
    case DAVA::TArc::PropertiesView::FullUpdate:
        model->Update();
        break;
    case DAVA::TArc::PropertiesView::FastUpdate:
        model->UpdateFast();
        break;
    default:
        DVASSERT(false, "Unimplemented update policy have been received");
        break;
    }

    UpdateExpanded();
    QModelIndex currentIndex = model->LookIndex(currentIndexPath);
    if (currentIndex.isValid())
    {
        QModelIndex viewCurrent = view->currentIndex();
        if (currentIndex.row() != viewCurrent.row() || currentIndex.internalPointer() != viewCurrent.internalPointer())
        {
            view->setCurrentIndex(currentIndex);
        }
    }
    else
    {
        view->clearSelection();
    }
}

void PropertiesView::UpdateExpanded()
{
    ScopedValueGuard<bool> guard(isExpandUpdate, true);
    QModelIndexList expandedList = model->GetExpandedList(view->rootIndex());
    foreach (const QModelIndex& index, expandedList)
    {
        view->expand(index);
    }
}

void PropertiesView::OnExpanded(const QModelIndex& index)
{
    SCOPED_VALUE_GUARD(bool, isExpandUpdate, true, void());
    model->SetExpanded(true, index);

    QModelIndexList expandedList = model->GetExpandedChildren(index);
    foreach (const QModelIndex& index, expandedList)
    {
        view->expand(index);
    }
}

void PropertiesView::OnCollapsed(const QModelIndex& index)
{
    SCOPED_VALUE_GUARD(bool, isExpandUpdate, true, void());
    model->SetExpanded(false, index);
}

void PropertiesView::OnFavoritesEditChanged(bool isChecked)
{
    view->SetFavoritesEditMode(isChecked);
}

PropertiesView::eViewMode PropertiesView::GetViewMode() const
{
    return viewMode;
}

void PropertiesView::SetViewMode(PropertiesView::eViewMode mode)
{
    view->setRootIndex(QModelIndex());
    viewMode = mode;
    UpdateViewRootIndex();
    UpdateExpanded();
}

bool PropertiesView::IsInDeveloperMode() const
{
    return model->IsDeveloperMode();
}

void PropertiesView::SetDeveloperMode(bool isDevMode)
{
    if (isDevMode == true)
    {
        ModalMessageParams p;
        p.defaultButton = ModalMessageParams::Cancel;
        p.icon = ModalMessageParams::Warning;
        p.title = "Switch to developer mode";
        p.message = "You are trying to switch into developer mode. It can be unsafe.\nAre you sure?";
        if (params.ui->ShowModalMessage(DAVA::TArc::mainWindowKey, p) == ModalMessageParams::Cancel)
        {
            return;
        }
    }

    model->SetDeveloperMode(isDevMode);
}

void PropertiesView::UpdateViewRootIndex()
{
    QModelIndex newRootIndex = model->GetRegularRootIndex();
    if (viewMode == VIEW_MODE_FAVORITES_ONLY)
    {
        newRootIndex = model->GetFavoriteRootIndex();
    }

    view->setRootIndex(newRootIndex);
}

void PropertiesView::OnCurrentChanged(const QModelIndex& index, const QModelIndex& prev)
{
    if (isModelUpdate == false)
    {
        currentIndexPath = model->GetIndexPath(index);
    }
}

} // namespace TArc
} // namespace DAVA

ENUM_DECLARE(DAVA::TArc::PropertiesView::eViewMode)
{
    ENUM_ADD_DESCR(DAVA::TArc::PropertiesView::eViewMode::VIEW_MODE_NORMAL, "Normal");
    ENUM_ADD_DESCR(DAVA::TArc::PropertiesView::eViewMode::VIEW_MODE_FAVORITES_ONLY, "Favorites only");
}
