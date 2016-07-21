#ifndef __RESOURCEEDITORQT__CUSTOMCOLORSCOMMANDS2__
#define __RESOURCEEDITORQT__CUSTOMCOLORSCOMMANDS2__

#include "Commands2/Base/RECommand.h"
#include "Commands2/Base/CommandAction.h"

#include "DAVAEngine.h"
#include "Main/Request.h"

class CustomColorsProxy;
class SceneEditor2;

class ModifyCustomColorsCommand : public RECommand
{
public:
    ModifyCustomColorsCommand(DAVA::Image* originalImage, DAVA::Image* currentImage, CustomColorsProxy* customColorsProxy,
                              const DAVA::Rect& updatedRect, bool clearTexture);
    ~ModifyCustomColorsCommand() override;

    void Undo() override;
    void Redo() override;

private:
    void ApplyImage(DAVA::Image* image, bool disableBlend);

private:
    CustomColorsProxy* customColorsProxy = nullptr;
    DAVA::Image* undoImage = nullptr;
    DAVA::Image* redoImage = nullptr;
    DAVA::Rect updatedRect;
    bool shouldClearTexture = false;
};

#endif /* defined(__RESOURCEEDITORQT__CUSTOMCOLORSCOMMANDS2__) */
