#pragma once

#include <TArc/Controls/PropertyPanel/PropertyModelExtensions.h>
#include <TArc/Core/ContextAccessor.h>

#include <Base/BaseTypes.h>

class REModifyPropertyExtension : public DAVA::TArc::ModifyExtension
{
public:
    REModifyPropertyExtension(DAVA::TArc::ContextAccessor* accessor);

    void ProduceCommand(const DAVA::Vector<DAVA::Reflection::Field>& objects, const DAVA::Any& newValue) override;

private:
    DAVA::TArc::ContextAccessor* accessor = nullptr;
};

class EntityChildCreator : public DAVA::TArc::ChildCreatorExtension
{
public:
    void ExposeChildren(const std::shared_ptr<const DAVA::TArc::PropertyNode>& parent, DAVA::Vector<std::shared_ptr<DAVA::TArc::PropertyNode>>& children) const override;
};
