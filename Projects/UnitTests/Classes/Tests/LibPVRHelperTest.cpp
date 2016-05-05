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

#include "DAVAEngine.h"
#include "Render/Image/LibPVRHelper.h"
#include "UnitTests/UnitTests.h"
#include "Infrastructure/TextureUtils.h"
#include "Utils/CRC32.h"

using namespace DAVA;

namespace LibPVRHelperTestLocal
{
void PrepareWorkingFolder(const FilePath& folder)
{
    FileSystem::Instance()->DeleteDirectory(folder, true);
    FileSystem::Instance()->CreateDirectory(folder, true);
}

void ReleaseImages(Vector<Image*>& imageSet)
{
    for (Image* image : imageSet)
    {
        SafeRelease(image);
    }
    imageSet.clear();
}

struct TestData
{
    FilePath path;
    uint32 width;
    uint32 height;
    uint32 mipmapsCount;
    int32 fromMipmap;
    PixelFormat format;
};
}

DAVA_TESTCLASS (LibPVRHelperTest)
{
    DAVA_TEST (NoCubeMapTest)
    {
        const FilePath outFolderPathname = "~doc:/TestData/LibPVRHelperTest/";
        LibPVRHelperTestLocal::PrepareWorkingFolder(outFolderPathname);

        static Vector<LibPVRHelperTestLocal::TestData> testData =
        {
          { "~res:/TestData/LibPVRHelperTest/a8.pvr", 32, 32, 6, 0, PixelFormat::FORMAT_A8 },
          { "~res:/TestData/LibPVRHelperTest/etc1.pvr", 32, 32, 6, 0, PixelFormat::FORMAT_ETC1 },
          { "~res:/TestData/LibPVRHelperTest/pvr2.pvr", 32, 32, 6, 0, PixelFormat::FORMAT_PVR2 },
          { "~res:/TestData/LibPVRHelperTest/pvr4.pvr", 32, 32, 6, 0, PixelFormat::FORMAT_PVR4 },
          { "~res:/TestData/LibPVRHelperTest/rgb565.pvr", 32, 32, 6, 0, PixelFormat::FORMAT_RGB565 },
          { "~res:/TestData/LibPVRHelperTest/rgb888.pvr", 32, 32, 6, 0, PixelFormat::FORMAT_RGB888 },
          { "~res:/TestData/LibPVRHelperTest/rgba4444.pvr", 32, 32, 6, 0, PixelFormat::FORMAT_RGBA4444 },
          { "~res:/TestData/LibPVRHelperTest/rgba5551.pvr", 32, 32, 6, 0, PixelFormat::FORMAT_RGBA5551 },
          { "~res:/TestData/LibPVRHelperTest/rgba8888.pvr", 32, 32, 6, 0, PixelFormat::FORMAT_RGBA8888 },
          { "~res:/TestData/LibPVRHelperTest/rgba16161616.pvr", 32, 32, 6, 0, PixelFormat::FORMAT_RGBA16161616 },
          { "~res:/TestData/LibPVRHelperTest/rgba32323232.pvr", 32, 32, 6, 0, PixelFormat::FORMAT_RGBA32323232 },

          { "~res:/TestData/LibPVRHelperTest/a8.pvr", 16, 16, 5, 1, PixelFormat::FORMAT_A8 },
          { "~res:/TestData/LibPVRHelperTest/etc1.pvr", 8, 8, 4, 2, PixelFormat::FORMAT_ETC1 },
          { "~res:/TestData/LibPVRHelperTest/pvr2.pvr", 4, 4, 3, 3, PixelFormat::FORMAT_PVR2 },
          { "~res:/TestData/LibPVRHelperTest/pvr4.pvr", 2, 2, 2, 4, PixelFormat::FORMAT_PVR4 },
          { "~res:/TestData/LibPVRHelperTest/rgb565.pvr", 1, 1, 1, 5, PixelFormat::FORMAT_RGB565 },

          { "~res:/TestData/LibPVRHelperTest/pvr4_zeroMip.pvr", 32, 32, 1, 0, PixelFormat::FORMAT_PVR4 },
        };

        for (const LibPVRHelperTestLocal::TestData& td : testData)
        {
            Vector<Image*> imageSet;

            { // Load
                ScopedPtr<File> infile(File::Create(td.path, File::OPEN | File::READ));
                eErrorCode loadCode = ImageSystem::Instance()->LoadWithoutDecompession(infile, imageSet, td.fromMipmap, 0);
                TEST_VERIFY(eErrorCode::SUCCESS == loadCode);

                bool loaded = imageSet.size() == td.mipmapsCount;
                TEST_VERIFY(loaded);
                if (loaded)
                {
                    for (uint32 mip = 0; mip < td.mipmapsCount; ++mip)
                    {
                        Image* image = imageSet[mip];

                        TEST_VERIFY(image->mipmapLevel == mip);
                        TEST_VERIFY(image->cubeFaceID == Texture::INVALID_CUBEMAP_FACE);

                        TEST_VERIFY(image->width == (td.width >> mip));
                        TEST_VERIFY(image->height == (td.height >> mip));
                        TEST_VERIFY(image->format == td.format);
                    }
                }
            }

            { //Save
                FilePath savePath(td.path);
                savePath.ReplaceDirectory(outFolderPathname);

                eErrorCode saveCode = ImageSystem::Instance()->Save(savePath, imageSet, td.format);
                TEST_VERIFY(eErrorCode::SUCCESS == saveCode);

                Vector<Image*> reLoadedImageSet;
                { // Load saved images
                    ScopedPtr<File> infile(File::Create(savePath, File::OPEN | File::READ));
                    eErrorCode loadCode = ImageSystem::Instance()->LoadWithoutDecompession(infile, reLoadedImageSet, 0, 0);
                    TEST_VERIFY(eErrorCode::SUCCESS == loadCode);
                }

                bool sizeEqual = (reLoadedImageSet.size() == imageSet.size());
                TEST_VERIFY(sizeEqual);
                if (sizeEqual)
                {
                    for (uint32 mip = 0; mip < td.mipmapsCount; ++mip)
                    {
                        TEST_VERIFY(imageSet[mip]->width == reLoadedImageSet[mip]->width);
                        TEST_VERIFY(imageSet[mip]->height == reLoadedImageSet[mip]->height);
                        TEST_VERIFY(imageSet[mip]->format == reLoadedImageSet[mip]->format);
                        TEST_VERIFY(imageSet[mip]->dataSize == reLoadedImageSet[mip]->dataSize);
                        TEST_VERIFY(Memcmp(imageSet[mip]->data, reLoadedImageSet[mip]->data, reLoadedImageSet[mip]->dataSize) == 0);
                        TEST_VERIFY(imageSet[mip]->mipmapLevel == reLoadedImageSet[mip]->mipmapLevel);
                        TEST_VERIFY(imageSet[mip]->cubeFaceID == reLoadedImageSet[mip]->cubeFaceID);
                    }
                }

                LibPVRHelperTestLocal::ReleaseImages(reLoadedImageSet);
            }

            LibPVRHelperTestLocal::ReleaseImages(imageSet);
        }
    }

    DAVA_TEST (CubeMapTest)
    {
        const FilePath outFolderPathname = "~doc:/TestData/LibPVRHelperTest/";
        LibPVRHelperTestLocal::PrepareWorkingFolder(outFolderPathname);

        static Vector<LibPVRHelperTestLocal::TestData> testData =
        {
          { "~res:/TestData/LibPVRHelperTest/pvr2_cube.pvr", 32, 32, 6, 0, PixelFormat::FORMAT_PVR2 },
        };

        for (const LibPVRHelperTestLocal::TestData& td : testData)
        {
            Vector<Image*> imageSet;
            Vector<Vector<Image*>> cubeImageSet(Texture::CUBE_FACE_COUNT); // [face][mip]

            { // Load
                ScopedPtr<File> infile(File::Create(td.path, File::OPEN | File::READ));
                eErrorCode loadCode = ImageSystem::Instance()->LoadWithoutDecompession(infile, imageSet, td.fromMipmap, 0);
                TEST_VERIFY(eErrorCode::SUCCESS == loadCode);

                bool loaded = imageSet.size() == td.mipmapsCount * Texture::CUBE_FACE_COUNT;
                TEST_VERIFY(loaded);
                if (loaded)
                {
                    uint32 imageIndex = 0;
                    for (uint32 mip = 0; mip < td.mipmapsCount; ++mip)
                    {
                        for (uint32 face = 0; face < Texture::CUBE_FACE_COUNT; ++face)
                        {
                            Image* image = imageSet[imageIndex++];
                            cubeImageSet[face].push_back(image);

                            TEST_VERIFY(image->mipmapLevel == mip);
                            TEST_VERIFY(image->cubeFaceID == face);

                            TEST_VERIFY(image->width == (td.width >> mip));
                            TEST_VERIFY(image->height == (td.height >> mip));
                            TEST_VERIFY(image->format == td.format);
                        }
                    }
                }
            }

            { //Save
                FilePath savePath(td.path);
                savePath.ReplaceDirectory(outFolderPathname);

                eErrorCode saveCode = ImageSystem::Instance()->SaveAsCubeMap(savePath, cubeImageSet, td.format);
                TEST_VERIFY(eErrorCode::SUCCESS == saveCode);

                Vector<Image*> reLoadedImageSet;
                { // Load saved images
                    ScopedPtr<File> infile(File::Create(savePath, File::OPEN | File::READ));
                    eErrorCode loadCode = ImageSystem::Instance()->LoadWithoutDecompession(infile, reLoadedImageSet, 0, 0);
                    TEST_VERIFY(eErrorCode::SUCCESS == loadCode);
                }

                bool sizeEqual = (reLoadedImageSet.size() == imageSet.size());
                TEST_VERIFY(sizeEqual);
                if (sizeEqual)
                {
                    size_type count = imageSet.size();
                    for (size_type i = 0; i < count; ++i)
                    {
                        TEST_VERIFY(imageSet[i]->width == reLoadedImageSet[i]->width);
                        TEST_VERIFY(imageSet[i]->height == reLoadedImageSet[i]->height);
                        TEST_VERIFY(imageSet[i]->format == reLoadedImageSet[i]->format);
                        TEST_VERIFY(imageSet[i]->dataSize == reLoadedImageSet[i]->dataSize);
                        TEST_VERIFY(Memcmp(imageSet[i]->data, reLoadedImageSet[i]->data, reLoadedImageSet[i]->dataSize) == 0);
                        TEST_VERIFY(imageSet[i]->mipmapLevel == reLoadedImageSet[i]->mipmapLevel);
                        TEST_VERIFY(imageSet[i]->cubeFaceID == reLoadedImageSet[i]->cubeFaceID);
                    }
                }

                LibPVRHelperTestLocal::ReleaseImages(reLoadedImageSet);
            }

            LibPVRHelperTestLocal::ReleaseImages(imageSet);
        }
    }

    DAVA_TEST (CRCTest)
    {
        const FilePath outFolderPathname = "~doc:/TestData/LibPVRHelperTest/";
        LibPVRHelperTestLocal::PrepareWorkingFolder(outFolderPathname);

        FilePath sourceImagePath = "~res:/TestData/LibPVRHelperTest/pvr4.pvr";
        FilePath savePath(sourceImagePath);
        savePath.ReplaceDirectory(outFolderPathname);

        TEST_VERIFY(FileSystem::Instance()->CopyFile(sourceImagePath, savePath));

        LibPVRHelper helper;
        uint32 crc = helper.GetCRCFromFile(savePath);
        TEST_VERIFY(crc == 0);

        crc = CRC32::ForFile(savePath);
        helper.AddCRCIntoMetaData(savePath);

        TEST_VERIFY(crc == helper.GetCRCFromFile(savePath));
    }
};
