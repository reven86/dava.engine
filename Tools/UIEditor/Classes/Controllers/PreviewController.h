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


#ifndef __PREVIEWCONTROLLER__H__
#define __PREVIEWCONTROLLER__H__

#include "DAVAEngine.h"

class DefaultScreen;
namespace DAVA {

struct PreviewSettingsData
{
    int32 id;

    String deviceName;
    Vector2 screenSize;
    uint32 dpi;
    bool isPredefined;
    
    int32 positionInList;
};
  
// Transform Data is used by UI Editor to adjust the screen size
// according to the Preview Settings selected.
struct PreviewTransformData
{
    Vector2 screenSize;
    float32 zoomLevel;
};

// Recalculate screen diagonal to DPI and wice versa.
class ScreenParamsConverter
{
public:
    static int32 ConvertDiagonalToDPI(const Vector2& screenSize, float32 diagonal);
    static float32 ConverDPIToDiagonal(const Vector2& screenSize, int32 dpi);
};

class PreviewController : public Singleton<PreviewController>
{
public:
    PreviewController();
    virtual ~PreviewController();
    
    void EnablePreview(bool applyScale);
    const PreviewTransformData& SetPreviewMode(const PreviewSettingsData& data,
                                              const Vector2& virtualScreenSize,
                                              uint32 screenDPI);
    void DisablePreview();

    bool IsPreviewEnabled() const {return previewEnabled;};
    int32 GetActivePreviewSettingsID() const {return activePreviewSettingsID;};

    // Get the current preview screen size.
    const PreviewTransformData& GetTransformData() const;

    // Save/load the Preview Settings.
    void LoadPreviewSettings(const YamlNode* rootNode);
    void SavePreviewSettings(YamlNode* rootNode);
    
    // Editing functionality.
    PreviewSettingsData GetPreviewSettingsData(int32 id) const;
    PreviewSettingsData GetActivePreviewSettingsData() const;

    void AddPreviewSettingsData(const PreviewSettingsData& data);
    void RemovePreviewSettingsData(int32 id);

    // Get the preview settings list.
    const List<PreviewSettingsData>& GetPreviewSettings() const;
    
    // Return true if some changes were made in the preview settings list after the initial load.
    bool HasUnsavedChanges() const;
    
    // Make the screenshot of the screen passed.
    void MakeScreenshot(const String& fileName, DefaultScreen* screen);

protected:
    // Calculate the transform data.
    PreviewTransformData CalculateTransform(const PreviewSettingsData& data,
                                            const Vector2& virtualScreenSize,
                                            uint32 screenDPI);

    // Load the predefined (hardcoded) preview settings.
    void LoadPredefinedPreviewSettings();

    // Set the dirty flag.
    void SetDirty(bool value);

private:
    bool previewEnabled;
    PreviewTransformData currentTransformData;
    
    // Supported Preview Settings.
    List<PreviewSettingsData> previewSettings;

    // Next index for preview settings while adding/removing.
    static int32 nextId;
    
    // Dirty flag.
    bool isDirty;
    
    // Active Preview Settings ID.
    int32 activePreviewSettingsID;
    
    // Whether the preview scale is needed.
    bool isApplyPreviewScale;
};

};

#endif /* defined(__PREVIEWCONTROLLER__H__) */
