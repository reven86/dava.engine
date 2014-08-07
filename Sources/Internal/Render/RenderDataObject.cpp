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
#include "Job/JobManager.h"
#include "Job/JobWaiter.h"

namespace DAVA 
{
    
RenderDataStream::RenderDataStream()
{
    formatMark = EVF_VERTEX;
    type = TYPE_FLOAT;
    size = 0;
    stride = 0;
    pointer = 0;
    
//#if defined (__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_MACOS__)
//    savedPointerData = NULL;
//#endif //#if defined (__DAVAENGINE_ANDROID__)
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
    
//#if defined (__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_MACOS__)
//    savedPointerData = pointer;
//#endif //#if defined (__DAVAENGINE_ANDROID__)

}
    
RenderDataObject::RenderDataObject()
	: RenderResource()
{
    resultVertexFormat = 0;
    vboBuffer = 0;
    vertexAttachmentActive = false;
    
//#if defined (__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_MACOS__)
//    savedVertexCount = 0;
//    savedIndices = NULL;
//    isLost = false;
//#endif //#if defined(__DAVAENGINE_ANDROID__)

    indexFormat = EIF_16;
    indices = 0;
    indexBuffer = 0;
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
    
	DestructorContainer * container = new DestructorContainer();
	container->vboBuffer = vboBuffer;
	container->indexBuffer = indexBuffer;
	ScopedPtr<Job> job = JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN, Message(this, &RenderDataObject::DeleteBuffersInternal, container), Job::NO_FLAGS);
}

void RenderDataObject::DeleteBuffersInternal(BaseObject * caller, void * param, void *callerData)
{
	DestructorContainer * container = (DestructorContainer*)param;
	DVASSERT(container);
#if defined(__DAVAENGINE_OPENGL__)
#if defined(__DAVAENGINE_OPENGL_ARB_VBO__)
	if (container->vboBuffer)
		RENDER_VERIFY(glDeleteBuffersARB(1, &container->vboBuffer));
	if (container->indexBuffer)
		RENDER_VERIFY(glDeleteBuffersARB(1, &container->indexBuffer));
#else 
	if (container->vboBuffer)
		RENDER_VERIFY(glDeleteBuffers(1, &container->vboBuffer));
	if (container->indexBuffer)
		RENDER_VERIFY(glDeleteBuffers(1, &container->indexBuffer));
#endif
#endif
	SafeDelete(container);
}

RenderDataStream * RenderDataObject::SetStream(eVertexFormat formatMark, eVertexDataType vertexType, int32 size, int32 stride, const void * pointer)
{
    DVASSERT(!vertexAttachmentActive);
    
    Map<eVertexFormat, RenderDataStream *>::iterator iter = streamMap.find(formatMark);
    RenderDataStream * stream = 0;
    if (iter == streamMap.end())
    {
        // New item - add it
        resultVertexFormat |= formatMark;
        stream = new RenderDataStream(); // todo optimize dynamic object cache
        
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
    
void RenderDataObject::BuildVertexBuffer(int32 vertexCount, bool synchronously)
{
    DVASSERT(!vertexAttachmentActive);
    
	ScopedPtr<Job> job = JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN, Message(this, &RenderDataObject::BuildVertexBufferInternal, reinterpret_cast<void*>(vertexCount)));
    
    if(synchronously)
    {
        JobInstanceWaiter waiter(job);
        waiter.Wait();
    }
}

void RenderDataObject::BuildVertexBufferInternal(BaseObject * caller, void * param, void *callerData)
{
	DVASSERT(Thread::IsMainThread());
#if defined (__DAVAENGINE_OPENGL__)
    
    uint32 size = streamArray.size();
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
        RENDER_VERIFY(glDeleteBuffers(1, &vboBuffer));
        vboBuffer = 0;
    }
    
    RENDER_VERIFY(glGenBuffers(1, &vboBuffer));
    RENDER_VERIFY(RenderManager::Instance()->HWglBindBuffer(GL_ARRAY_BUFFER, vboBuffer));

	int32 vertexCount = (int32)((int64)param);
    RENDER_VERIFY(glBufferData(GL_ARRAY_BUFFER, vertexCount * stride, streamArray[0]->pointer, GL_STATIC_DRAW));

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
    
//#if defined (__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_MACOS__)
//    savedIndices = indices;
//#endif //#if defined (__DAVAENGINE_ANDROID__)
}

