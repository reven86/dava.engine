#pragma once

#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"

#include <Command/Command.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectedMeta.h>

#include <QIcon>
#include <QString>

namespace DAVA
{
namespace TArc
{
class OperationInvoker;
class ContextAccessor;
class UI;
}

namespace Metas
{
class ProxyMetaRequire
{
};

class FieldExpanded
{
public:
    bool isExpanded = true;
};

class CommandProducer
{
public:
    struct Params
    {
        TArc::OperationInvoker* invoker = nullptr;
        TArc::ContextAccessor* accessor = nullptr;
        TArc::UI* ui = nullptr;
    };

    struct Info
    {
        QIcon icon;
        QString tooltip;
        String description;
    };

    virtual ~CommandProducer() = default;
    virtual bool IsApplyable(const std::shared_ptr<DAVA::TArc::PropertyNode>& field) const = 0;
    virtual Info GetInfo() const = 0;
    virtual bool OnlyForSingleSelection() const;
    virtual void CreateCache(DAVA::TArc::ContextAccessor* accessor);
    virtual void ClearCache();
    virtual std::unique_ptr<Command> CreateCommand(const std::shared_ptr<DAVA::TArc::PropertyNode>& field, const Params& params) const = 0;
};

class CommandProducerHolder
{
public:
    CommandProducerHolder() = default;
    void AddCommandProducer(std::shared_ptr<CommandProducer>&& producer);
    const Vector<std::shared_ptr<CommandProducer>>& GetCommandProducers() const;

private:
    Vector<std::shared_ptr<CommandProducer>> commandProducers;
};
} // namespace Metas

namespace M
{
using ProxyMetaRequire = Meta<Metas::ProxyMetaRequire>;
using FieldExpanded = Meta<Metas::FieldExpanded>;
using CommandProducer = Metas::CommandProducer;
using CommandProducerHolder = Meta<Metas::CommandProducerHolder>;
} // namespace M
} // namespace DAVA