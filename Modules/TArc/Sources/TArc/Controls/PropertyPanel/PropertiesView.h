#pragma once

#include "TArc/Core/ContextAccessor.h"
#include "TArc/Core/FieldBinder.h"
#include "TArc/DataProcessing/Common.h"
#include "TArc/Utils/QtConnections.h"

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
    /**
        Create PropertiesView widget with ReflectedModel. As data source for ReflectedMode use value of "objectsField"
        Value of "objectsField" could be casted to Vector<Reflection>
    */
    struct Params
    {
        ContextAccessor* accessor = nullptr;
        OperationInvoker* invoker = nullptr;
        UI* ui = nullptr;
        FieldDescriptor objectsField;
        String settingsNodeName;
    };

    PropertiesView(const Params& params);
    ~PropertiesView();

    void RegisterExtension(const std::shared_ptr<ExtensionChain>& extension);
    void UnregisterExtension(const std::shared_ptr<ExtensionChain>& extension);

private:
    void SetupUI();
    void OnObjectsChanged(const Any& objects);
    void OnColumnResized(int columnIndex, int oldSize, int newSize);

    void OnExpanded(const QModelIndex& index);
    void OnCollapsed(const QModelIndex& index);

private:
    FieldBinder binder;
    Params params;
    QTreeView* view = nullptr;
    std::unique_ptr<ReflectedPropertyModel> model;
    QtConnections connections;
};
}
}