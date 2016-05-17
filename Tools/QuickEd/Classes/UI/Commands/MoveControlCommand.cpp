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


#include "MoveControlCommand.h"

#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/PackageNode.h"

using namespace DAVA;

MoveControlCommand::MoveControlCommand(PackageNode* root_, ControlNode* node_,
                                       ControlsContainerNode* src_, int srcIndex_,
                                       ControlsContainerNode* dest_, int destIndex_,
                                       QUndoCommand* parent)
    : QUndoCommand(parent)
    , root(SafeRetain(root_))
    , node(SafeRetain(node_))
    , src(SafeRetain(src_))
    , srcIndex(srcIndex_)
    , dest(SafeRetain(dest_))
    , destIndex(destIndex_)
{
}

MoveControlCommand::~MoveControlCommand()
{
    SafeRelease(root);
    SafeRelease(node);
    SafeRelease(src);
    SafeRelease(dest);
}

void MoveControlCommand::redo()
{
    root->RemoveControl(node, src);
    root->InsertControl(node, dest, destIndex);
}

void MoveControlCommand::undo()
{
    root->RemoveControl(node, dest);
    root->InsertControl(node, src, srcIndex);
}
