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


#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

namespace ResourceEditor 
{

enum eNodeType
{
    NODE_LANDSCAPE  = 0,
    NODE_LIGHT,
    NODE_SERVICE_NODE,
    NODE_CAMERA,
    NODE_IMPOSTER,
    NODE_PARTICLE_EMITTER,
    NODE_USER_NODE,
	NODE_SWITCH_NODE,
	NODE_PARTICLE_EFFECT,
    
    NODE_COUNT
};
    
enum eViewportType
{
    VIEWPORT_IPHONE = 0,
    VIEWPORT_RETINA,
    VIEWPORT_IPAD,
    VIEWPORT_DEFAULT,
    
    VIEWPORT_COUNT
};
  
  
enum eHideableWidgets
{
    HIDABLEWIDGET_SCENE_GRAPH = 0,
    HIDABLEWIDGET_PROPERTIES,
    HIDABLEWIDGET_LIBRARY,
    HIDABLEWIDGET_TOOLBAR,
	HIDABLEWIDGET_REFERENCES,
    HIDABLEWIDGET_CUSTOMCOLORS,
	HIDEBLEWIDGET_VISIBILITYCHECKTOOL,
    HIDEBLEWIDGET_PARTICLE_EDITOR,
	HIDEBLEWIDGET_HANGINGOBJECTS,
	HIDEBLEWIDGET_SETSWITCHINDEX,
	HIDEBLEWIDGET_SCENEINFO,
	HIDABLEWIDGET_CUSTOMCOLORS2,
	HIDEBLEWIDGET_VISIBILITYCHECKTOOL2,
	HIDEBLEWIDGET_HEIGHTMAPEDITOR,
	HIDEBLEWIDGET_TILEMASKEDITOR,

    HIDABLEWIDGET_COUNT
};
	
enum eModificationActions
{
	MODIFY_NONE = 0,
	MODIFY_MOVE,
	MODIFY_ROTATE,
	MODIFY_SCALE,
	MODIFY_PLACE_ON_LAND,
	MODIFY_SNAP_TO_LAND,

	MODIFY_COUNT
};

enum eEditActions
{
	EDIT_UNDO,
	EDIT_REDO,
	
	EDIT_COUNT
};

// list: ["No Collision", "Tree", "Bush", "Fragile Proj", "Fragile ^Proj", "Falling", "Building", "Invisible Wall", "Speed Tree", "Undefined collision"]
enum eSceneObjectType
{
	ESOT_NONE = -1,
	ESOT_NO_COLISION,
	ESOT_TREE,
	ESOT_BUSH,
	ESOT_FRAGILE_PROJ,
	ESOT_FRAGILE_PROJ_INV,
	ESOT_FALLING,
	ESOT_BUILDING,
	ESOT_INVISIBLE_WALL,
    ESOT_SPEED_TREE,
    ESOT_UNDEFINED_COLLISION,

	ESOT_COUNT,
};

// coefficient for converting brush size from UI value to system value for landscape editors
const DAVA::float32 LANDSCAPE_BRUSH_SIZE_UI_TO_SYSTEM_COEF = 4.0f;

// default coefficient for converting brush size from UI value to system value for heightmap editors
// heightmap size in heightmap editor is almost 4 times smaller than landscape texture size
const DAVA::float32 HEIGHTMAP_BRUSH_SIZE_UI_TO_SYSTEM_COEF = 4.0f;

const DAVA::int32 SLIDER_WIDGET_CHANGE_VALUE_STEP_SMALL = 1;
const DAVA::int32 SLIDER_WIDGET_CHANGE_VALUE_STEP_LARGE = 10;

const DAVA::int32 BRUSH_MIN_BOUNDARY = 1;
const DAVA::int32 BRUSH_MAX_BOUNDARY = 999;
    
const DAVA::int32 DEFAULT_TOOLBAR_CONTROL_SIZE_WITH_TEXT = 100;
const DAVA::int32 DEFAULT_TOOLBAR_CONTROL_SIZE_WITH_ICON = 40;


};




#endif //#ifndef __CONSTANTS_H__