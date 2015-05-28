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


#include "graphbuilderadapter.h"

namespace YAML
{
  int GraphBuilderAdapter::ContainerFrame::sequenceMarker;
  
  void GraphBuilderAdapter::OnNull(const Mark& mark, anchor_t anchor)
  {
    void *pParent = GetCurrentParent();
    void *pNode = m_builder.NewNull(mark, pParent);
    RegisterAnchor(anchor, pNode);
    
    DispositionNode(pNode);
  }
  
  void GraphBuilderAdapter::OnAlias(const Mark& mark, anchor_t anchor)
  {
    void *pReffedNode = m_anchors.Get(anchor);
    DispositionNode(m_builder.AnchorReference(mark, pReffedNode));
  }
  
  void GraphBuilderAdapter::OnScalar(const Mark& mark, const std::string& tag, anchor_t anchor, const std::string& value)
  {
    void *pParent = GetCurrentParent();
    void *pNode = m_builder.NewScalar(mark, tag, pParent, value);
    RegisterAnchor(anchor, pNode);
    
    DispositionNode(pNode);
  }
  
  void GraphBuilderAdapter::OnSequenceStart(const Mark& mark, const std::string& tag, anchor_t anchor)
  {
    void *pNode = m_builder.NewSequence(mark, tag, GetCurrentParent());
    m_containers.push(ContainerFrame(pNode));
    RegisterAnchor(anchor, pNode);
  }
  
  void GraphBuilderAdapter::OnSequenceEnd()
  {
    void *pSequence = m_containers.top().pContainer;
    m_containers.pop();
    
    DispositionNode(pSequence);
  }
  
  void GraphBuilderAdapter::OnMapStart(const Mark& mark, const std::string& tag, anchor_t anchor)
  {
    void *pNode = m_builder.NewMap(mark, tag, GetCurrentParent());
    m_containers.push(ContainerFrame(pNode, m_pKeyNode));
    m_pKeyNode = NULL;
    RegisterAnchor(anchor, pNode);
  }
  
  void GraphBuilderAdapter::OnMapEnd()
  {
    void *pMap = m_containers.top().pContainer;
    m_pKeyNode = m_containers.top().pPrevKeyNode;
    m_containers.pop();
    DispositionNode(pMap);
  }
  
  void *GraphBuilderAdapter::GetCurrentParent() const
  {
    if (m_containers.empty()) {
      return NULL;
    }
    return m_containers.top().pContainer;
  }
  
  void GraphBuilderAdapter::RegisterAnchor(anchor_t anchor, void *pNode)
  {
    if (anchor) {
      m_anchors.Register(anchor, pNode);
    }
  }
  
  void GraphBuilderAdapter::DispositionNode(void *pNode)
  {
    if (m_containers.empty()) {
      m_pRootNode = pNode;
      return;
    }
    
    void *pContainer = m_containers.top().pContainer;
    if (m_containers.top().isMap()) {
      if (m_pKeyNode) {
        m_builder.AssignInMap(pContainer, m_pKeyNode, pNode);
        m_pKeyNode = NULL;
      } else {
        m_pKeyNode = pNode;
      }
    } else {
      m_builder.AppendToSequence(pContainer, pNode);
    }
  }
}
