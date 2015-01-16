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



#include "CloneLastBatchCommand.h"

CloneLastBatchCommand::CloneLastBatchCommand(DAVA::RenderObject *ro)
	: Command2(CMDID_CLONE_LAST_BATCH, "Clone Last Batch")
{
    DVASSERT(ro);
    renderObject = SafeRetain(ro);

    // find proper LOD and switch indexes and last batches
    maxLodIndexes[0] = maxLodIndexes[1] = -1;
    DAVA::RenderBatch *lastBatches[2] = {NULL, NULL};

    const DAVA::uint32 count = renderObject->GetRenderBatchCount(); 
    for(DAVA::uint32 i = 0; i < count; ++i) 
    {
        DAVA::int32 lod, sw;
        DAVA::RenderBatch *batch = renderObject->GetRenderBatch(i, lod, sw);

        DVASSERT(sw < 2);
        if((lod > maxLodIndexes[sw]) && (sw >= 0 && sw < 2))
        {
            maxLodIndexes[sw] = lod;
            lastBatches[sw] = batch;
        }
    }
    DVASSERT(maxLodIndexes[0] != maxLodIndexes[1]);

    //detect switch index to clone batches
    requestedSwitchIndex = 0;
    DAVA::int32 maxIndex = maxLodIndexes[1];
    if(maxLodIndexes[0] > maxLodIndexes[1])
    {
        requestedSwitchIndex = 1;
        maxIndex = maxLodIndexes[0];
    }

    //clone batches
    for(DAVA::int32 i = maxLodIndexes[requestedSwitchIndex]; i < maxIndex; ++i)
    {
        newBatches.push_back(lastBatches[requestedSwitchIndex]->Clone());
    }
}

CloneLastBatchCommand::~CloneLastBatchCommand()
{
    SafeRelease(renderObject);
    
    for_each(newBatches.begin(), newBatches.end(), DAVA::SafeRelease< DAVA::RenderBatch>);
    newBatches.clear();
}

void CloneLastBatchCommand::Redo()
{
    const DAVA::uint32 count = (DAVA::uint32)newBatches.size();
    DAVA::int32 lodIndex = maxLodIndexes[requestedSwitchIndex] + 1;
    for(DAVA::uint32 i = 0; i < count; ++i)
    {
        renderObject->AddRenderBatch(newBatches[i], lodIndex, requestedSwitchIndex);
        ++lodIndex;
    }
}

void CloneLastBatchCommand::Undo()
{
    const DAVA::int32 count = (DAVA::int32)renderObject->GetRenderBatchCount();
    for (DAVA::int32 i = count - 1; i >= 0; --i)
    {
        DAVA::int32 lod, sw;
        renderObject->GetRenderBatch(i, lod, sw);

        if((sw == requestedSwitchIndex) && (lod > maxLodIndexes[requestedSwitchIndex]))
        {
            renderObject->RemoveRenderBatch(i);
        }
    }
}


DAVA::Entity * CloneLastBatchCommand::GetEntity() const
{
    return NULL;
}


