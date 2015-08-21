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


#include "Render/RenderBase.h"
#include "Render/RenderDataObject.h"
#include "Render/RenderManager.h"

namespace DAVA 
{
    
RenderDataStream::RenderDataStream()
{
    formatMark = EVF_VERTEX;
    type = TYPE_FLOAT;
    size = 0;
    stride = 0;
    pointer = 0;
    
#if defined (__DAVAENGINE_ANDROID__)
    savedPointerData = NULL;
#endif //#if defined (__DAVAENGINE_ANDROID__)
}

RenderDataStream::~RenderDataStream()
{
}

void RenderDataStream::Set(eVertexDataType _type, int32 _size, int32 _stride, const void * _pointer)
{
    type = _type;
    size = _size;
    stride = _stride;
    pointer = _pointer;
    
#if defined (__DAVAENGINE_ANDROID__)
    savedPointerData = pointer;
#endif //#if defined (__DAVAENGINE_ANDROID__)

}
    
RenderDataObject::RenderDataObject()
    : RenderResource()
{
    resultVertexFormat = 0;
    vboBuffer = 0;
    vertexAttachmentActive = false;
    
    indexFormat = EIF_16;
    indices = 0;
    indexBuffer = 0;

#if defined (__DAVAENGINE_ANDROID__)
    forceVerticesCount = 0;
    forceIndicesCount = 0;
    savedVertexCount = 0;
    savedIndices = NULL;
    savedVertexBufferType = BDT_STATIC_DRAW;
    savedIndexBufferType = BDT_STATIC_DRAW;
    isLost = false;
    buildIndexBuffer = false;
#endif //#if defined(__DAVAENGINE_ANDROID__)

    indexCount = 0;
}

RenderDataObject::~RenderDataObject()
{
    if(vertexAttachmentActive)
    {
        DetachVertices();
    }
    else
    {
        uint32 size = (uint32)streamArray.size();
        for (uint32 k = 0; k < size; ++k)
        {
            SafeRelease(streamArray[k]);
        }
    }
    //streamArray.clear();
    //streamMap.clear();
    
    Function<void()> fn = Bind(&RenderDataObject::DeleteBuffersInternal, this, vboBuffer, indexBuffer);
	JobManager::Instance()->CreateMainJob(fn);
}

void RenderDataObject::DeleteBuffersInternal(uint32 vboBuffer, uint32 indexBuffer)
{
#if defined(__DAVAENGINE_OPENGL__)
    if(vboBuffer)
    {
        RENDER_VERIFY(RenderManager::Instance()->HWglDeleteBuffers(1, &vboBuffer));
        DAVA_MEMORY_PROFILER_GPU_DEALLOC(vboBuffer, ALLOC_GPU_RDO_VERTEX);
    }
    if(indexBuffer)
    {
        RENDER_VERIFY(RenderManager::Instance()->HWglDeleteBuffers(1, &indexBuffer));
        DAVA_MEMORY_PROFILER_GPU_DEALLOC(indexBuffer, ALLOC_GPU_RDO_INDEX);
    }
#endif
}

RenderDataStream * RenderDataObject::SetStream(eVertexFormat formatMark, eVertexDataType vertexType, int32 size, int32 stride, const void * pointer)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(!vertexAttachmentActive);
    
    Map<eVertexFormat, RenderDataStream *>::iterator iter = streamMap.find(formatMark);
    RenderDataStream * stream = 0;
    if (iter == streamMap.end())
    {
        // New item - add it
        resultVertexFormat |= formatMark;
        stream = new RenderDataStream; // todo optimize dynamic object cache
        
        streamMap[formatMark] = stream;
        streamArray.push_back(stream);
    }else
    {
        stream = iter->second;
    }
    
    stream->formatMark = formatMark;
    stream->Set(vertexType, size, stride, pointer);

    return stream;
}

void RenderDataObject::RemoveStream(eVertexFormat formatMark)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(!vertexAttachmentActive);
    
	Map<eVertexFormat, RenderDataStream*>::iterator it = streamMap.find(formatMark);
	if (it!=streamMap.end())
	{
		RenderDataStream *stream = (*it).second;
		streamMap.erase(it);
		resultVertexFormat &= ~formatMark;	
		/*remove from array*/
		for (Vector<RenderDataStream *>::iterator vec_it = streamArray.begin(), e = streamArray.end(); vec_it!=e; ++vec_it)
		{
			if ((*vec_it) == stream)
			{
				streamArray.erase(vec_it);
				break;
			}
		}
        
        SafeRelease(stream);
	}
}

uint32 RenderDataObject::GetResultFormat() const
{
    return resultVertexFormat;
}
    
void RenderDataObject::BuildVertexBuffer(int32 vertexCount, eBufferDrawType type, bool synchronously)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	DVASSERT(!vertexAttachmentActive);

    Function<void()> fn = Bind(MakeFunction(MakeSharedObject(this), &RenderDataObject::BuildVertexBufferInternal), vertexCount, type);
	if(synchronously)
    {
        uint32 jobId = JobManager::Instance()->CreateMainJob(fn);
		JobManager::Instance()->WaitMainJobID(jobId);
	}
    else
    {
        JobManager::Instance()->CreateMainJob(fn);
    }
}

