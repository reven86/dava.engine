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



#ifndef RESOURCEEDITORQT_COMMANDLIST_H
#define RESOURCEEDITORQT_COMMANDLIST_H

class CommandList
{
public:
	enum eCommandId
	{
		ID_COMMAND_UNKNOWN = 0,

		ID_COMMAND_CHANGE_MATERIAL_VIEW_OPTION,

		ID_COMMAND_TOGGLE_SET_SWITCH_INDEX,

		ID_COMMAND_SAVE_TEXTURE_VISIBILITY_TOOL,
		ID_COMMAND_PLACE_POINT_VISIBILITY_TOOL,
		ID_COMMAND_PLACE_AREA_VISIBILITY_TOOL,

		ID_COMMAND_RELOAD_TEXTURES_AS,
        ID_COMMAND_REPLACE_MIPMAP_LEVEL,

		ID_COMMAND_SAVE_TEXTURE_CUSTOM_COLORS,
		ID_COMMAND_LOAD_TEXTURE_CUSTOM_COLORS,
		ID_COMMAND_DRAW_CUSTOM_COLORS,

		ID_COMMAND_ADD_PARTICLE_EMITTER,
		ID_COMMAND_START_STOP_PARTICLE_EFFECT,
		ID_COMMAND_RESTART_PARTICLE_EFFECT,
		ID_COMMAND_ADD_PARTICLE_EMITTER_LAYER,
		ID_COMMAND_REMOVE_PARTICLE_EMITTER_LAYER,
		ID_COMMAND_CLONE_PARTICLE_EMITTER_LAYER,
		ID_COMMAND_ADD_PARTICLE_EMITTER_FORCE,
		ID_COMMAND_REMOVE_PARTICLE_EMITTER_FORCE,
		ID_COMMAND_UPDATE_EFFECT,
		ID_COMMAND_UPDATE_EMITTER,
		ID_COMMAND_UPDATE_PARTICLE_LAYER,
		ID_COMMAND_UPDATE_PARTICLE_LAYER_TIME,
		ID_COMMAND_UPDATE_PARTICLE_LAYER_ENABLED,
		ID_COMMAND_UPDATE_PARTICLE_FORCE,
		ID_COMMAND_LOAD_PARTICLE_EMITTER_FROM_YAML,
		ID_COMMAND_SAVE_PARTICLE_EMITTER_TO_YAML,

		ID_COMMAND_LOAD_INNER_EMITTER_FROM_YAML,
		ID_COMMAND_SAVE_INNER_EMITTER_TO_YAML,

		ID_COMMAND_ADD_SCENE,
		ID_COMMAND_EDIT_SCENE,
		ID_COMMAND_RELOAD_SCENE,
		ID_COMMAND_RELOAD_ENTITY_FROM,
		ID_COMMAND_CONVERT_SCENE,

		ID_COMMAND_REMOVE_ROOT_NODES,
		ID_COMMAND_LOOK_AT_OBJECT,
		ID_COMMAND_REMOVE_SCENE_NODE,
		ID_COMMAND_INTERNAL_REMOVE_SCENE_NODE,
		ID_COMMAND_DEBUG_FLAGS,

		ID_COMMAND_BEAST,
		ID_COMMAND_CONVERT_TO_SHADOW,

		ID_COMMAND_CREATE_NODE,
		ID_COMMAND_OPEN_SCENE,
		ID_COMMAND_SAVE_SPECIFIED_SCENE,
		ID_COMMAND_EXPORT,
		ID_COMMAND_SAVE_TO_FOLDER_WITH_CHILDS,

		ID_COMMAND_DRAW_TILEMAP,

		ID_COMMAND_DRAW_HEIGHTMAP,
		ID_COMMAND_COPY_PASTE_HEIGHTMAP,

		ID_COMMAND_TRANSFORM_OBJECT,
		ID_COMMAND_CLONE_OBJECT,
		ID_COMMAND_CLONE_AND_TRANSFORM,
		ID_COMMAND_PLACE_ON_LANDSCAPE,
		ID_COMMAND_RESTORE_ORIGINAL_TRANSFORM,

		ID_COMMAND_CREATE_NODE_SCENE_EDITOR,

		ID_COMMAND_UNITE_ENTITIES_FOR_MULTISELECT,

		ID_COMMAND_COUNT
	};
};


/*
 * MaterialViewOptionsCommands.h
 */
class CommandChangeMaterialViewOption;


/*
 * VisibilityCheckToolCommands.h
 */
class CommandSaveTextureVisibilityTool;
class CommandPlacePointVisibilityTool;
class CommandPlaceAreaVisibilityTool;


/*
 * TextureOptionsCommands.h
 */
class ReloadTexturesAsCommand;


/*
 * CustomColorCommands.h
 */
class CommandSaveTextureCustomColors;
class CommandLoadTextureCustomColors;
class CommandDrawCustomColors;


/*
 * ParticleEditorCommands.h
 */
class CommandAddParticleEmitter;
class CommandStartStopParticleEffect;
class CommandRestartParticleEffect;
class CommandAddParticleEmitterLayer;
class CommandRemoveParticleEmitterLayer;
class CommandCloneParticleEmitterLayer;
class CommandAddParticleEmitterForce;
class CommandRemoveParticleEmitterForce;
class CommandUpdateEffect;
class CommandUpdateEmitter;
class CommandUpdateParticleLayer;
class CommandUpdateParticleLayerTime;
class CommandUpdateParticleLayerEnabled;
class CommandUpdateParticleForce;
class CommandLoadParticleEmitterFromYaml;
class CommandSaveParticleEmitterToYaml;


/*
 * LibraryCommands.h
 */
class CommandAddScene;
class CommandEditScene;
class CommandReloadScene;
class CommandReloadEntityFrom;
class CommandConvertScene;


/*
 * SceneGraphCommands.h
 */
class CommandRemoveRootNodes;
class CommandLockAtObject;
class CommandRemoveSceneNode;
class CommandInternalRemoveSceneNode;
class CommandDebugFlags;


/*
 * ToolsCommands.h
 */
class CommandConvertToShadow;


/*
 * CommandCreateNode.h
 */
class CommandCreateNode;


/*
 * FileCommands.h
 */
class CommandOpenScene;
class CommandExport;
class CommandSaveToFolderWithChilds;


/*
 * TilemapEditorCommands.h
 */
class CommandDrawTilemap;


/*
 * HeightmapEditorCommands.h
 */
class CommandDrawHeightmap;
class CommandCopyPasteHeightmap;


/*
 * EditorBodyControlCommands.h
 */
class CommandTransformObject;
class CommandCloneObject;
class CommandCloneAndTransform;
class CommandPlaceOnLandscape;
class CommandRestoreOriginalTransform;


/*
 * SceneEditorScreenMainCommands.h
 */
class CommandCreateNodeSceneEditor;

#endif
