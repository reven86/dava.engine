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


#ifndef __DAVAENGINE_QUADTREE_H__
#define __DAVAENGINE_QUADTREE_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"

namespace DAVA
{
    
template<typename T>
struct AbstractQuadTreeNode
{
    T data;
    AbstractQuadTreeNode<T>** children; //*[4]
    AbstractQuadTreeNode<T>* parent;
    
    inline AbstractQuadTreeNode();
    inline ~AbstractQuadTreeNode();
    
    inline bool IsTerminalLeaf() const;
};

template<typename T>
class AbstractQuadTree
{
public:
    
    inline AbstractQuadTree();
    inline ~AbstractQuadTree();
    
    inline void Init(uint32 depth);
    inline void Clear();
    inline AbstractQuadTreeNode<T>* GetRoot() const;
    inline uint32 GetLevelCount() const;
    
private:
    
    void BuildTreeNode(AbstractQuadTreeNode<T>* node, int32 currentDepth);
    
private:
    
    AbstractQuadTreeNode<T>* root;
    uint32 treeDepth;
};

template<typename T>
inline AbstractQuadTreeNode<T>::AbstractQuadTreeNode()
{
    children = NULL;
    parent = NULL;
}
    
template<typename T>
inline AbstractQuadTreeNode<T>::~AbstractQuadTreeNode()
{
    if(!IsTerminalLeaf())
    {
        for(uint32 i = 0; i < 4; ++i)
        {
            SafeDelete(children[i]);
        }
    }
    
    SafeDeleteArray(children);
}
  
template<typename T>
inline bool AbstractQuadTreeNode<T>::IsTerminalLeaf() const
{
    return (NULL == children);
}
    
template<typename T>
inline AbstractQuadTree<T>::AbstractQuadTree() :
        root(NULL),
        treeDepth(0)
{
}

template<typename T>
inline AbstractQuadTree<T>::~AbstractQuadTree()
{
    Clear();
}

template<typename T>
inline void AbstractQuadTree<T>::Init(uint32 depth)
{
    Clear();
    
    treeDepth = depth;
    
    if(treeDepth > 0)
    {
        root = new AbstractQuadTreeNode<T>();
        BuildTreeNode(root, treeDepth);
    }
}

template<typename T>
inline void AbstractQuadTree<T>::Clear()
{
    SafeDelete(root);
    treeDepth = 0;
}
    
template<typename T>
inline AbstractQuadTreeNode<T>* AbstractQuadTree<T>::GetRoot() const
{
    return root;
}

template<typename T>
inline uint32 AbstractQuadTree<T>::GetLevelCount() const
{
    return treeDepth;
}

template<typename T>
void AbstractQuadTree<T>::BuildTreeNode(AbstractQuadTreeNode<T>* node, int32 currentDepth)
{
    currentDepth--;
    
    if(currentDepth >= 0)
    {
        node->children = new AbstractQuadTreeNode<T>*[4];
        for(uint32 i = 0; i < 4; ++i)
        {
            node->children[i] = new AbstractQuadTreeNode<T>();
            node->children[i]->parent = node;
            
            BuildTreeNode(node->children[i], currentDepth);
        }
    }
}
    
};

#endif
