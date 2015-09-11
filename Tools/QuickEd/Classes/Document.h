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


#ifndef __QUICKED_DOCUMENT_H__
#define __QUICKED_DOCUMENT_H__

#include <QUndoStack>
#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "Functional/Signal.h"
#include "SelectionTracker.h"
#include "Math/Rect.h"


struct WidgetContext
{
    virtual ~WidgetContext() = 0;
};

inline WidgetContext::~WidgetContext()
{
    
}

struct HUDareaInfo
{
    enum eArea
    {
        TOP_LEFT_AREA,
        TOP_CENTER_AREA,
        TOP_RIGHT_AREA,
        CENTER_LEFT_AREA,
        CENTER_RIGHT_AREA,
        BOTTOM_LEFT_AREA,
        BOTTOM_CENTER_AREA,
        BOTTOM_RIGHT_AREA,
        FRAME_AREA,
        PIVOT_POINT_AREA,
        ROTATE_AREA,
        NO_AREA,
        CORNERS_COUNT = FRAME_AREA - TOP_LEFT_AREA,
        AREAS_COUNT = NO_AREA - TOP_LEFT_AREA
    };
    ControlNode *owner = nullptr;
    eArea area = NO_AREA;
};

namespace DAVA {
    class FilePath;
    class UIControl;
    class UIEvent;
}

class BaseSystem;
class PackageNode;
class QtModelPackageCommandExecutor;

class PropertiesModel;
class PackageModel;
class ControlNode;
class AbstractProperty;

class Document final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int scale READ GetScale WRITE SetScale NOTIFY ScaleChanged RESET ResetScale);
    Q_PROPERTY(bool emulationMode READ IsInEmulationMode WRITE SetEmulationMode NOTIFY EmulationModeChanged RESET ClearEmulationMode)
    
public:
    explicit Document(PackageNode *package, QObject *parent = nullptr);
    ~Document();

    int GetScale() const;
    bool IsInEmulationMode() const;
    DAVA::UIControl* GetRootControl();
    DAVA::UIControl* GetScalableControl();
    const DAVA::FilePath &GetPackageFilePath() const;
    QUndoStack *GetUndoStack();
    PackageNode *GetPackage();
    QtModelPackageCommandExecutor *GetCommandExecutor();
    WidgetContext* GetContext(QObject* requester) const;

    void Deactivate();
    void Activate();

    void SetContext(QObject* requester, WidgetContext* widgetContext);

    bool OnInput(DAVA::UIEvent *currentInput);

    void RefreshLayout();

    void GetControlNodesByPos(DAVA::Vector<ControlNode*> &controlNodes, const DAVA::Vector2 &pos) const;
    void GetControlNodesByRect(SelectedControls &controlNodes, const DAVA::Rect &rect) const;
    ControlNode* GetControlByMenu(const DAVA::Vector<ControlNode*> &nodes, const DAVA::Vector2 &pos) const;

    DAVA::Signal<const SelectedNodes& /*selected*/, const SelectedNodes& /*deselected*/> SelectionChanged;
    DAVA::Signal<const HUDareaInfo &/*areaInfo*/> ActiveAreaChanged;
    DAVA::Signal<const DAVA::Rect &/*selectionRectControl*/> SelectionRectChanged;
    DAVA::Signal<bool> EmulationModeChangedSignal;
    
signals:
    void ScaleChanged(int scale);
    void EmulationModeChanged(bool emulationMode);
    void SelectedNodesChanged(const SelectedNodes &selected, const SelectedNodes &deselected);
    void CanvasSizeChanged();
public slots:
    void SetScale(int scale);
    void ResetScale();
    void SetEmulationMode(bool emulationMode);
    void ClearEmulationMode();
    void RefreshAllControlProperties();
    void SetSelectedNodes(const SelectedNodes &selected, const SelectedNodes &deselected);

private:
    void GetControlNodesByPosImpl(DAVA::Vector<ControlNode*> &controlNodes, const DAVA::Vector2 &pos, ControlNode *node) const;
    void GetControlNodesByRectImpl(DAVA::Set<ControlNode*> &controlNodes, const DAVA::Rect &rect, ControlNode *node) const;
    static const int defaultScale = 100;
    int scale = defaultScale;
    static const bool emulationByDefault = false;
    bool emulationMode = emulationByDefault;

    DAVA::UIControl *rootControl = nullptr;
    DAVA::UIControl *scalableControl = nullptr;

    DAVA::UnorderedMap < QObject*, WidgetContext* > contexts;

    PackageNode *package = nullptr;
    QtModelPackageCommandExecutor *commandExecutor = nullptr;
    QUndoStack *undoStack = nullptr;

    QList<BaseSystem*> systems;

    SelectionTracker selectionTracker;
};

#endif // __QUICKED_DOCUMENT_H__
