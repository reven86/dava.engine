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


#ifndef __UIEditor__PlatformMetadata__
#define __UIEditor__PlatformMetadata__

#include "BaseMetadata.h"
#include "HierarchyTreePlatformNode.h"
#include "HierarchyTreeScreenNode.h"

namespace DAVA {

// Metadata class for Platform Node.
class PlatformMetadata : public BaseMetadata
{
    Q_OBJECT
    
    // Properties which are specific for Platform Node..
    // Width and height.
    Q_PROPERTY(float Width READ GetWidth WRITE SetWidth);
    Q_PROPERTY(float Height READ GetHeight WRITE SetHeight);
    
protected:
    // Default Flags.
    virtual bool GetInitialInputEnabled() const {return true;};

    // Rename the platform.
    virtual void ApplyRename(const QString& originalName, const QString& newName);

    // Accessors to the Tree Node.
    HierarchyTreePlatformNode* GetPlatformNode() const;

    // Getters/setters.
    virtual QString GetName() const;
    virtual void SetName(const QString& name);
    
    float GetHeight() const;
    void SetHeight(float value);
    float GetWidth() const;
    void SetWidth(float value);
};
    
};

#endif /* defined(__UIEditor__PlatformMetadata__) */
