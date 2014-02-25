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


#ifndef __DAVAENGINE_RENDERDATAOBJECT_H__
#define __DAVAENGINE_RENDERDATAOBJECT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Render/RenderBase.h"
#include "Render/RenderResource.h"

namespace DAVA
{
    
class RenderDataObject;
class RenderManager;
class RenderManagerGL20;
class NMaterial;
    
class RenderDataStream : public BaseObject
{
protected:
    virtual ~RenderDataStream();
public:
    RenderDataStream();
    
    void Set(eVertexDataType type, int32 size, int32 stride, const void * pointer);
    
    eVertexFormat formatMark;
    eVertexDataType type;
    int32 size;
    int32 stride;
    const void * pointer;
#if defined (__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_MACOS__)
    void * savedPointerData;
#endif //#if defined (__DAVAENGINE_ANDROID__)
};

class RenderDataObject : public RenderResource //BaseObject
{
protected:
    virtual ~RenderDataObject();
public:
    RenderDataObject();
    
    RenderDataStream * SetStream(eVertexFormat formatMark, eVertexDataType vertexType, int32 size, int32 stride, const void * pointer);
	void RemoveStream(eVertexFormat formatMark);
    uint32 GetResultFormat() const;

    uint32 GetStreamCount() const { return (uint32)streamArray.size(); };
    RenderDataStream * GetStream(uint32 index) { return streamArray[index]; }
    /*
        We think that render data object can pack data automatically. 
        Interleaved data is the fastest way to submit data to any modern hw, so renderdataobject support buffers only 
        for interleaved data. This means we can have only 1 buffer for 1 RenderDataObject
    */
    void BuildVertexBuffer(int32 vertexCount); // pack data to VBOs and allow to use VBOs instead of SetStreams
	void BuildVertexBufferInternal(BaseObject * caller, void * param, void *callerData);
	void DeleteBuffersInternal(BaseObject * caller, void * param, void *callerData);
    
//#if defined (__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_MACOS__)
//	virtual void SaveToSystemMemory();
//	virtual void Lost();
//	virtual void Invalidate();
//	int32 savedVertexCount;
//    bool isLost;
//#endif //#if defined(__DAVAENGINE_ANDROID__)
    
    void SetIndices(eIndexFormat format, uint8 * indices, int32 count);
    void BuildIndexBuffer();
	void BuildIndexBufferInternal(BaseObject * caller, void * param, void *callerData);
    uint32 GetIndexBufferID() const { return indexBuffer; };

    
private:
    Map<eVertexFormat, RenderDataStream *> streamMap;
    Vector<RenderDataStream *> streamArray;
    uint32 resultVertexFormat;

    // TODO: add DX support
    uint32 vboBuffer; 
    
    eIndexFormat indexFormat;
    uint8 * indices;
//#if defined (__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_MACOS__)
//    uint8 * savedIndices;
//#endif //#if defined(__DAVAENGINE_ANDROID__)
    uint32 indexBuffer;
    int32 indexCount;
    
    friend class RenderManager;
    friend class RenderManagerGL20;
	friend class NMaterial;

	struct DestructorContainer
	{
		uint32 vboBuffer;
		uint32 indexBuffer;
	};
};
    
};

#endif // __DAVAENGINE_RENDERSTATEBLOCK_H__
