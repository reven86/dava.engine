#pragma once

#include <TArc/Core/ControllerModule.h>
#include <TArc/Core/OperationRegistrator.h>

namespace TestHelpers
{
class DocumentsModuleHelper : public DAVA::TArc::ControllerModule
{
    void PostInit() override;
    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;

    void OnRenderSystemInitialized(DAVA::Window* w) override;
    bool CanWindowBeClosedSilently(const DAVA::TArc::WindowKey& key, DAVA::String& requestWindowText) override;
    void SaveOnWindowClose(const DAVA::TArc::WindowKey& key) override;
    void RestoreOnWindowClose(const DAVA::TArc::WindowKey& key) override;

    void CloseAllDocuments();
    void CreateDummyContext();

    DAVA_VIRTUAL_REFLECTION(DocumentsModuleHelper, DAVA::TArc::ControllerModule);
};

DECLARE_OPERATION_ID(CreateDummyContextOperation);

class SampleData : public DAVA::TArc::DataNode
{
public:
    bool canClose = true;

    DAVA_VIRTUAL_REFLECTION(SampleData, DAVA::TArc::DataNode);
};
} //namespace TestHelpers
