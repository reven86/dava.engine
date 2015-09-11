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


#ifndef __QUICKED_SELECTION_TRACKER_H__
#define __QUICKED_SELECTION_TRACKER_H__

#include "Base/BaseTypes.h"

class PackageBaseNode;
class ControlNode;

using SelectedNodes = DAVA::Set < PackageBaseNode* > ;
using SelectedControls = DAVA::Set<ControlNode*>;

struct SelectionTracker
{
    template <typename ContainerIn, typename ContainerOut>
    void GetNotExistedItems(const ContainerIn& in, ContainerOut& out)
    {
        std::set_difference(in.begin(), in.end(), selectedNodes.begin(), selectedNodes.end(), std::inserter(out, out.end()));
    }

    template <typename ContainerIn, typename ContainerOut>
    void GetOnlyExistedItems(const ContainerIn& in, ContainerOut& out)
    {
        std::set_intersection(selectedNodes.begin(), selectedNodes.end(), in.begin(), in.end(), std::inserter(out, out.end()));
    }

    void MergeSelection(const SelectedNodes& selected, const SelectedNodes& deselected)
    {
        for (const auto &item : deselected)
        {
            selectedNodes.erase(item);
        }
        for (const auto &item : selected)
        {
            selectedNodes.insert(item);
        }
    }

    template <typename SetT, typename SetU>
    static SetT GetSetTFromSetU(const SetU& in)
    {
        using T = typename std::remove_reference<SetT>::type::value_type;
        SetT retVal;
        for (auto& u : in)
        {
            auto tmp = dynamic_cast<T>(u);
            if (nullptr != tmp)
            {
                retVal.insert(tmp);
            }
        }
        return retVal;
    }

    template <typename SetT>
    SetT GetSetTFromNodes() const
    {
        return GetSetTFromSetU<SetT>(selectedNodes);
    }
    SelectedNodes selectedNodes;
};

#endif // __QUICKED_SELECTION_TRACKER_H__