void RenderDataObject::BuildVertexBufferInternal(int32 vertexCount, eBufferDrawType type)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	DVASSERT(Thread::IsMainThread());
#if defined (__DAVAENGINE_OPENGL__)
    
    uint32 size = (uint32)streamArray.size();
    if (size == 0)return;

    for (uint32 k = 1; k < size; ++k)
    {
        DVASSERT(streamArray[k]->stride == streamArray[k - 1]->stride);
    }
    
    uint32 format = 0;
    for (uint32 k = 0; k < size; ++k)
    {
        format |= streamArray[k]->formatMark;
    }
    
    int32 stride = streamArray[0]->stride;
    
    if (vboBuffer)
    {
        RENDER_VERIFY(RenderManager::Instance()->HWglDeleteBuffers(1, &vboBuffer));
        vboBuffer = 0;
    }
    
    RENDER_VERIFY(glGenBuffers(1, &vboBuffer));
    RENDER_VERIFY(RenderManager::Instance()->HWglBindBuffer(GL_ARRAY_BUFFER, vboBuffer));

#if defined (__DAVAENGINE_ANDROID__)
    savedVertexCount = vertexCount;
    savedVertexBufferType = type;
#endif //#if defined (__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(glBufferData(GL_ARRAY_BUFFER, vertexCount * stride, streamArray[0]->pointer, BUFFERDRAWTYPE_MAP[type]));
    DAVA_MEMORY_PROFILER_GPU_ALLOC(vboBuffer, static_cast<size_t>(vertexCount * stride), ALLOC_GPU_RDO_VERTEX);

    streamArray[0]->pointer = 0;
    for (uint32 k = 1; k < size; ++k)
    {
        streamArray[k]->pointer = (uint8*)streamArray[k - 1]->pointer + GetVertexSize(streamArray[k - 1]->formatMark);
    }
    
    RENDER_VERIFY(RenderManager::Instance()->HWglBindBuffer(GL_ARRAY_BUFFER, 0));

#endif // #if defined (__DAVAENGINE_OPENGL__)
} 
    
void RenderDataObject::SetIndices(eIndexFormat _format, uint8 * _indices, int32 _count)
{
    indexFormat = _format;
    indexCount = _count;
    indices = _indices;
    
#if defined (__DAVAENGINE_ANDROID__)
    savedIndices = indices;
#endif //#if defined (__DAVAENGINE_ANDROID__)
}

void RenderDataObject::BuildIndexBuffer(eBufferDrawType type, bool synchronously)
{
	Function<void ()> fn = Bind(MakeFunction(MakeSharedObject(this), &RenderDataObject::BuildIndexBufferInternal), type);
    uint32 jobId = JobManager::Instance()->CreateMainJob(fn);

    if(synchronously)
    {
        JobManager::Instance()->WaitMainJobID(jobId);
    }
}

void RenderDataObject::BuildIndexBufferInternal(eBufferDrawType type)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(Thread::IsMainThread());
#if defined (__DAVAENGINE_OPENGL__)
    
    
#if defined (__DAVAENGINE_ANDROID__)
    buildIndexBuffer = true;
    savedIndexBufferType = type;
#endif
    
#if defined(__DAVAENGINE_OPENGL_ARB_VBO__)
    if (indexBuffer)
    {
        RENDER_VERIFY(glDeleteBuffersARB(1, &indexBuffer));
        indexBuffer = 0;
    }
    
    RENDER_VERIFY(glGenBuffersARB(1, &indexBuffer));
    RENDER_VERIFY(glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indexBuffer));
    RENDER_VERIFY(glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indexCount * INDEX_FORMAT_SIZE[indexFormat], indices, GL_STATIC_DRAW_ARB));
    MEMORY_PROFILER_GPU_ALLOC(indexBuffer, static_cast<size_t>(indexCount * INDEX_FORMAT_SIZE[indexFormat]), ALLOC_GPU_RDO_INDEX);
#else
    if (indexBuffer)
    {
        RENDER_VERIFY(RenderManager::Instance()->HWglDeleteBuffers(1, &indexBuffer));
        indexBuffer = 0;
    }
    RENDER_VERIFY(glGenBuffers(1, &indexBuffer));
    RENDER_VERIFY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer));
    RENDER_VERIFY(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * INDEX_FORMAT_SIZE[indexFormat], indices, GL_STATIC_DRAW));
    DAVA_MEMORY_PROFILER_GPU_ALLOC(indexBuffer, static_cast<size_t>(indexCount * INDEX_FORMAT_SIZE[indexFormat]), ALLOC_GPU_RDO_INDEX);
#endif
    
#if defined(__DAVAENGINE_OPENGL_ARB_VBO__)
    RENDER_VERIFY(glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0));
#else
    RENDER_VERIFY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
#endif
    
#endif // #if defined (__DAVAENGINE_OPENGL__)
}

