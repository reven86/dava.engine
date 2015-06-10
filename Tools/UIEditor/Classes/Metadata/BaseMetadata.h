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


#ifndef __UIEditor__BaseMetadata__
#define __UIEditor__BaseMetadata__

#include <QObject>
#include <QPoint>
#include <QColor>

#include "HierarchyTreeNode.h"
#include "UI/UIControl.h"

#include "BaseMetadataParams.h"

namespace DAVA {

// Base class for Metadata.
class BaseMetadata : public QObject
{
    Q_OBJECT
	
	Q_PROPERTY(QString Name READ GetName WRITE SetName);
    
public:
    // ExtraData update flags.
    enum eExtraDataUpdateStyle
    {
        // Update the ExtraData with control values.
        UPDATE_EXTRADATA_FROM_CONTROL,
        
        // Update Control with raw ExtraData values.
        UPDATE_CONTROL_FROM_EXTRADATA_RAW,
        
        // Update Control with Localized Extradata values.
        UPDATE_CONTROL_FROM_EXTRADATA_LOCALIZED
    };
    
    BaseMetadata(QObject *parent = 0);
    ~BaseMetadata();

    // Setup/cleanup the Tree Node Metadata Params.
    void SetupParams(const METADATAPARAMSVECT& params);
    void CleanupParams();
    
    // Initialize the control(s) attached.
    virtual void InitializeControl(const String& controlName, const Vector2& position);

    // Get the list of Params attached.
    int GetParamsCount() const;
    const METADATAPARAMSVECT& GetParams() const;
    
    // Get/Set the active Parameter in context. All the properties operations will be executed
    // on this control.
    BaseMetadataParams::METADATAPARAMID GetActiveParamID() const;
    void SetActiveParamID(BaseMetadataParams::METADATAPARAMID paramID);

	// Get/Set the active State in context.
	void SetActiveStateIndex(int32 index);
	int32 GetActiveStateIndex() const;
	void ResetActiveStateIndex();
	uint32 GetStatesCount() const;

    // UI Control State.
    const Vector<UIControl::eControlState> &GetUIControlStates() const;
    void SetUIControlStates(const Vector<UIControl::eControlState>& controlStates);

    // Apply move for all controls.
    virtual void ApplyMove(const Vector2&, bool /*alignControlsToIntegerPos*/) {};
    
    // Apply resize for all controls.
    virtual void ApplyResize(const Rect& /*originalRect*/, const Rect& /*newRect*/) {};
	
	// Apply rename for controls
	virtual void ApplyRename(const QString& /*originalName*/, const QString& /*newName*/);

    // Accessors to the Tree Node.
    HierarchyTreeNode* GetTreeNode(BaseMetadataParams::METADATAPARAMID paramID) const;
    HierarchyTreeNode* GetActiveTreeNode() const;

    // In case the control is not state-aware its Localized Text should be always
    // bound to the first state. This is defined by this virtual function.
    virtual UIControl::eControlState GetCurrentStateForLocalizedText() const;

    // Get the "reference" state to compare all other with.
    virtual UIControl::eControlState GetReferenceState();

    // Helper method to check whether at least one control's property differs from the reference ones.	
    bool IsStateDirty(UIControl::eControlState controlState);

    // The same for particular property and state.
    bool IsStateDirtyForProperty(UIControl::eControlState controlState, const QString& propertyName);
    void SetStateDirtyForProperty(UIControl::eControlState controlState, const QString& propertyName,
                                                bool value);

    // Helper for Active State.
    bool IsActiveStateDirtyForProperty(const QString& propertyName);
    void SetActiveStateDirtyForProperty(const QString& propertyName, bool value);
    
    // Fill ExtraData from attached Control values. Specific for each classes.
    virtual void UpdateExtraData(HierarchyTreeNodeExtraData& /*extraData*/, eExtraDataUpdateStyle /*updateStyle*/) {};

protected:
    // Default Flags.
    virtual bool GetInitialInputEnabled() const = 0;

    // Initialization constants.
    static const Vector2 INITIAL_CONTROL_SIZE;

	// if activeStateIndex equal to STATE_INDEX_DEFAULT, then DEFAULT_STATE_INDEX_VALUE is taken
	// as the result of GetActiveStateIndex()
	static const int32 STATE_INDEX_DEFAULT = -1;
	static const int32 DEFAULT_STATE_INDEX_VALUE = 0;
	
	// Getters/setters.
    virtual QString GetName() const { return QString(); };
    virtual void SetName(const QString& /*name*/) {};

    // Verify whether Param ID is OK.
    bool VerifyParamID(BaseMetadataParams::METADATAPARAMID paramID) const;

    // Verify whether Active Param ID is OK.
    bool VerifyActiveParamID() const;

    // Helper to access active Tree Node ID.
    HierarchyTreeNode::HIERARCHYTREENODEID GetActiveTreeNodeID() const;
    
    // Helper to access active UI Control.
    virtual UIControl* GetActiveUIControl() const;

    // Get the UI control class name.
    virtual QString GetUIControlClassName() const { return QString(); };
    
    // List of Params.
    METADATAPARAMSVECT treeNodeParams;

    // Active Parameter.
    BaseMetadataParams::METADATAPARAMID activeParamID;

	// Active State Index
	int32 activeStateIndex;

    // UI Control State.
    Vector<UIControl::eControlState> uiControlStates;
};

}

#endif /* defined(__UIEditor__BaseMetadata__) */
