#pragma once

#include <Base/BaseTypes.h>
#include <TArc/Core/ClientModule.h>
#include <TArc/DataProcessing/DataListener.h>
#include <TArc/Utils/QtConnections.h>

#include <memory>

class FindResultsWidget;
class FindFilter;

class FindResultsModule : public DAVA::TArc::ClientModule, public DAVA::TArc::DataListener
{
    void PostInit() override;

    void JumpToControl(const DAVA::FilePath& packagePath, const DAVA::String& controlName);
    void JumpToPackage(const DAVA::FilePath& packagePath);

    void RegisterOperations();

    void FindInProject(std::shared_ptr<FindFilter> filter);
    void FindInDocument(std::shared_ptr<FindFilter> filter);

    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;

    DAVA::TArc::QtConnections connections;

    FindResultsWidget* findResultsWidget = nullptr;
    DAVA::TArc::DataWrapper projectDataWrapper;

    DAVA_VIRTUAL_REFLECTION(FindResultsModule, DAVA::TArc::ClientModule);
};
