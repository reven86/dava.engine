#include "Classes/PropertyPanel/PropertyModelExt.h"
#include "Classes/SceneManager/SceneData.h"
#include "Classes/Commands2/SetFieldValueCommand.h"

#include "TArc/DataProcessing/DataContext.h"

REModifyPropertyExtension::REModifyPropertyExtension(DAVA::TArc::ContextAccessor* accessor_)
    : accessor(accessor_)
{
}

void REModifyPropertyExtension::ProduceCommand(const DAVA::Vector<DAVA::Reflection::Field>& objects, const DAVA::Any& newValue)
{
    using namespace DAVA::TArc;
    DataContext* ctx = accessor->GetActiveContext();
    if (ctx == nullptr)
    {
        return;
    }

    SceneData* data = ctx->GetData<SceneData>();
    DVASSERT(data != nullptr);

    DAVA::RefPtr<SceneEditor2> scene = data->GetScene();
    scene->BeginBatch("Set property value", objects.size());
    for (const DAVA::Reflection::Field& field : objects)
    {
        scene->Exec(std::make_unique<SetFieldValueCommand>(field, newValue));
    }
    scene->EndBatch();
}