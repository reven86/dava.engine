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



#include "GraphItem.h"
#include "DockSceneGraph/PointerHolder.h"

GraphItem::GraphItem(GraphItem *parent)
    :   userData(NULL)
{
    RegisterPointerType<GraphItem *>(String("GraphItem *"));
    
    SetParent(parent);
}

GraphItem::~GraphItem()
{
    parentItem = NULL;
    
	for_each(children.begin(), children.end(), SafeRelease<GraphItem>);
	children.clear();
}

void GraphItem::AppendChild(GraphItem *child)
{
    DVASSERT(child && "child can't be NULL.");
    
    child->SetParent(this);
	children.push_back(child);
}

void GraphItem::InsertChild(GraphItem *child, int32 pos)
{
    DVASSERT(child && "child can't be NULL.");
    DVASSERT((0 <= pos && pos < ChildrenCount()) && "Wrong position of insertion");
 
    Vector<GraphItem *>::iterator it = children.begin();
    std::advance(it, pos);
    children.insert(it, child);
}

void GraphItem::RemoveChild(int32 row)
{
    DVASSERT((0 <= row) && (row < ChildrenCount()) && "Wrong index");
    
    SafeRelease(children[row]);

    Vector<GraphItem *>::iterator rowIt = children.begin();
    std::advance(rowIt, row);
    children.erase(rowIt);
}

void GraphItem::RemoveChild(GraphItem *child)
{
    DVASSERT((NULL != child) && "Child can be NULL");
    
    child->SetParent(NULL);
    
    Vector<GraphItem *>::const_iterator endIt = children.end();
    for(Vector<GraphItem *>::iterator it = children.begin(); it != endIt; ++it)
    {
        if(*it == child)
        {
            children.erase(it);
            return;
        }
    }
}


GraphItem *GraphItem::Child(int32 row)
{
    return children[row];
}

int32 GraphItem::ChildrenCount() const
{
    return (int32)children.size();
}

int32 GraphItem::Row() const
{
    if (parentItem)
	{
		int32 count = parentItem->ChildrenCount();
		for(int32 i = 0; i < count; ++i)
		{
			if(parentItem->Child(i) == this)
			{
				return i;
			}
		}
	}

    return 0;
}

int32 GraphItem::ColumnCount() const
{
    return 1;
}

GraphItem *GraphItem::GetParent()
{
    return parentItem;
}

void GraphItem::SetParent(GraphItem *parent)
{
    parentItem = parent;
}

void * GraphItem::GetUserData()
{
    return userData;
}



