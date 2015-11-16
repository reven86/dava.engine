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


#ifndef __COLLADAINCLUDES_H__
#define __COLLADAINCLUDES_H__

#include "FCollada.h"

#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDLibrary.h"
#include "FCDocument/FCDGeometry.h"
#include "FCDocument/FCDImage.h"
#include "FCDocument/FCDSceneNode.h"
#include "FCDocument/FCDTransform.h"

#include "FCDocument/FCDGeometryMesh.h"
#include "FCDocument/FCDGeometryPolygons.h"
#include "FCDocument/FCDGeometrySource.h"
#include "FCDocument/FCDGeometryPolygonsTools.h"
#include "FCDocument/FCDGeometryPolygonsInput.h"
#include "FCDocument/FCDGeometryInstance.h"

#include "FCDocument/FCDLight.h"
#include "FCDocument/FCDMaterial.h"
#include "FCDocument/FCDMaterialInstance.h"
#include "FCDocument/FCDController.h"
#include "FCDocument/FCDControllerInstance.h"
#include "FCDocument/FCDSkinController.h"
#include "FCDocument/FCDAnimated.h"
#include "FCDocument/FCDAnimationCurve.h"
#include "FCDocument/FCDAnimation.h"
#include "FCDocument/FCDAnimationChannel.h"
#include "FCDocument/FCDAnimationKey.h"
#include "FCDocument/FCDCamera.h"


#include <FMath/FMMatrix44.h>

#include "Base/BaseMath.h"

#include <string>
#include <vector>
#include <list>
#include <map>

#define COLLADA_GLUT_RENDER

#ifdef COLLADA_GLUT_RENDER

    #if defined (__DAVAENGINE_WIN32__)
        #include <GL/glew.h>
    #endif

	#if defined(_WIN32)
	#	include <GL/glut.h>
	#	include <direct.h>
	#	define getcwd _getcwd

	#elif defined(__APPLE__) || defined(MACOSX)
	#	include <GLUT/glut.h>
	#endif

#endif // COLLADA_GLUT_RENDER


namespace DAVA 
{
	Matrix4 ConvertMatrix( FMMatrix44 & matrix );
	Matrix4 ConvertMatrixT( FMMatrix44 & matrix );
};
#endif // __COLLADAINCLUDES_H__


