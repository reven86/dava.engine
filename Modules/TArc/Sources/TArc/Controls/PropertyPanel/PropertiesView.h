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

class PropertiesView : public QWidget
{
    Q_OBJECT
public:
    /**
        Create PropertiesView widget with ReflectedModel. As data source for ReflectedMode use value of "objectsField"
        Value of "objectsField" could be casted to Vector<Reflection>
    */
    PropertiesView(ContextAccessor* accessor, const FieldDescriptor& objectsField);
    ~PropertiesView();

    void RegisterExtension(const std::shared_ptr<ExtensionChain>& extension);
    void UnregisterExtension(const std::shared_ptr<ExtensionChain>& extension);

private:
    void SetupUI();
    void OnObjectsChanged(const Any& objects);
    void OnColumnResized(int columnIndex, int oldSize, int newSize);

private:
    FieldBinder binder;
    QTreeView* view = nullptr;
    std::unique_ptr<ReflectedPropertyModel> model;
    QtConnections connections;
};
}
}