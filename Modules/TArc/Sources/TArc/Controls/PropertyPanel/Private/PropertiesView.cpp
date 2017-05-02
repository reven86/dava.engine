#include "TArc/Controls/PropertyPanel/PropertiesView.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/Private/PropertiesViewDelegate.h"
#include "TArc/Controls/ComboBox.h"
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
#include <QtWidgets/private/qheaderview_p.h>

namespace DAVA
{
namespace TArc
{
namespace PropertiesViewDetail
{
const char* SeparatorPositionKey = "SeparatorPosition";
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
            const char* iconPath = isFavotire == true ? ":/QtIcons/star.png" : ":/QtIcons/star_empty.png";
            SharedIcon(iconPath).paint(painter, iconRect);
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
                    else
                    {
                        propertiesModel->AddFavorite(index);
                    }
                }
            }
        }
        QTreeView::mouseReleaseEvent(event);
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
    QObject::connect(view, &QTreeView::expanded, this, &PropertiesView::OnExpanded);
    QObject::connect(view, &QTreeView::collapsed, this, &PropertiesView::OnCollapsed);
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

    ControlDescriptorBuilder<ComboBox::Fields> descr;
    descr[ComboBox::Fields::Value] = "viewMode";
    ComboBox* comboBox = new ComboBox(descr, params.accessor, Reflection::Create(ReflectedObject(this)), toolBar);
    toolBar->addWidget(comboBox->ToWidgetCast());

    QHeaderView* headerView = view->header();
    connections.AddConnection(headerView, &QHeaderView::sectionResized, MakeFunction(this, &PropertiesView::OnColumnResized));
}

void PropertiesView::OnObjectsChanged(const Any& objects)
{
    if (objects.IsEmpty())
    {
        model->SetObjects(Vector<Reflection>());
        return;
    }

    DVASSERT(objects.CanCast<Vector<Reflection>>());
    model->SetObjects(objects.Cast<Vector<Reflection>>());
    UpdateExpanded();
}

void PropertiesView::OnColumnResized(int columnIndex, int oldSize, int newSize)
{
    PropertiesViewDelegate* d = qobject_cast<PropertiesViewDelegate*>(view->itemDelegate());
    DVASSERT(d != nullptr);
    if (d->UpdateSizeHints(columnIndex, newSize) == true)
    {
        model->HideEditors();
    }
}

void PropertiesView::Update(UpdatePolicy policy)
{
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
}

void PropertiesView::UpdateExpanded()
{
    ScopedValueGuard<bool> guard(isExpandUpdate, true);
    QModelIndexList expandedList = model->GetExpandedList();
    foreach (const QModelIndex& index, expandedList)
    {
        view->expand(index);
    }
}

void PropertiesView::OnExpanded(const QModelIndex& index)
{
    SCOPED_VALUE_GUARD(bool, isExpandUpdate, true, void());
    model->SetExpanded(true, index);
    model->HideEditors();

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
    model->HideEditors();
}

void PropertiesView::OnFavoritesEditChanged(bool isChecked)
{
    view->SetFavoritesEditMode(isChecked);
}

PropertiesView::eViewMode PropertiesView::GetViewMode() const
{
    return model->IsFavoriteOnly() == true ? VIEW_MODE_FAVORITES_ONLY : VIEW_MODE_NORMAL;
}

void PropertiesView::SetViewMode(PropertiesView::eViewMode mode)
{
    model->SetFavoriteOnly(mode == VIEW_MODE_FAVORITES_ONLY);
    model->HideEditors();
    UpdateExpanded();
}

} // namespace TArc
} // namespace DAVA

ENUM_DECLARE(DAVA::TArc::PropertiesView::eViewMode)
{
    ENUM_ADD_DESCR(DAVA::TArc::PropertiesView::eViewMode::VIEW_MODE_NORMAL, "Normal");
    ENUM_ADD_DESCR(DAVA::TArc::PropertiesView::eViewMode::VIEW_MODE_FAVORITES_ONLY, "Favorites only");
}
