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



#ifndef __SCENE_STATE_H__
#define __SCENE_STATE_H__

enum ST_PivotPoint
{
	ST_PIVOT_ENTITY_CENTER,
	ST_PIVOT_COMMON_CENTER
};

enum ST_ModifMode
{
	ST_MODIF_OFF,
	ST_MODIF_MOVE,
	ST_MODIF_ROTATE,
	ST_MODIF_SCALE
};

enum ST_Axis
{
	ST_AXIS_NONE = 0,

	ST_AXIS_X = 0x1,
	ST_AXIS_Y = 0x2,
	ST_AXIS_Z = 0x4,
	ST_AXIS_XY = ST_AXIS_X | ST_AXIS_Y,
	ST_AXIS_XZ = ST_AXIS_X | ST_AXIS_Z,
	ST_AXIS_YZ = ST_AXIS_Y | ST_AXIS_Z
};

enum ST_SelectionDrawMode
{
	ST_SELDRAW_NOTHING = 0x0,

	ST_SELDRAW_DRAW_SHAPE = 0x1,
	ST_SELDRAW_DRAW_CORNERS	= 0x2,
	ST_SELDRAW_FILL_SHAPE = 0x4,
	ST_SELDRAW_NO_DEEP_TEST	= 0x10,

	ST_SELDRAW_ALL = 0xFFFFFFFF
};

enum ST_CollisionDrawMode
{
	ST_COLL_DRAW_NOTHING = 0x0,

	ST_COLL_DRAW_OBJECTS = 0x1,
	ST_COLL_DRAW_OBJECTS_SELECTED = 0x2,
	ST_COLL_DRAW_OBJECTS_RAYTEST = 0x4,

	ST_COLL_DRAW_LAND = 0x10,
	ST_COLL_DRAW_LAND_RAYTEST = 0x20,
	ST_COLL_DRAW_LAND_COLLISION = 0x40,

	ST_COLL_DRAW_ALL = 0xFFFFFFFF
};

#endif // __SCENE_STATE_H__
