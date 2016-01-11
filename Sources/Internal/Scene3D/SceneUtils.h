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
#ifndef __DAVAENGINE_SCENE_UTILS_H__
#define __DAVAENGINE_SCENE_UTILS_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
class RenderObject;
class RenderBatch;
class Entity;
class Scene;

namespace SceneUtils
{
/*
 \brief Iterates over all scene nodes, searches nodes with speciefic names and makes new nodes with lods.
*/
void CombineLods(Scene * scene);

/*
 \brief Creates Entity name for speciefic lod index by pattern.
 */
String LodNameForIndex(const String & pattern, uint32 lodIndex);

/*
 \brief Finds all enitities with SameName_lod0 and then iterates over that nodes and searches similar ones with another lods.
        Then takes all found nodes with the same name and different lods and makes one new entity with multiple render barches for 
        different lods. It is our convience - "EntityName_lod%d". If we'll change it - we should write new function like this.
 */
void CombineEntityLods(Entity * forRootNode);

/*
 \brief Iterates ecursive over all nodes from currentNode and deeper. 
        Accumulates transform from parent to currentNode.
        Bakes accumulated transform into currentNode.
 
        In result we have all entities from currentNode and deeper with IDENTITY transforms and transformed geometry which looks the same as before.
*/
void BakeTransformsUpToFarParent(Entity * parent, Entity * currentNode);

/*
 \brief Takes all childs render batches of node Entity recursive and puts them into node entity as render batches for one lod.
 */
void CollapseRenderBatchesRecursiveAsLod(Entity * node, uint32 lod, RenderObject * ro);

/*
 brief Takes all node childs animations and puts them into node.
 */
void CollapseAnimationsUpToFarParent(Entity * node, Entity * parent);

} //namespace SceneUtils

} //namespace DAVA

#endif //__DAVAENGINE_SCENE_UTILS_H__
