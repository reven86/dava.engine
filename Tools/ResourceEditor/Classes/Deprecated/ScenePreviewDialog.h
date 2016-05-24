#ifndef __SCENE_PREVIEW_DIALOG_H__
#define __SCENE_PREVIEW_DIALOG_H__

#include "DAVAEngine.h"
#include "Deprecated/ExtendedDialog.h"
#include "Deprecated/ScenePreviewControl.h"

class ScenePreviewDialog : public ExtendedDialog
{
public:
    ScenePreviewDialog();
    virtual ~ScenePreviewDialog();

    void Show(const DAVA::FilePath& scenePathname);
    void Close() override;

protected:
    const DAVA::Rect GetDialogRect() const override;
    void UpdateSize() override;

    void OnClose(BaseObject*, void*, void*);

    DAVA::ScopedPtr<ScenePreviewControl> preview;
    DAVA::ScopedPtr<DAVA::UIStaticText> errorMessage;
    DAVA::ScopedPtr<DAVA::UIControl> clickableBackgound;
};



#endif // __SCENE_PREVIEW_DIALOG_H__