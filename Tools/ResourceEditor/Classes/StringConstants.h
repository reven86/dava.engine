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



#ifndef __STRING_CONSTANTS_H__
#define __STRING_CONSTANTS_H__

using namespace DAVA;

namespace ResourceEditor 
{
	// Node names
	static const String LANDSCAPE_NODE_NAME = "Landscape";
	static const String LIGHT_NODE_NAME = "Light";
	static const String SERVICE_NODE_NAME = "Servicenode";
	static const String CAMERA_NODE_NAME = "Camera";
	static const String IMPOSTER_NODE_NAME = "Imposter";
	static const String PARTICLE_EMITTER_NODE_NAME = "Particle Emitter";
	static const String USER_NODE_NAME = "UserNode";
	static const String SWITCH_NODE_NAME = "SwitchNode";
	static const String PARTICLE_EFFECT_NODE_NAME = "Particle Effect";
	static const String LAYER_NODE_NAME = "Layer";
	static const String ENTITY_NAME = "Entity";

	// Base node names
	static const String EDITOR_BASE = "editor.";
	static const String EDITOR_MAIN_CAMERA = "editor.main-camera";
	static const String EDITOR_DEBUG_CAMERA = "editor.debug-camera";
	static const String EDITOR_ARROWS_NODE = "editor.arrows-node";
	static const String EDITOR_CAMERA_LIGHT = "editor.camera-light";
	
	// Headers
	static const WideString CREATE_NODE_LANDSCAPE = L"createnode.landscape";
	static const WideString CREATE_NODE_LIGHT = L"createnode.light";
	static const WideString CREATE_NODE_SERVICE = L"createnode.servicenode";
	static const WideString CREATE_NODE_CAMERA = L"createnode.camera";
	static const WideString CREATE_NODE_IMPOSTER = L"createnode.imposter";
	static const WideString CREATE_NODE_PARTICLE_EMITTER = L"createnode.particleemitter";
	static const WideString CREATE_NODE_USER = L"createnode.usernode";
	static const WideString CREATE_NODE_SWITCH = L"createnode.switchnode";
	static const WideString CREATE_NODE_PARTICLE_EFFECT = L"Particle Effect";
	
	// Properties
	static const String EDITOR_REFERENCE_TO_OWNER = "editor.referenceToOwner";
	static const String EDITOR_IS_LOCKED = "editor.isLocked";
	static const String EDITOR_DO_NOT_REMOVE = "editor.donotremove";
    static const String EDITOR_DYNAMIC_LIGHT_ENABLE = "editor.dynamiclight.enable";

    //Documentation
    static const String DOCUMENTATION_PATH = "~doc:/ResourceEditorHelp/";
    
    //service strings
    static const String TAG = "tag";
	
	//settings properties names
	static const String SETTINGS_3D_DATA_SOURCEPATH	=	"3dDataSourcePath";
	static const String SETTINGS_PROJECT_PATH =			"ProjectPath";
	static const String SETTINGS_CAMERA_SPEED_INDEX =	"CameraSpeedIndex";
	static const String SETTINGS_CAMERA_SPEED_VALUE =	"CameraSpeedValue";
	static const String SETTINGS_SCREEN_WIDTH =			"ScreenWidth";
	static const String SETTINGS_SCREEN_HEIGHT =		"ScreenHeight";
	static const String SETTINGS_LANGUAGE =				"Language";
	static const String SETTINGS_SHOW_OUTPUT =			"ShowOutput";
	static const String SETTINGS_LEFT_PANEL_WIDTH =		"LeftPanelWidth";
	static const String SETTINGS_RIGHT_PANEL_WIDTH =	"RightPanelWidth";
	static const String SETTINGS_LOD_LAYER =			"LODLayer";
	static const String SETTINGS_LAST_OPENED_FILE =		"LastOpenedFile";
	static const String SETTINGS_LAST_OPENED_FILES_COUNT = "LastOpenedFilesCount";
	static const String SETTINGS_DRAW_GRID =			"DrawGrid";
	static const String SETTINGS_ENABLE_IMPOSTERS =		"enableImposters";
	static const String SETTINGS_TEXTURE_VIEW_GPU =		"TextureViewGPU";
	static const String SETTINGS_MATERIALS_AMBIENT =	"materials.ambient";
	static const String SETTINGS_MATERIALS_DIFFUSE =	"materials.diffuse";
	static const String SETTINGS_MATERIALS_SPECULAR	=	"materials.specular";
	static const String SETTINGS_DESIGNER_NAME =		"DesignerName";
	static const String SETTINGS_PREVIEW_DIALOG_ENABLED = "PreviewDialogEnabled";

