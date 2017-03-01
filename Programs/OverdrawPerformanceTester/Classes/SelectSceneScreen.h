#ifndef __SELECT_SCENE_SCREEN_H__
#define __SELECT_SCENE_SCREEN_H__

#include "BaseScreen.h"

using namespace DAVA;

class SelectSceneScreen : public BaseScreen, UIFileSystemDialogDelegate
{
protected:
    virtual ~SelectSceneScreen()
    {
    }

public:    

    SelectSceneScreen();

    virtual void LoadResources();
    virtual void UnloadResources();

    virtual void OnFileSelected(UIFileSystemDialog* forDialog, const FilePath& pathToFile);
    virtual void OnFileSytemDialogCanceled(UIFileSystemDialog* forDialog);
    void SetWarningMessage(const WideString&& message);

protected:
    void OnSelectDocumentsPath(BaseObject* caller, void* param, void* callerData);
    void OnSelectResourcesPath(BaseObject* caller, void* param, void* callerData);
    void OnSelectExternalStoragePath(BaseObject* caller, void* param, void* callerData);
    void OnClearPath(BaseObject* caller, void* param, void* callerData);
    void OnStart(BaseObject* caller, void* param, void* callerData);

    void SetScenePath(const FilePath& path);
    void LoadSettings();
    void SaveSettings();

private:
    struct ButtonInfo
    {
        WideString caption;
        int32 tag;
        Rect rect;
        union
        {
            uint16 resolution;
            DAVA::PixelFormat pixelFormat;
            int8 overdrawChangeInfo;
        };
    };

    UIButton* CreateUIButton(const WideString& caption, const Rect& rect, int32 tag, DAVA::Font* font, const DAVA::Message& msg);
    void OnResolutionButtonClick(BaseObject* sender, void* data, void* callerData);
    void OnTextureFormatButtonClick(BaseObject* sender, void* data, void* callerData);
    void OnChangeOverdrawButtonClick(BaseObject* sender, void* data, void* callerData);

    UIStaticText* fileNameText = nullptr;
    UIFileSystemDialog* fileSystemDialog = nullptr;
    UIStaticText* overdrawInfoMessage = nullptr;
    UIStaticText* overdrawCountLabel = nullptr;

    FilePath scenePath;

    DAVA::UnorderedMap<UIButton*, ButtonInfo> resolutionButtons;
    DAVA::UnorderedMap<UIButton*, ButtonInfo> texturePixelFormatButtons;
    DAVA::UnorderedMap<UIButton*, ButtonInfo> overdrawButtons;

    UITextFieldDelegate* inputDelegate = nullptr;

    static const Array<ButtonInfo, 4> resolutionButtonsInfo;
    static const Array<ButtonInfo, 5> texturePixelFormatButtonsInfo;
    static const Array<ButtonInfo, 2> overdrawButtonsInfo;
    static const Color Red;
    static const Color Green;
    static const float32 resolutionButtonsXOffset;
    static const float32 resolutionButtonsYOffset;
    static const float32 buttonHeight;
    static const float32 buttonWidth;
    static const float32 heigthDistanceBetweenButtons;
    static const float32 texturePixelFormatXOffset;
    static const float32 texturePixelFormatYOffset; 
    static const float32 overdrawXOffset;
    static const float32 overdrawYOffset;
};

#endif //__SELECT_SCENE_SCREEN_H__
