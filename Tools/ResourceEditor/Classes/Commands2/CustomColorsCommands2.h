#pragma once

#include "Commands2/Base/RECommand.h"

#include "DAVAEngine.h"

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
