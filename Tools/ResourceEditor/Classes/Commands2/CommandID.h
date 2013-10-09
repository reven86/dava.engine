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



#ifndef __COMMAND_ID_H__
#define __COMMAND_ID_H__

enum CommandID
{
	CMDID_UNKNOWN	= -1,
	CMDID_BATCH		=  0,

	CMDID_TRANSFORM,

	CMDID_ENTITY_REMOVE,
	CMDID_ENTITY_MOVE,
	CMDID_PARTICLE_LAYER_REMOVE,
	CMDID_PARTICLE_LAYER_MOVE,
	CMDID_PARTICLE_FORCE_REMOVE,
	CMDID_PARTICLE_FORCE_MOVE,
	CMDID_ADD_ENTITY,
	CMDID_GROUP_ENTITIES_FOR_MULTISELECT,

	CMDID_META_OBJ_MODIFY,

	CMDID_MODIFY_CUSTOM_COLORS,
	CMDID_SET_VISIBILITY_POINT,
	CMDID_SET_VISIBILITY_AREA,
	CMDID_DRAW_HEIGHTMAP,
	CMDID_COPY_PASTE_HEIGHTMAP,
	CMDID_MODIFY_TILEMASK,
	
	CMDID_UPDATE_PARTICLE_EFFECT,
	CMDID_UPDATE_PARTICLE_EMITTER,
	CMDID_UPDATE_PARTICLE_LAYER,
	CMDID_UPDATE_PARTILCE_LAYER_TIME,
	CMDID_UPDATE_PARTICLE_LAYER_ENABLED,
	CMDID_UPDATE_PARTICLE_LAYER_LODS,
	CMDID_UPDATE_PARTICLE_FORCE,

	CMDID_ADD_PARTICLE_EMITTER,

	CMDID_START_STOP_PARTICLE_EFFECT,
	CMDID_RESTART_PARTICLE_EFFECT,

	CMDID_ADD_PARTICLE_EMITTER_LAYER,
	CMDID_REMOVE_PARTICLE_EMITTER_LAYER,
	CMDID_CLONE_PARTICLE_EMITTER_LAYER,
	CMDID_ADD_PARTICLE_EMITTER_FORCE,
	CMDID_REMOVE_PARTICLE_EMITTER_FORCE,
	
	CMDID_LOAD_PARTICLE_EMITTER_FROM_YAML,
	CMDID_SAVE_PARTICLE_EMITTER_TO_YAML,

	CMDID_DAE_CONVERT,
	CMDID_SAVE_ENTITY_AS,
    
    CMDID_CONVERT_TO_SHADOW,

	CMDID_USER		= 0xF000
};

#endif // __COMMAND_ID_H__
