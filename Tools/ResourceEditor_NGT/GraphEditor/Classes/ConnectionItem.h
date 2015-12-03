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

#ifndef __GRAPHEDITOR_CONNECTIONITEM_H__
#define __GRAPHEDITOR_CONNECTIONITEM_H__

#include <QObject>
#include <QQuickItem>

class ConnectionItem : public QQuickItem
{
    Q_OBJECT
public:
    ConnectionItem(QQuickItem* parent = 0);
    virtual ~ConnectionItem();

    size_t GetUID() const;

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;

private:
    Q_PROPERTY(QQuickItem* outputSlot READ GetOutputSlotItem WRITE SetOutputSlotItem);
    Q_PROPERTY(QQuickItem* inputSlot READ GetInputSlotItem WRITE SetInputSlotItem);
    Q_PROPERTY(QVariant uid READ GetQmlUID WRITE SetQmlUID)

    QQuickItem* GetOutputSlotItem() const;
    QQuickItem* GetInputSlotItem() const;
    void SetOutputSlotItem(QQuickItem* item);
    void SetInputSlotItem(QQuickItem* item);
    QVariant GetQmlUID() const;
    void SetQmlUID(QVariant const& uid);

private:
    QQuickItem* outputSlot = nullptr;
    QQuickItem* inputSlot = nullptr;

    size_t uid = 0;
};

class InteractiveConnectionItem : public QQuickItem
{
    Q_OBJECT
public:
    InteractiveConnectionItem(QQuickItem* parent = 0);

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;

private:
    Q_PROPERTY(float startX READ GetStartX WRITE SetStartX);
    Q_PROPERTY(float startY READ GetStartY WRITE SetStartY);
    Q_PROPERTY(float endX READ GetEndX WRITE SetEndX);
    Q_PROPERTY(float endY READ GetEndY WRITE SetEndY);

    float GetStartX() const;
    float GetStartY() const;
    float GetEndX() const;
    float GetEndY() const;

    void SetStartX(float x);
    void SetStartY(float y);
    void SetEndX(float x);
    void SetEndY(float y);

private:
    QPointF startPt;
    QPointF endPt;
};

#endif // __GRAPHEDITOR_CONNECTIONITEM_H__