#pragma once

#include "TArc/Core/ContextAccessor.h"
#include "TArc/Core/FieldBinder.h"
#include "TArc/DataProcessing/Common.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/WindowSubSystem/UI.h"

#include <Functional/Function.h>

#include <QWidget>

class QTreeView;

namespace DAVA
{
namespace TArc
{
class ReflectedPropertyModel;
class ExtensionChain;
class ContextAccessor;
class OperationInvoker;
class UI;

class PropertiesView : public QWidget
{
    Q_OBJECT
public:
    enum UpdatePolicy
    {
        FullUpdate,
        FastUpdate
    };

    class Updater
    {
    public:
        virtual ~Updater() = default;

        Signal<UpdatePolicy> update;
    };
    /**
        Create PropertiesView widget with ReflectedModel. As data source for ReflectedMode use value of "objectsField"
        Value of "objectsField" could be casted to Vector<Reflection>
    */
    struct Params
    {
        Params(const WindowKey& key)
            : wndKey(key)
        {
        }

        WindowKey wndKey;
        ContextAccessor* accessor = nullptr;
        OperationInvoker* invoker = nullptr;
        UI* ui = nullptr;
        FieldDescriptor objectsField;
        String settingsNodeName;
        std::weak_ptr<Updater> updater;
    };

    PropertiesView(const Params& params);
    ~PropertiesView();

    void RegisterExtension(const std::shared_ptr<ExtensionChain>& extension);
    void UnregisterExtension(const std::shared_ptr<ExtensionChain>& extension);

private:
    void SetupUI();
    void OnObjectsChanged(const Any& objects);
    void OnColumnResized(int columnIndex, int oldSize, int newSize);
    void Update(UpdatePolicy policy);

    void UpdateExpanded();
    void OnExpanded(const QModelIndex& index);
    void OnCollapsed(const QModelIndex& index);

private:
    FieldBinder binder;
    Params params;
    QTreeView* view = nullptr;
    std::unique_ptr<ReflectedPropertyModel> model;
    SigConnectionID updateConnectionID;
    QtConnections connections;
    bool isExpandUpdate = false;
};
}
}