#include "TArc/Controls/PropertyPanel/PropertyPanelMeta.h"

namespace DAVA
{
namespace Metas
{
void CommandProducerHolder::AddCommandProducer(std::shared_ptr<CommandProducer>&& producer)
{
    commandProducers.push_back(std::move(producer));
}

const Vector<std::shared_ptr<CommandProducer>>& CommandProducerHolder::GetCommandProducers() const
{
    return commandProducers;
}

bool CommandProducer::OnlyForSingleSelection() const
{
    return false;
}

void CommandProducer::CreateCache(TArc::ContextAccessor* accessor)
{
}

void CommandProducer::ClearCache()
{
}

} // namespace Metas
} // namespace DAVA