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


#ifndef GRAPHBUILDERADAPTER_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define GRAPHBUILDERADAPTER_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) || (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || (__GNUC__ >= 4)) // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <cstdlib>
#include <map>
#include <stack>
#include "yaml-cpp/eventhandler.h"
#include "yaml-cpp/contrib/anchordict.h"
#include "yaml-cpp/contrib/graphbuilder.h"

namespace YAML
{
  class GraphBuilderAdapter : public EventHandler
  {
  public:
    GraphBuilderAdapter(GraphBuilderInterface& builder)
    : m_builder(builder), m_pRootNode(NULL), m_pKeyNode(NULL)
    {
    }
    
    virtual void OnDocumentStart(const Mark& mark) {(void)mark;}
    virtual void OnDocumentEnd() {}
    
    virtual void OnNull(const Mark& mark, anchor_t anchor);
    virtual void OnAlias(const Mark& mark, anchor_t anchor);
    virtual void OnScalar(const Mark& mark, const std::string& tag, anchor_t anchor, const std::string& value);
    
    virtual void OnSequenceStart(const Mark& mark, const std::string& tag, anchor_t anchor);
    virtual void OnSequenceEnd();
    
    virtual void OnMapStart(const Mark& mark, const std::string& tag, anchor_t anchor);
    virtual void OnMapEnd();
    
    void *RootNode() const {return m_pRootNode;}
  
  private:
    struct ContainerFrame
    {
      ContainerFrame(void *pSequence)
      : pContainer(pSequence), pPrevKeyNode(&sequenceMarker)
      {}
      ContainerFrame(void *pMap, void* pPrevKeyNode)
      : pContainer(pMap), pPrevKeyNode(pPrevKeyNode)
      {}
      
      void *pContainer;
      void *pPrevKeyNode;
      
      bool isMap() const {return pPrevKeyNode != &sequenceMarker;}
    
    private:
      static int sequenceMarker;
    };
    typedef std::stack<ContainerFrame> ContainerStack;
    typedef AnchorDict<void*> AnchorMap;
    
    GraphBuilderInterface& m_builder;
    ContainerStack m_containers;
    AnchorMap m_anchors;
    void *m_pRootNode;
    void *m_pKeyNode;
    
    void *GetCurrentParent() const;
    void RegisterAnchor(anchor_t anchor, void *pNode);
    void DispositionNode(void *pNode);
  };
}

#endif // GRAPHBUILDERADAPTER_H_62B23520_7C8E_11DE_8A39_0800200C9A66
