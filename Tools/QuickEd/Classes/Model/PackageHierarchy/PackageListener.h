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


#ifndef __QUICKED_PACKAGE_LISTENER_H__
#define __QUICKED_PACKAGE_LISTENER_H__

class PackageNode;
class ControlNode;
class ControlsContainerNode;
class StyleSheetNode;
class StyleSheetsNode;
class PackageControlsNode;
class AbstractProperty;
class ImportedPackagesNode;

class PackageListener
{
public:
    virtual void ControlPropertyWasChanged(ControlNode *node, AbstractProperty *property) = 0;
    virtual void StylePropertyWasChanged(StyleSheetNode *node, AbstractProperty *property) = 0;
    
    virtual void ControlWillBeAdded(ControlNode *node, ControlsContainerNode *destination, int index) = 0;
    virtual void ControlWasAdded(ControlNode *node, ControlsContainerNode *destination, int index) = 0;
    
    virtual void ControlWillBeRemoved(ControlNode *node, ControlsContainerNode *from) = 0;
    virtual void ControlWasRemoved(ControlNode *node, ControlsContainerNode *from) = 0;
    
    virtual void StyleWillBeAdded(StyleSheetNode *node, StyleSheetsNode *destination, int index) = 0;
    virtual void StyleWasAdded(StyleSheetNode *node, StyleSheetsNode *destination, int index) = 0;
    
    virtual void StyleWillBeRemoved(StyleSheetNode *node, StyleSheetsNode *from) = 0;
    virtual void StyleWasRemoved(StyleSheetNode *node, StyleSheetsNode *from) = 0;
    
    virtual void ImportedPackageWillBeAdded(PackageNode *node, ImportedPackagesNode *to, int index) = 0;
    virtual void ImportedPackageWasAdded(PackageNode *node, ImportedPackagesNode *to, int index) = 0;

    virtual void ImportedPackageWillBeRemoved(PackageNode *node, ImportedPackagesNode *from) = 0;
    virtual void ImportedPackageWasRemoved(PackageNode *node, ImportedPackagesNode *from) = 0;
    
};

#endif // __QUICKED_PACKAGE_LISTENER_H__
