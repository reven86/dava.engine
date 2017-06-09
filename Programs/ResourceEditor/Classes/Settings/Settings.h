#pragma once

#include "Base/FastName.h"

#include "Render/RenderBase.h"

class Settings
{
public:
    static const char Delimiter;

    static const DAVA::FastName General_RecentFilesCount;
    static const DAVA::FastName General_RecentProjectsCount;
    static const DAVA::FastName General_ReloadParticlesOnPojectOpening;
    static const DAVA::FastName General_PreviewEnabled;
    static const DAVA::FastName General_OpenByDBClick;
    static const DAVA::FastName General_CompressionQuality;
    static const DAVA::FastName General_ShowErrorDialog;

    static const DAVA::FastName General_MaterialEditor_SwitchColor0;
    static const DAVA::FastName General_MaterialEditor_SwitchColor1;
    static const DAVA::FastName General_MaterialEditor_LodColor0;
    static const DAVA::FastName General_MaterialEditor_LodColor1;
    static const DAVA::FastName General_MaterialEditor_LodColor2;
    static const DAVA::FastName General_MaterialEditor_LodColor3;

    static const DAVA::FastName General_LODEditor_LodColor0;
    static const DAVA::FastName General_LODEditor_LodColor1;
    static const DAVA::FastName General_LODEditor_LodColor2;
    static const DAVA::FastName General_LODEditor_LodColor3;
    static const DAVA::FastName General_LODEditor_InactiveColor;
    static const DAVA::FastName General_LODEditor_FitSliders;

    static const DAVA::FastName General_ParticleEditor_ParticleDebugAlphaTheshold;

    static const DAVA::FastName General_HeighMaskTool_Color0;
    static const DAVA::FastName General_HeighMaskTool_Color1;

    static const DAVA::FastName General_ColorMultiplyMax;

    static const DAVA::FastName General_AssetCache_UseCache;
    static const DAVA::FastName General_AssetCache_Ip;
    static const DAVA::FastName General_AssetCache_Port;
    static const DAVA::FastName General_AssetCache_Timeout;

    static const DAVA::FastName General_AutoConvertation;
    static const DAVA::FastName General_RenderBackend;

    static const DAVA::FastName Scene_GridStep;
    static const DAVA::FastName Scene_GridSize;
    static const DAVA::FastName Scene_CameraSpeed0;
    static const DAVA::FastName Scene_CameraSpeed1;
    static const DAVA::FastName Scene_CameraSpeed2;
    static const DAVA::FastName Scene_CameraSpeed3;
    static const DAVA::FastName Scene_CameraFOV;
    static const DAVA::FastName Scene_CameraNear;
    static const DAVA::FastName Scene_CameraFar;
    static const DAVA::FastName Scene_CameraHeightOnLandscape;
    static const DAVA::FastName Scene_CameraHeightOnLandscapeStep;
    static const DAVA::FastName Scene_SelectionSequent;
    static const DAVA::FastName Scene_SelectionOnClick;
    static const DAVA::FastName Scene_SelectionDrawMode;
    static const DAVA::FastName Scene_CollisionDrawMode;
    static const DAVA::FastName Scene_ModificationByGizmoOnly;
    static const DAVA::FastName Scene_GizmoScale;
    static const DAVA::FastName Scene_DebugBoxScale;
    static const DAVA::FastName Scene_DebugBoxUserScale;
    static const DAVA::FastName Scene_DebugBoxParticleScale;
    static const DAVA::FastName Scene_DebugBoxWaypointScale;
    static const DAVA::FastName Scene_DragAndDropWithShift;
    static const DAVA::FastName Scene_AutoselectNewEntities;
    static const DAVA::FastName Scene_RememberForceParameters;
    static const DAVA::FastName Scene_SaveEmitters;
    static const DAVA::FastName Scene_SaveStaticOcclusion;
    static const DAVA::FastName Scene_DefaultCustomColorIndex;

    static const DAVA::FastName Scene_Sound_SoundObjectDraw;
    static const DAVA::FastName Scene_Sound_SoundObjectBoxColor;
    static const DAVA::FastName Scene_Sound_SoundObjectSphereColor;

    static const DAVA::FastName Scene_Grab_Size_Width;
    static const DAVA::FastName Scene_Grab_Size_Height;

    static const DAVA::FastName General_Mouse_InvertWheel;
    static const DAVA::FastName General_Mouse_WheelMoveCamera;
    static const DAVA::FastName General_Mouse_WheelMoveIntensity;

    // this settings won't be shown in settings dialog
    // and are used only by application
    static const DAVA::FastName InternalGroup;
    static const DAVA::FastName Internal_TextureViewGPU;
    static const DAVA::FastName Internal_SpriteViewGPU;
    static const DAVA::FastName Internal_LastProjectPath;
    static const DAVA::FastName Internal_EditorVersion;
    static const DAVA::FastName Internal_CubemapLastFaceDir;
    static const DAVA::FastName Internal_CubemapLastProjDir;
    static const DAVA::FastName Internal_ParticleLastSaveEmitterDir;
    static const DAVA::FastName Internal_ParticleLastLoadEmitterDir;

    static const DAVA::FastName Internal_RecentFiles;
    static const DAVA::FastName Internal_RecentProjects;
    static const DAVA::FastName Internal_MaterialsLightViewMode;
    static const DAVA::FastName Internal_MaterialsShowLightmapCanvas;
    static const DAVA::FastName Internal_LicenceAccepted;
    static const DAVA::FastName Internal_LODEditor_Mode;
    static const DAVA::FastName Internal_LODEditor_Recursive;

    static const DAVA::FastName Internal_ImageSplitterPath;
    static const DAVA::FastName Internal_ImageSplitterPathSpecular;

    static const DAVA::FastName Internal_CustomPalette;
    static const DAVA::FastName Internal_LogWidget;
    static const DAVA::FastName Internal_EnableSounds;
    static const DAVA::FastName Internal_GizmoEnabled;

    static const DAVA::FastName Internal_Validate_Matrices;
    static const DAVA::FastName Internal_Validate_SameNames;
    static const DAVA::FastName Internal_Validate_CollisionProperties;
    static const DAVA::FastName Internal_Validate_TexturesRelevance;
    static const DAVA::FastName Internal_Validate_MaterialGroups;
    static const DAVA::FastName Internal_Validate_ShowConsole;

    static DAVA::eGPUFamily GetGPUFormat();
}; //End of Settings
