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

#ifndef __DAVAENGINE_DDS_HANDLERS_H__
#define __DAVAENGINE_DDS_HANDLERS_H__

#include "Base/BaseTypes.h"
#include "Render/Image/ImageFormatInterface.h"

namespace DAVA
{
class Image;

struct DDSReader;
struct DDSWriter;

struct DDSReader
{
    virtual ~DDSReader() = default;
    virtual ImageInfo GetImageInfo() = 0;
    virtual bool GetImages(Vector<Image*>& images, const ImageSystem::LoadingParams& loadingParams) = 0;
    virtual bool GetCRC(uint32& crc) const = 0;
    virtual bool AddCRC() = 0;

    static std::unique_ptr<DDSReader> CreateReader(const ScopedPtr<File>& file);
};

struct DDSWriter
{
    virtual ~DDSWriter() = default;
    virtual bool Write(const Vector<Vector<Image*>>& images, PixelFormat dstFormat) = 0;

    static std::unique_ptr<DDSWriter> CreateWriter(const ScopedPtr<File>& file);
};

} // namespace DAVA

#endif // __DAVAENGINE_DDS_HANDLERS_H__