void RenderDataObject::BuildIndexBuffer(bool synchronously)
{
	ScopedPtr<Job> job = JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN, Message(this, &RenderDataObject::BuildIndexBufferInternal));
    
    if(synchronously)
    {
        JobInstanceWaiter waiter(job);
        waiter.Wait();
    }
}

void RenderDataObject::BuildIndexBufferInternal(BaseObject * caller, void * param, void *callerData)
{
	DVASSERT(Thread::IsMainThread());
#if defined (__DAVAENGINE_OPENGL__)
    
    
    
#if defined(__DAVAENGINE_OPENGL_ARB_VBO__)
    if (indexBuffer)
    {
        RENDER_VERIFY(glDeleteBuffersARB(1, &indexBuffer));
        indexBuffer = 0;
    }
    
    RENDER_VERIFY(glGenBuffersARB(1, &indexBuffer));
    RENDER_VERIFY(glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indexBuffer));
    RENDER_VERIFY(glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indexCount * INDEX_FORMAT_SIZE[indexFormat], indices, GL_STATIC_DRAW_ARB));
#else
    if (indexBuffer)
    {
        RENDER_VERIFY(glDeleteBuffers(1, &indexBuffer));
        indexBuffer = 0;
    }
    RENDER_VERIFY(glGenBuffers(1, &indexBuffer));
//    Logger::FrameworkDebug("glGenBuffers index: %d", indexBuffer);
    RENDER_VERIFY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer));
    RENDER_VERIFY(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * INDEX_FORMAT_SIZE[indexFormat], indices, GL_STATIC_DRAW));
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

//#if defined (__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_MACOS__)
//void RenderDataObject::SaveToSystemMemory()
//{
//}
//
//void RenderDataObject::Lost()
//{
////    Logger::FrameworkDebug("[RenderDataObject::Lost]");
//    //    vboBuffer = 0;
//#if defined(__DAVAENGINE_OPENGL__)
//#if defined(__DAVAENGINE_OPENGL_ARB_VBO__)
//    if (vboBuffer)
//        RENDER_VERIFY(glDeleteBuffersARB(1, &vboBuffer));
//    if (indexBuffer)
//        RENDER_VERIFY(glDeleteBuffersARB(1, &indexBuffer));
//#else 
//    if (vboBuffer)
//        RENDER_VERIFY(glDeleteBuffers(1, &vboBuffer));
//    if (indexBuffer)
//        RENDER_VERIFY(glDeleteBuffers(1, &indexBuffer));
//#endif
//    
//    vboBuffer = 0;
//    indexBuffer = 0;
//#endif
//
//    isLost = true;
//
//}
//
//void RenderDataObject::Invalidate()
//{
////    Logger::FrameworkDebug("[RenderDataObject::Invalidate]");
//
//    if(isLost)
//    {
//        isLost = false;
//        
//        if(savedVertexCount)
//        {
//            uint32 size = streamArray.size();
//            for (uint32 k = 0; k < size; ++k)
//            {
//                streamArray[k]->pointer = streamArray[k]->savedPointerData;
//            }
//            
//            BuildVertexBuffer(savedVertexCount);
//        }
//        
//        if(savedIndices)
//        {
//            indices = savedIndices;
//            BuildIndexBuffer();
//        }
//    }
//    else
//    {
//        Logger::Warning("[RenderDataObject::Invalidate] not lost !!!");
//    }
//}
//
//#endif //#if defined(__DAVAENGINE_ANDROID__)
}