#pragma once

#include "Classes/Library/Private/ScenePreviewDialog.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Core/FieldBinder.h"

class LibraryWidget;
class LibraryModule : public DAVA::TArc::ClientModule
{
public:
    ~LibraryModule() override;

protected:
    void PostInit() override;

    void OnProjectChanged(const DAVA::Any& projectFieldValue);
    void OnSelectedPathChanged(const DAVA::Any& selectedPathValue);

private:
    void ShowPreview(const DAVA::FilePath& path);
    void HidePreview();

    DAVA::RefPtr<ScenePreviewDialog> previewDialog;
    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;
};