	static const String CUSTOM_COLOR_TEXTURE_PROP = "customColorTexture";
	
	static const String CUBEMAP_LAST_PROJECT_DIR_KEY = "cubemap_last_proj_dir";

	static const String TILEMASK_EDITOR_BRUSH_SIZE_CAPTION	= "Brush\nsize:";
	static const String TILEMASK_EDITOR_BRUSH_IMAGE_CAPTION	= "Brush\nimage:";
	static const String TILEMASK_EDITOR_DRAW_CAPTION		= "Normal draw";
	static const String TILEMASK_EDITOR_COPY_PASTE_CAPTION	= "Copy/paste";
	static const String TILEMASK_EDITOR_TILE_TEXTURE_CAPTION= "Tile texture:";
	static const String TILEMASK_EDITOR_STRENGTH_CAPTION	= "Strength:";
	static const String TILEMASK_EDITOR_BRUSH_SIZE_MIN		= "tilemask-editor.brush-size.min";
	static const String TILEMASK_EDITOR_BRUSH_SIZE_MAX		= "tilemask-editor.brush-size.max";
	static const String TILEMASK_EDITOR_STRENGTH_MIN		= "tilemask-editor.strength.min";
	static const String TILEMASK_EDITOR_STRENGTH_MAX		= "tilemask-editor.strength.max";
	static const String TILEMASK_EDITOR_TOOLS_PATH			= "~res:/LandscapeEditor/Tools/";
	static const String TILEMASK_EDITOR_ENABLE_ERROR		= "Error enabling Tile Mask Editor. Make sure there is valid landscape at the scene.";
	static const String TILEMASK_EDITOR_DISABLE_ERROR		= "Error disabling Tile Mask Editor";

	static const String CUSTOM_COLORS_BRUSH_SIZE_CAPTION	= "Brush\nsize:";
	static const String CUSTOM_COLORS_BRUSH_SIZE_MIN		= "custom-colors.brush-size.min";
	static const String CUSTOM_COLORS_BRUSH_SIZE_MAX		= "custom-colors.brush-size.max";
	static const String CUSTOM_COLORS_PROPERTY_COLORS		= "LandscapeCustomColors";
	static const String CUSTOM_COLORS_PROPERTY_DESCRIPTION	= "LandscapeCustomColorsDescription";
	static const String CUSTOM_COLORS_SAVE_CAPTION			= "Save texture";
	static const String CUSTOM_COLORS_LOAD_CAPTION			= "Load texture";
	static const String CUSTOM_COLORS_FILE_FILTER			= "PNG image (*.png)";
	static const String CUSTOM_COLORS_ENABLE_ERROR			= "Error enabling Custom Colors editor. Make sure there is valid landscape at the scene.";
	static const String CUSTOM_COLORS_DISABLE_ERROR			= "Error disabling Custom Colors editor.";

	static const String VISIBILITY_TOOL_AREA_SIZE_CAPTION	= "Visibility Area Size:";
	static const String VISIBILITY_TOOL_AREA_SIZE_MIN		= "visibility-tool.area-size.min";
	static const String VISIBILITY_TOOL_AREA_SIZE_MAX		= "visibility-tool.area-size.max";
	static const String VISIBILITY_TOOL_ENABLE_ERROR		= "Error enabling Visibility Check Tool. Make sure there is valid landscape at the scene.";
	static const String VISIBILITY_TOOL_DISABLE_ERROR		= "Error disabling Visibility Check Tool";
	static const String VISIBILITY_TOOL_SAVE_CAPTION		= "Save visibility tool texture";
	static const String VISIBILITY_TOOL_FILE_FILTER			= "PNG image (*.png)";
	static const String VISIBILITY_TOOL_SET_AREA_CAPTION	= "Set Visibility Area";
	static const String VISIBILITY_TOOL_SET_POINT_CAPTION	= "Set Visibility Point";
	static const String VISIBILITY_TOOL_SAVE_TEXTURE_CAPTION= "Save Texture";

