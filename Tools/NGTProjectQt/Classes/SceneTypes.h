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
	ST_MODIF_SCALE,
};

enum ST_Axis
{
	ST_AXIS_NONE = 0,

	ST_AXIS_X = 0x1,
	ST_AXIS_Y = 0x2,
	ST_AXIS_Z = 0x4,
	ST_AXIS_XY = ST_AXIS_X | ST_AXIS_Y,
	ST_AXIS_XZ = ST_AXIS_X | ST_AXIS_Z,
	ST_AXIS_YZ = ST_AXIS_Y | ST_AXIS_Z,
};

#endif // __SCENE_STATE_H__