void RenderDataObject::AttachVertices(RenderDataObject* vertexSource)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(vertexSource);
    DVASSERT(0 == vboBuffer);
    DVASSERT(streamArray.size() == 0);
    DVASSERT(streamMap.size() == 0);
    
    vboBuffer = vertexSource->vboBuffer;
    for(Map<eVertexFormat, RenderDataStream *>::iterator it = vertexSource->streamMap.begin();
        it != vertexSource->streamMap.end();
        ++it)
    {
        streamMap[it->first] = it->second;
    }
    
    for(Vector<RenderDataStream *>::iterator it = vertexSource->streamArray.begin();
        it != vertexSource->streamArray.end();
        ++it)
    {
        streamArray.push_back(SafeRetain(*it));
    }
    
    vertexAttachmentActive = true;
}
    
void RenderDataObject::DetachVertices()
{
    vboBuffer = 0;
    
    for(Vector<RenderDataStream *>::iterator it = streamArray.begin();
        it != streamArray.end();
        ++it)
    {
        SafeRelease(*it);
    }

    streamArray.clear();
    streamMap.clear();
    
    vertexAttachmentActive = false;
}

void RenderDataObject::UpdateVertexBuffer(int32 offset, int32 vertexCount, bool synchronously)
{
    uint32 jobId = JobManager::Instance()->CreateMainJob(Bind(&RenderDataObject::UpdateVertexBufferInternal, this, offset, vertexCount));

    if (synchronously)
    {
        JobManager::Instance()->WaitMainJobID(jobId);
    }
}

void RenderDataObject::UpdateVertexBufferInternal(int32 offset, int32 vertexCount)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(Thread::IsMainThread());

#if defined (__DAVAENGINE_OPENGL__)
    DVASSERT(vboBuffer);

    uint32 size = (uint32)streamArray.size();
    if (size == 0)return;

    for (uint32 k = 1; k < size; ++k)
    {
        DVASSERT(streamArray[k]->stride == streamArray[k - 1]->stride);
    }

    uint32 format = 0;
    for (uint32 k = 0; k < size; ++k)
    {
        format |= streamArray[k]->formatMark;
    }
    int32 stride = streamArray[0]->stride;

    RENDER_VERIFY(RenderManager::Instance()->HWglBindBuffer(GL_ARRAY_BUFFER, vboBuffer));
    RENDER_VERIFY(glBufferSubData(GL_ARRAY_BUFFER, offset * stride, vertexCount * stride, streamArray[0]->pointer));
    RENDER_VERIFY(RenderManager::Instance()->HWglBindBuffer(GL_ARRAY_BUFFER, 0));
#endif // #if defined (__DAVAENGINE_OPENGL__)

    streamArray[0]->pointer = 0;
    for (uint32 k = 1; k < size; ++k)
    {
        streamArray[k]->pointer = (uint8*)streamArray[k - 1]->pointer + GetVertexSize(streamArray[k - 1]->formatMark);
    }
}

void RenderDataObject::UpdateIndexBuffer(int32 offset, bool synchronously)
{
    uint32 jobId = JobManager::Instance()->CreateMainJob(Bind(&RenderDataObject::UpdateIndexBufferInternal, this, offset));

    if (synchronously)
    {
        JobManager::Instance()->WaitMainJobID(jobId);
    }
}

void RenderDataObject::UpdateIndexBufferInternal(int32 offset)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(Thread::IsMainThread());
#if defined (__DAVAENGINE_OPENGL__)
    DVASSERT(indexBuffer);

    RENDER_VERIFY(RenderManager::Instance()->HWglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer));
    RENDER_VERIFY(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset * INDEX_FORMAT_SIZE[indexFormat], indexCount * INDEX_FORMAT_SIZE[indexFormat], indices));
    RENDER_VERIFY(RenderManager::Instance()->HWglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
#endif // #if defined (__DAVAENGINE_OPENGL__)
}

#if defined (__DAVAENGINE_ANDROID__)
void RenderDataObject::SaveToSystemMemory()
{
}

void RenderDataObject::Lost()
{
    vboBuffer = 0;
    indexBuffer = 0;
    isLost = true;
}

void RenderDataObject::Invalidate()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

//    Logger::FrameworkDebug("[RenderDataObject::Invalidate]");

    if(isLost)
    {
        isLost = false;

        if(!HasVertexAttachment() && (savedVertexCount || forceVerticesCount))
        {
            uint32 size = streamArray.size();
            for (uint32 k = 0; k < size; ++k)
            {
            	streamArray[k]->pointer = streamArray[k]->savedPointerData;
            }
            BuildVertexBuffer(forceVerticesCount > 0 ? forceVerticesCount : savedVertexCount, savedVertexBufferType);
        }

        if(buildIndexBuffer && (savedIndices || forceIndicesCount))
        {
        	if(forceIndicesCount)
        	{
        		indexCount = forceIndicesCount;
        	}
            indices = savedIndices;
            BuildIndexBuffer(savedIndexBufferType);
        }
    }
    else
    {
        Logger::Warning("[RenderDataObject::Invalidate] not lost !!!");
    }
}

#endif //#if defined(__DAVAENGINE_ANDROID__)
}