	static const String RULER_TOOL_LINE_WIDTH_CAPTION		= "Line\nwidth:";
	static const String RULER_TOOL_LENGTH_CAPTION			= "Length:";
	static const String RULER_TOOL_PREVIEW_LENGTH_CAPTION	= "Preview length:";
	static const String RULER_TOOL_ENABLE_ERROR				= "Error enabling Ruler Tool. Make sure there is valid landscape at the scene.";
	static const String RULER_TOOL_DISABLE_ERROR			= "Error disabling Ruler Tool";
	static const String LANDSCAPE_DIALOG_WRONG_PNG_ERROR	= "PNG file should be in format A8 or A16.";

	static const String HEIGHTMAP_EDITOR_BRUSH_SIZE_CAPTION			= "Brush\nsize:";
	static const String HEIGHTMAP_EDITOR_STRENGTH_CAPTION			= "Strength:";
	static const String HEIGHTMAP_EDITOR_AVERAGE_STRENGTH_CAPTION	= "Average\nstrength:";
	static const String HEIGHTMAP_EDITOR_BRUSH_SIZE_MIN				= "heightmap-editor.brush-size.min";
	static const String HEIGHTMAP_EDITOR_BRUSH_SIZE_MAX				= "heightmap-editor.brush-size.max";
	static const String HEIGHTMAP_EDITOR_STRENGTH_MAX				= "heightmap-editor.strength.max";
	static const String HEIGHTMAP_EDITOR_AVERAGE_STRENGTH_MIN		= "heightmap-editor.average-strength.min";
	static const String HEIGHTMAP_EDITOR_AVERAGE_STRENGTH_MAX		= "heightmap-editor.average-strength.max";
	static const String HEIGHTMAP_EDITOR_TOOLS_PATH					= "~res:/LandscapeEditor/Tools/";
	static const String HEIGHTMAP_EDITOR_ENABLE_ERROR				= "Error enabling Height Map editor. Make sure there is valid landscape at the scene.";
	static const String HEIGHTMAP_EDITOR_DISABLE_ERROR				= "Error disabling Height Map editor.";
	static const String HEIGHTMAP_EDITOR_RADIO_COPY_PASTE			= "Copy/paste";
	static const String HEIGHTMAP_EDITOR_RADIO_ABS_DROP				= "Abs & Drop";
	static const String HEIGHTMAP_EDITOR_RADIO_ABSOLUTE				= "Absolute";
	static const String HEIGHTMAP_EDITOR_RADIO_AVERAGE				= "Average";
	static const String HEIGHTMAP_EDITOR_RADIO_DROPPER				= "Dropper";
	static const String HEIGHTMAP_EDITOR_RADIO_RELATIVE				= "Relative";
	static const String HEIGHTMAP_EDITOR_CHECKBOX_HEIGHTMAP			= "Height Map";
	static const String HEIGHTMAP_EDITOR_CHECKBOX_TILEMASK			= "Tile Mask";
	static const String HEIGHTMAP_EDITOR_LABEL_BRUSH_IMAGE			= "Brush\nimage:";
	static const String HEIGHTMAP_EDITOR_LABEL_DROPPER_HEIGHT		= "Height:";

	static const String NOT_PASSABLE_TERRAIN_ENABLE_ERROR			= "Error enabling Not Passable Terrain. Make sure there is valid landscape at the scene.";

