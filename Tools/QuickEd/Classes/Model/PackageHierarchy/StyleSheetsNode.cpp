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


#include "StyleSheetsNode.h"

#include "PackageVisitor.h"

using namespace DAVA;

StyleSheetsNode::StyleSheetsNode(PackageBaseNode *parent) : PackageBaseNode(parent)
{
}

StyleSheetsNode::~StyleSheetsNode()
{
    for (StyleSheetNode *styleSheet : styleSheets)
        styleSheet->Release();
    styleSheets.clear();
}

void StyleSheetsNode::Add(StyleSheetNode *node)
{
    DVASSERT(node->GetParent() == NULL);
    node->SetParent(this);
    styleSheets.push_back(SafeRetain(node));
}

void StyleSheetsNode::InsertAtIndex(DAVA::int32 index, StyleSheetNode *node)
{
    DVASSERT(node->GetParent() == NULL);
    node->SetParent(this);
    
    styleSheets.insert(styleSheets.begin() + index, SafeRetain(node));
}

void StyleSheetsNode::Remove(StyleSheetNode *node)
{
    auto it = find(styleSheets.begin(), styleSheets.end(), node);
    if (it != styleSheets.end())
    {
        DVASSERT(node->GetParent() == this);
        node->SetParent(NULL);
        
        styleSheets.erase(it);
        SafeRelease(node);
    }
    else
    {
        DVASSERT(false);
    }
}

int StyleSheetsNode::GetCount() const
{
    return (int) styleSheets.size();
}

StyleSheetNode *StyleSheetsNode::Get(int index) const
{
    return styleSheets[index];
}

void StyleSheetsNode::Accept(PackageVisitor *visitor)
{
    visitor->VisitStyleSheets(this);
}

String StyleSheetsNode::GetName() const
{
    return "Style Sheets";
}

bool StyleSheetsNode::IsInsertingStylesSupported() const
{
    return true;
}

bool StyleSheetsNode::CanInsertStyle(StyleSheetNode *node, DAVA::int32 pos) const
{
    return !IsReadOnly();
}
