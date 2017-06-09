#pragma once

#include "EditorSystems/SelectionContainer.h"
#include "Model/PackageHierarchy/PackageListener.h"

#include <QAbstractItemModel>
#include <QMimeData>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
class UI;
}
}

class AbstractProperty;
class PackageNode;
class ControlNode;
class PackageBaseNode;
class StyleSheetsNode;
class PackageControlsNode;
class ControlsContainerNode;
class ImportedPackagesNode;
class QtModelPackageCommandExecutor;

class PackageModel : public QAbstractItemModel, PackageListener
{
    Q_OBJECT

public:
    PackageModel(QObject* parent = nullptr);
    ~PackageModel() override;

    void SetAccessor(DAVA::TArc::ContextAccessor* accessor);
    void SetUI(DAVA::TArc::UI* ui);

    void Reset(PackageNode* package);

    QModelIndex indexByNode(PackageBaseNode* node) const;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

signals:
    void BeforeProcessNodes(const SelectedNodes& nodes);
    void AfterProcessNodes(const SelectedNodes& nodes);

public slots:
    void OnDropMimeData(const QMimeData* data, Qt::DropAction action, PackageBaseNode* targetNode, DAVA::uint32 destIndex, const DAVA::Vector2* pos);

private: // PackageListener
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void StylePropertyWasChanged(StyleSheetNode* node, AbstractProperty* property) override;

    void ControlWillBeAdded(ControlNode* node, ControlsContainerNode* destination, int row) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int row) override;

    void ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from) override;

    void StyleWillBeAdded(StyleSheetNode* node, StyleSheetsNode* destination, int index) override;
    void StyleWasAdded(StyleSheetNode* node, StyleSheetsNode* destination, int index) override;

    void StyleWillBeRemoved(StyleSheetNode* node, StyleSheetsNode* from) override;
    void StyleWasRemoved(StyleSheetNode* node, StyleSheetsNode* from) override;

    void ImportedPackageWillBeAdded(PackageNode* node, ImportedPackagesNode* to, int index) override;
    void ImportedPackageWasAdded(PackageNode* node, ImportedPackagesNode* to, int index) override;

    void ImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from) override;
    void ImportedPackageWasRemoved(PackageNode* node, ImportedPackagesNode* from) override;

    int GetRowIndex(int row, const QModelIndex& parent) const;

    DAVA::RefPtr<PackageNode> package;
    DAVA::TArc::ContextAccessor* accessor = nullptr;
    DAVA::TArc::UI* ui = nullptr;
};