	static const String SHORTCUT_BRUSH_SIZE_INCREASE_SMALL			= "landscape-editor.brush-size.increase.small";
	static const String SHORTCUT_BRUSH_SIZE_DECREASE_SMALL			= "landscape-editor.brush-size.decrease.small";
	static const String SHORTCUT_BRUSH_SIZE_INCREASE_LARGE			= "landscape-editor.brush-size.increase.large";
	static const String SHORTCUT_BRUSH_SIZE_DECREASE_LARGE			= "landscape-editor.brush-size.decrease.large";
	static const String SHORTCUT_BRUSH_IMAGE_NEXT					= "landscape-editor.brush-image.next";
	static const String SHORTCUT_BRUSH_IMAGE_PREV					= "landscape-editor.brush-image.prev";
	static const String SHORTCUT_TEXTURE_NEXT						= "landscape-editor.texture.next";
	static const String SHORTCUT_TEXTURE_PREV						= "landscape-editor.texture.prev";
	static const String SHORTCUT_STRENGTH_INCREASE_SMALL			= "landscape-editor.strength.increase.small";
	static const String SHORTCUT_STRENGTH_DECREASE_SMALL			= "landscape-editor.strength.decrease.small";
	static const String SHORTCUT_STRENGTH_INCREASE_LARGE			= "landscape-editor.strength.increase.large";
	static const String SHORTCUT_STRENGTH_DECREASE_LARGE			= "landscape-editor.strength.decrease.large";
	static const String SHORTCUT_AVG_STRENGTH_INCREASE_SMALL		= "landscape-editor.average-strength.increase.small";
	static const String SHORTCUT_AVG_STRENGTH_DECREASE_SMALL		= "landscape-editor.average-strength.decrease.small";
	static const String SHORTCUT_AVG_STRENGTH_INCREASE_LARGE		= "landscape-editor.average-strength.increase.large";
	static const String SHORTCUT_AVG_STRENGTH_DECREASE_LARGE		= "landscape-editor.average-strength.decrease.large";
	static const String SHORTCUT_VISIBILITY_TOOL_SET_POINT			= "visibility-tool.set-point";
	static const String SHORTCUT_VISIBILITY_TOOL_SET_AREA			= "visibility-tool.set-area";
	static const String SHORTCUT_SET_COPY_PASTE						= "heightmap-editor.set-copy-paste";
	static const String SHORTCUT_SET_ABSOLUTE						= "heightmap-editor.set-absolute";
	static const String SHORTCUT_SET_RELATIVE						= "heightmap-editor.set-relative";
	static const String SHORTCUT_SET_AVERAGE						= "heightmap-editor.set-average";
	static const String SHORTCUT_SET_ABS_DROP						= "heightmap-editor.set-abs-drop";
	static const String SHORTCUT_SET_DROPPER						= "heightmap-editor.set-dropper";
	static const String SHORTCUT_NORMAL_DRAW_TILEMASK				= "tilemaskeditor-editor.set-normal-draw";
	static const String SHORTCUT_COPY_PASTE_TILEMASK				= "tilemaskeditor-editor.set-copy-paste";

	static const String NO_LANDSCAPE_ERROR_MESSAGE					= "Error. Check is there landscape at the scene.";
	static const String INVALID_LANDSCAPE_MESSAGE					= "Error. Check if all necessary properties of the landscape are set.";
    
    static const String SCENE_NODE_DESIGNER_NAME_PROPERTY_NAME = "editor.designerName";
	static const String SCENE_NODE_MODIFICATION_DATA_PROPERTY_NAME = "editor.modificationData";

	static const String SLIDER_WIDGET_CHANGE_VALUE_TOOLTIP			= "Double click to change value";
	static const String SLIDER_WIDGET_CURRENT_VALUE					= "Current value";

	static const String TILE_TEXTURE_PREVIEW_CHANGE_COLOR_TOOLTIP	= "Click to change color";

	static const String LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS				= "No errors.";
	static const String LANDSCAPE_EDITOR_SYSTEM_DISABLE_EDITORS			= "Error: All another landscape editors should be disabled";
	static const String LANDSCAPE_EDITOR_SYSTEM_LANDSCAPE_ENTITY_ABSENT	= "Error: landscape entity is absent.";
	static const String LANDSCAPE_EDITOR_SYSTEM_TILEMASK_TEXTURE_ABSETN	= "Error: tile mask texture is absent.";
	static const String LANDSCAPE_EDITOR_SYSTEM_FULL_TILED_TEXTURE_ABSETN= "Error: full tiled texture is absent.";
	static const String LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE0_ABSENT	= "Error: tile texture 0 is absent.";
	static const String LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE1_ABSENT	= "Error: tile texture 1 is absent.";
	static const String LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE2_ABSENT	= "Error: tile texture 2 is absent.";
	static const String LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE3_ABSENT	= "Error: tile texture 3 is absent.";
	static const String LANDSCAPE_EDITOR_SYSTEM_HEIGHTMAP_ABSENT		= "Error: heightmap is absent.";

	static const String ADD_SWITCH_NODE_DIALOG_NO_CHILDREN			= "Error: switch node must be created with state nodes.";
	static const String SCENE_TREE_WRONG_REF_TO_OWNER				= "Wrong reference(s) to owner: ";

}

#endif //#ifndef __STRING_CONSTANTS_H__
