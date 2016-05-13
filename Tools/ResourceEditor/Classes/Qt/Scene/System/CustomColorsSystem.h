/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __RESOURCEEDITORQT__CUSTOMCOLORSSYSTEM__
#define __RESOURCEEDITORQT__CUSTOMCOLORSSYSTEM__

#include "LandscapeEditorSystem.h"
#include "LandscapeEditorDrawSystem.h"
#include "Main/Request.h"

#include "Commands2/Base/Command2.h"

class CustomColorsSystem : public LandscapeEditorSystem
{
public:
    CustomColorsSystem(DAVA::Scene* scene);
    ~CustomColorsSystem() override;

    LandscapeEditorDrawSystem::eErrorType EnableLandscapeEditing();
    bool DisableLandscapeEdititing(bool saveNeeded = true);

    void Process(DAVA::float32 timeElapsed) override;
    void Input(DAVA::UIEvent* event) override;

    void SetBrushSize(DAVA::int32 brushSize, bool updateDrawSystem = true);
    DAVA::int32 GetBrushSize();
    void SetColor(DAVA::int32 colorIndex);
    DAVA::int32 GetColor();

    void SaveTexture(const DAVA::FilePath& filePath);
    bool LoadTexture(const DAVA::FilePath& filePath, bool createUndo);
    DAVA::FilePath GetCurrentSaveFileName();

    bool ChangesPresent();

private:
    bool CouldApplyImage(DAVA::Image* image, const DAVA::String& imageName) const;

    void UpdateToolImage(bool force = false);
    void UpdateBrushTool();
    void CreateToolImage(const DAVA::FilePath& filePath);

    void AddRectToAccumulator(const DAVA::Rect& rect);
    void ResetAccumulatorRect();
    DAVA::Rect GetUpdatedRect();

    void StoreOriginalState();
    void CreateUndoPoint();

    void StoreSaveFileName(const DAVA::FilePath& filePath);

    DAVA::FilePath GetScenePath();
    DAVA::String GetRelativePathToScenePath(const DAVA::FilePath& absolutePath);
    DAVA::FilePath GetAbsolutePathFromScenePath(const DAVA::String& relativePath);
    DAVA::String GetRelativePathToProjectPath(const DAVA::FilePath& absolutePath);
    DAVA::FilePath GetAbsolutePathFromProjectPath(const DAVA::String& relativePath);

    void FinishEditing();

    Command2::Pointer CreateSaveFileNameCommand(const DAVA::String& filePath);

private:
    DAVA::Texture* toolImageTexture = nullptr;
    DAVA::Texture* loadedTexture = nullptr;
    DAVA::Image* originalImage = nullptr;
    DAVA::Color drawColor = DAVA::Color::Transparent;
    DAVA::int32 colorIndex = 0;
    DAVA::int32 curToolSize = 120;
    DAVA::Rect updatedRectAccumulator;
    bool editingIsEnabled = false;
};

#endif /* defined(__RESOURCEEDITORQT__CUSTOMCOLORSSYSTEM__) */
