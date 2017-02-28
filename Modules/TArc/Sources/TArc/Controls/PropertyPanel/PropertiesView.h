#pragma once

#include "TArc/Core/ContextAccessor.h"
#include "TArc/Core/FieldBinder.h"
#include "TArc/DataProcessing/Common.h"
#include "TArc/Utils/QtConnections.h"

#include <Functional/Function.h>

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
    PropertiesView(ContextAccessor* accessor, const FieldDescriptor& objectsField, const std::weak_ptr<Updater>& updater);
    ~PropertiesView();

    void RegisterExtension(const std::shared_ptr<ExtensionChain>& extension);
    void UnregisterExtension(const std::shared_ptr<ExtensionChain>& extension);

private:
    void SetupUI();
    void OnObjectsChanged(const Any& objects);
    void OnColumnResized(int columnIndex, int oldSize, int newSize);
    void Update(UpdatePolicy policy);

private:
    FieldBinder binder;
    QTreeView* view = nullptr;
    std::unique_ptr<ReflectedPropertyModel> model;
    std::weak_ptr<Updater> updater;
    SigConnectionID updateConnectionID;
    QtConnections connections;
};
}
}