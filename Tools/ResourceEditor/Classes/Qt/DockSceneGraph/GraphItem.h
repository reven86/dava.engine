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



#ifndef __GRAPH_ITEM_H__
#define __GRAPH_ITEM_H__

#include "DAVAEngine.h"
#include <QVariant>

using namespace DAVA;

class GraphItem: public BaseObject
{
protected:
    virtual ~GraphItem();
public:
    GraphItem(GraphItem *parent = 0);
    
	GraphItem *GetParent();
    void SetParent(GraphItem * parent);

	virtual void SetUserData(void *data) = 0;
    void * GetUserData();

    void AppendChild(GraphItem *child);
    void InsertChild(GraphItem *child, int32 pos);
    void RemoveChild(int32 row);
    void RemoveChild(GraphItem *child);
	GraphItem *Child(int32 row);
	int32 ChildrenCount() const;

	int32 Row() const;
    int32 ColumnCount() const;

	virtual QVariant Data(int32 column) = 0;

protected:

	virtual void ReleaseUserData()= 0;

protected:
	void *userData;

private:
	Vector<GraphItem *>children;
    GraphItem *parentItem;
};

#include "DockSceneGraph/PointerHolder.h"
DECLARE_POINTER_TYPE(GraphItem *);


#endif // __GRAPH_ITEM_H__
