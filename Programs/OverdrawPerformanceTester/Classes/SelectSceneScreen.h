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

protected:
    void OnSelectDocumentsPath(BaseObject* caller, void* param, void* callerData);
    void OnSelectResourcesPath(BaseObject* caller, void* param, void* callerData);
    void OnSelectExternalStoragePath(BaseObject* caller, void* param, void* callerData);
    void OnClearPath(BaseObject* caller, void* param, void* callerData);
    void OnStart(BaseObject* caller, void* param, void* callerData);

    void SetScenePath(const FilePath& path);
    void LoadSettings();
    void SaveSettings();

    UIStaticText* fileNameText;
    UIFileSystemDialog* fileSystemDialog;

    FilePath scenePath;
};

#endif //__SELECT_SCENE_SCREEN_H__
