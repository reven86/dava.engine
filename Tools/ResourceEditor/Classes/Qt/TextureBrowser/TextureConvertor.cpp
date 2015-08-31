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


#include <QPainter>
#include <QProcess>
#include <QTextOption>
#include <QPushButton>
#include <QLabel>
#include <QFileInfo>
#include <QtConcurrent>
#include <QImage>

#include "Main/mainwindow.h"
#include "TextureBrowser/TextureConvertor.h"
#include "TextureCompression/TextureConverter.h"
#include "Deprecated/SceneValidator.h"
#include "Render/Image/LibDdsHelper.h"

#include "FileSystem/FileSystem.h"

#include "QtLayer.h"
#include "Main/QtUtils.h"
#include "Scene/SceneHelper.h"
#include "ImageTools/ImageTools.h"
#include "Settings/SettingsManager.h"

TextureConvertor::TextureConvertor()
	: jobIdCounter(1)
	, convertJobQueueSize(0)
	, waitingComletion(0)
	, curJobThumbnail(NULL)
	, curJobOriginal(NULL)
	, curJobConverted(NULL)
	, waitDialog(NULL)
{
	// slots will be called in connector(this) thread
	QObject::connect(&thumbnailWatcher, SIGNAL(finished()), this, SLOT(threadThumbnailFinished()), Qt::QueuedConnection);
	QObject::connect(&originalWatcher, SIGNAL(finished()), this, SLOT(threadOriginalFinished()), Qt::QueuedConnection);
	QObject::connect(&convertedWatcher, SIGNAL(finished()), this, SLOT(threadConvertedFinished()), Qt::QueuedConnection);
}

TextureConvertor::~TextureConvertor()
{
	CancelConvert();
	thumbnailWatcher.waitForFinished();
	originalWatcher.waitForFinished();
	convertedWatcher.waitForFinished();
}

int TextureConvertor::GetThumbnail(const DAVA::TextureDescriptor *descriptor)
{
	int ret = 0;

	if(NULL != descriptor)
	{
		// check if requested texture isn't the same that is loading now
		if(NULL == curJobThumbnail || curJobThumbnail->descriptor != descriptor)
		{
			JobItem newJob;
			newJob.id = jobIdCounter++;
			newJob.descriptor = descriptor;

			jobStackThumbnail.push(newJob);
			jobRunNextThumbnail();

			ret = newJob.id;
		}
	}

	return ret;
}

int TextureConvertor::GetOriginal(const DAVA::TextureDescriptor *descriptor)
{
	int ret = 0;

	if(NULL != descriptor)
	{
		// check if requested texture isn't the same that is loading now
		if(NULL == curJobOriginal || curJobOriginal->descriptor != descriptor)
		{
			JobItem newJob;
			newJob.id = jobIdCounter++;
			newJob.descriptor = descriptor;

			jobStackOriginal.push(newJob);
			jobRunNextOriginal();

			ret = newJob.id;
		}
	}

	return ret;
}

int TextureConvertor::GetConverted(const DAVA::TextureDescriptor *descriptor, DAVA::eGPUFamily gpu,
                                   eTextureConvertMode convertMode /* = CONVERT_NOT_EXISTENT */)
{
	int ret = 0;

	if(NULL != descriptor)
	{
		JobItem newJob;
		newJob.id = jobIdCounter++;
		newJob.convertMode = convertMode;
		newJob.type = gpu;
		newJob.descriptor = descriptor;

		if(jobStackConverted.push(newJob))
		{
			convertJobQueueSize++;
		}

		jobRunNextConvert();

		ret = newJob.id;
	}

	return ret;
}

int TextureConvertor::Reconvert(DAVA::Scene *scene, eTextureConvertMode convertMode)
{
	int ret = 0;

	if(NULL != scene)
	{
		// get list of all scenes textures
		DAVA::TexturesMap allTextures;
		SceneHelper::EnumerateSceneTextures(scene, allTextures, SceneHelper::TexturesEnumerateMode::EXCLUDE_NULL);

		// add jobs to convert every texture
		if(allTextures.size() > 0)
		{
			DAVA::TexturesMap::iterator begin = allTextures.begin();
			DAVA::TexturesMap::iterator end = allTextures.end();

			for(; begin != end; begin++)
			{
				DAVA::TextureDescriptor *descriptor = begin->second->GetDescriptor();

				if(NULL != descriptor)
				{
					DVASSERT(descriptor->compression);
					for(int gpu = 0; gpu < DAVA::GPU_DEVICE_COUNT; ++gpu)
					{
						if( ! GPUFamilyDescriptor::IsFormatSupported((eGPUFamily)gpu, (PixelFormat)descriptor->compression[gpu].format))
						{
							continue;
						}

						JobItem newJob;
						newJob.id = jobIdCounter++;
						newJob.descriptor = descriptor;
						newJob.convertMode = convertMode;
						newJob.type = gpu;

						if(jobStackConverted.push(newJob))
						{
							convertJobQueueSize++;
						}

						jobRunNextConvert();

						ret = newJob.id;
					}
				}
			}
		}
	}

	// 0 means no job were created
	return ret;
}

void TextureConvertor::WaitConvertedAll(QWidget *parent)
{
	if(convertJobQueueSize > 0)
	{
        if (nullptr == parent)
        {
            parent = QtMainWindow::Instance();
        }

		waitDialog = new QtWaitDialog(parent);
		bool hasCancel = false;
		
		if(jobStackConverted.size() > 0)
		{
			hasCancel = true;
		}

        connect(waitDialog, &QtWaitDialog::canceled, this, &TextureConvertor::waitCanceled);

		waitDialog->SetRange(0, convertJobQueueSize);
		waitDialog->SetValue(convertJobQueueSize - jobStackConverted.size());
		waitDialog->SetMessage(waitStatusText);

		waitingComletion = true;
		waitDialog->Exec("Waiting for conversion completion", waitStatusText, true, hasCancel);

		waitDialog->deleteLater();
		waitDialog = nullptr;
	}
}

void TextureConvertor::CancelConvert()
{
	JobItem *item = jobStackConverted.pop();

	while (NULL != item)
	{
		delete item;
		item = jobStackConverted.pop();
	}
}

void TextureConvertor::jobRunNextThumbnail()
{
	// if there is no already running work
	if((thumbnailWatcher.isFinished() || thumbnailWatcher.isCanceled()) && NULL == curJobThumbnail)
	{
		// get the new work
		curJobThumbnail = jobStackThumbnail.pop();
		if(NULL != curJobThumbnail)
		{
			// copy descriptor
			QFuture< TextureInfo > f = QtConcurrent::run(this, &TextureConvertor::GetThumbnailThread, curJobThumbnail);
			thumbnailWatcher.setFuture(f);
		}
	}
}


void TextureConvertor::jobRunNextOriginal()
{
	// if there is no already running work
	if((originalWatcher.isFinished() || originalWatcher.isCanceled()) && NULL == curJobOriginal)
	{
		// get the new work
		curJobOriginal = jobStackOriginal.pop();
		if(NULL != curJobOriginal)
		{
			// copy descriptor
			QFuture< TextureInfo > f = QtConcurrent::run(this, &TextureConvertor::GetOriginalThread, curJobOriginal);
			originalWatcher.setFuture(f);
		}
	}
}

void TextureConvertor::jobRunNextConvert()
{
	// if there is no already running work
	if((convertedWatcher.isFinished() || convertedWatcher.isCanceled()) && NULL == curJobConverted)
	{
		// get the next job
		curJobConverted = jobStackConverted.pop();
		if(NULL != curJobConverted)
		{
			TextureDescriptor *desc = (TextureDescriptor *) curJobConverted->descriptor;

			QFuture< TextureInfo > f = QtConcurrent::run(this, &TextureConvertor::GetConvertedThread, curJobConverted);
			convertedWatcher.setFuture(f);

			emit ConvertStatusImg(desc->pathname.GetAbsolutePathname().c_str(), curJobConverted->type);
			emit ConvertStatusQueue(convertJobQueueSize - jobStackConverted.size(), convertJobQueueSize);

			// generate current wait message, that can be displayed by wait dialog
			waitStatusText = "Path: ";
			waitStatusText += desc->pathname.GetAbsolutePathname().c_str();
			waitStatusText += "\n\nGPU: ";
			waitStatusText += GlobalEnumMap<DAVA::eGPUFamily>::Instance()->ToString(curJobConverted->type);

			if(NULL != waitDialog)
			{
				waitDialog->SetValue(convertJobQueueSize - jobStackConverted.size());
				waitDialog->SetMessage(waitStatusText);
			}
		}
		else
		{
			waitStatusText = "";

			// if no job in stack, emit signal that all jobs are finished
			if(jobStackOriginal.size() == 0 && jobStackConverted.size() == 0)
			{
				emit ReadyConvertedAll();
			}

			convertJobQueueSize = 0;

			emit ConvertStatusImg("", DAVA::GPU_ORIGIN);
			emit ConvertStatusQueue(0, 0);

			if(NULL != waitDialog)
			{
				// close wait dialog
				waitDialog->Reset();
			}
		}
	}
	else
	{
		emit ConvertStatusQueue(convertJobQueueSize - jobStackConverted.size(), convertJobQueueSize);

		if(NULL != waitDialog)
		{
			waitDialog->SetRangeMax(convertJobQueueSize);
		}
	}
}

void TextureConvertor::threadThumbnailFinished()
{
	if(thumbnailWatcher.isFinished() && NULL != curJobThumbnail)
	{
		const DAVA::TextureDescriptor *thumbnailDescriptor = (DAVA::TextureDescriptor *) curJobThumbnail->descriptor;

		TextureInfo watcherResult = thumbnailWatcher.result();
		emit ReadyThumbnail(thumbnailDescriptor, watcherResult);

		delete curJobThumbnail;
		curJobThumbnail = NULL;
	}

	jobRunNextThumbnail();
}


void TextureConvertor::threadOriginalFinished()
{
	if(originalWatcher.isFinished() && NULL != curJobOriginal)
	{
		const DAVA::TextureDescriptor *originalDescriptor = (DAVA::TextureDescriptor *) curJobOriginal->descriptor;

		TextureInfo watcherResult = originalWatcher.result();
		emit ReadyOriginal(originalDescriptor, watcherResult);

		delete curJobOriginal;
		curJobOriginal = NULL;
	}

	jobRunNextOriginal();
}

void TextureConvertor::threadConvertedFinished()
{
	if(convertedWatcher.isFinished() && NULL != curJobConverted)
	{
		const DAVA::TextureDescriptor *convertedDescriptor = (DAVA::TextureDescriptor *) curJobConverted->descriptor;

		TextureInfo watcherResult = convertedWatcher.result();
		emit ReadyConverted(convertedDescriptor, (DAVA::eGPUFamily) curJobConverted->type, watcherResult);

		delete curJobConverted;
		curJobConverted = NULL;
	}

	jobRunNextConvert();
}

void TextureConvertor::waitCanceled()
{
	CancelConvert();
}

TextureInfo TextureConvertor::GetThumbnailThread(JobItem *item)
{
	TextureInfo result;

	void *pool = DAVA::QtLayer::Instance()->CreateAutoreleasePool();

	if(NULL != item && NULL != item->descriptor)
	{
		TextureDescriptor *descriptor = (TextureDescriptor *) item->descriptor;

		DAVA::uint32 fileSize = 0;
		if(descriptor->IsCubeMap())
		{
			DAVA::Vector<DAVA::FilePath> cubeFaceNames;
			descriptor->GetFacePathnames(cubeFaceNames);

			for(auto& faceName : cubeFaceNames)
			{
                if(faceName.IsEmpty())
                    continue;

                QImage img = ImageTools::FromDavaImage(faceName);
                result.images.push_back(img);
                fileSize += QFileInfo(faceName.GetAbsolutePathname().c_str()).size();
			}
		}
		else
		{
            QImage img = ImageTools::FromDavaImage(descriptor->GetSourceTexturePathname());
			result.images.push_back(img);
			fileSize = QFileInfo(descriptor->GetSourceTexturePathname().GetAbsolutePathname().c_str()).size();
		}

		result.dataSize = ImageTools::GetTexturePhysicalSize(descriptor, DAVA::GPU_ORIGIN);
		result.fileSize = fileSize;
	}

	DAVA::QtLayer::Instance()->ReleaseAutoreleasePool(pool);

	return result;
}

TextureInfo TextureConvertor::GetOriginalThread(JobItem *item)
{
	TextureInfo result;

	void *pool = DAVA::QtLayer::Instance()->CreateAutoreleasePool();
    
	if(NULL != item && NULL != item->descriptor)
	{
		TextureDescriptor *descriptor = (TextureDescriptor *) item->descriptor;
		
		DAVA::uint32 fileSize = 0;
		if(descriptor->IsCubeMap())
		{
			DAVA::Vector<DAVA::FilePath> cubeFaceNames;
			descriptor->GetFacePathnames(cubeFaceNames);
			
			for(auto& faceName : cubeFaceNames)
			{
				if(faceName.IsEmpty())
                    continue;

                QImage img = ImageTools::FromDavaImage(faceName);
                result.images.push_back(img);

                fileSize += QFileInfo(faceName.GetAbsolutePathname().c_str()).size();
			}
		}
		else
		{
			QImage img = ImageTools::FromDavaImage(descriptor->GetSourceTexturePathname());
			result.images.push_back(img);
			fileSize = QFileInfo(descriptor->GetSourceTexturePathname().GetAbsolutePathname().c_str()).size();
		}

		result.dataSize = ImageTools::GetTexturePhysicalSize(descriptor, DAVA::GPU_ORIGIN);
		result.fileSize = fileSize;

		if(result.images.size())
		{
			result.imageSize.setWidth(result.images[0].width());
			result.imageSize.setHeight(result.images[0].height());
		}
	}

    DAVA::QtLayer::Instance()->ReleaseAutoreleasePool(pool);
	
	return result;
}

TextureInfo TextureConvertor::GetConvertedThread(JobItem *item)
{
	void *pool = DAVA::QtLayer::Instance()->CreateAutoreleasePool();

	TextureInfo result;

	DAVA::Vector<DAVA::Image*> convertedImages;

	if(NULL != item)
	{
		DAVA::TextureDescriptor *descriptor = (DAVA::TextureDescriptor*) item->descriptor;
		DAVA::eGPUFamily gpu = (DAVA::eGPUFamily) item->type;

		if( NULL != descriptor &&
			gpu >= 0 && gpu < DAVA::GPU_FAMILY_COUNT && 
			descriptor->compression[gpu].format > DAVA::FORMAT_INVALID && descriptor->compression[gpu].format < DAVA::FORMAT_COUNT)
		{
			DAVA::FilePath compressedTexturePath = descriptor->CreatePathnameForGPU(gpu);

			ImageFormat compressedFormat = GPUFamilyDescriptor::GetCompressedFileFormat(gpu, (DAVA::PixelFormat) descriptor->compression[gpu].format);
            if (compressedFormat == IMAGE_FORMAT_PVR || compressedFormat == IMAGE_FORMAT_DDS)
			{
				DAVA::Logger::FrameworkDebug("Starting %s conversion (%s), id %d..., (%s)",
                    (compressedFormat == IMAGE_FORMAT_PVR ? "PVR" : "DDS"),
					GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(descriptor->compression[gpu].format), 
                    item->id, 
                    descriptor->pathname.GetAbsolutePathname().c_str());
				convertedImages = ConvertFormat(descriptor, gpu, item->convertMode);
				DAVA::Logger::FrameworkDebug("Done, id %d", item->id);
			}
			else
			{
				DVASSERT(false);
			}


			result.dataSize = ImageTools::GetTexturePhysicalSize(descriptor, gpu);

			result.fileSize = QFileInfo(compressedTexturePath.GetAbsolutePathname().c_str()).size();

			if(convertedImages.size() && convertedImages[0])
			{
				result.imageSize.setWidth(convertedImages[0]->GetWidth());
				result.imageSize.setHeight(convertedImages[0]->GetHeight());
			}
		}
		else
		{
            if(descriptor)
            {
                DAVA::Logger::FrameworkDebug("%s has no converted image for %s", descriptor->pathname.GetStringValue().c_str(), DAVA::GPUFamilyDescriptor::GetGPUName(gpu).c_str());
            }
            else
            {
                DAVA::Logger::Error("[TextureConvertor::GetConvertedThread] NULL descriptor for job(%d)", item->id);
            }
		}

	}

	if(convertedImages.size() > 0)
	{
		for(size_t i = 0; i < convertedImages.size(); ++i)
		{
			if(convertedImages[i] != NULL)
			{
                QImage img = ImageTools::FromDavaImage(convertedImages[i]);
				result.images.push_back(img);
			
				convertedImages[i]->Release();
			}
			else
			{
				QImage img;
				result.images.push_back(img);
			}
		}
	}
	else
	{
		int stubImageCount = Texture::CUBE_FACE_COUNT;
		if(NULL != item)
		{
			DAVA::TextureDescriptor *descriptor = (DAVA::TextureDescriptor*) item->descriptor;
			if(NULL != descriptor &&
			   !descriptor->IsCubeMap())
			{
				stubImageCount = 1;
			}
		}
		
		for(int i = 0; i < stubImageCount; ++i)
		{
			QImage img;
			result.images.push_back(img);
		}
	}


	DAVA::QtLayer::Instance()->ReleaseAutoreleasePool(pool);
	
	return result;
}

DAVA::Vector<DAVA::Image*> TextureConvertor::ConvertFormat(DAVA::TextureDescriptor *descriptor, DAVA::eGPUFamily gpu,
                                                           eTextureConvertMode convertMode)
{
	DAVA::Vector<DAVA::Image*> resultImages;
	DAVA::FilePath outputPath = TextureConverter::GetOutputPath(*descriptor, gpu);
	if(!outputPath.IsEmpty())
	{
        bool convert = false;

        switch (convertMode)
        {
            case CONVERT_FORCE:
                convert = true;
                break;

            case CONVERT_MODIFIED:
                convert = !descriptor->IsCompressedTextureActual(gpu);
                break;

            case CONVERT_NOT_EXISTENT:
                convert = !DAVA::FileSystem::Instance()->IsFile(outputPath);
                break;

            default:
                DVASSERT(false && "Invalid case");
                break;
        }

		if(convert)
		{
            DAVA::VariantType quality = SettingsManager::Instance()->GetValue(Settings::General_CompressionQuality);
			outputPath = TextureConverter::ConvertTexture(*descriptor, gpu, true, (TextureConverter::eConvertQuality)quality.AsInt32());
        }
		
        Vector<DAVA::Image *> davaImages;
        DAVA::ImageSystem::Instance()->Load(outputPath, davaImages);
		
		if(davaImages.size() > 0)
		{
			if(!descriptor->IsCubeMap())
			{
				DAVA::Image* image = davaImages[0];
				image->Retain();
				
				resultImages.push_back(image);
			}
			else
			{
				//select images with mipmap level = 0 for cube map display
				for(size_t i = 0; i < davaImages.size(); ++i)
				{
					DAVA::Image* image = davaImages[i];
					if(0 == image->mipmapLevel)
					{
						image->Retain();
						resultImages.push_back(image);
					}
				}
				
				if(resultImages.size() < Texture::CUBE_FACE_COUNT)
				{
					int imagesToAdd = Texture::CUBE_FACE_COUNT - resultImages.size();
					for(int i = 0; i < imagesToAdd; ++i)
					{
						resultImages.push_back(NULL);
					}
				}
			}
			
			for_each(davaImages.begin(), davaImages.end(),  DAVA::SafeRelease<DAVA::Image>);
		}
		else
		{
			int stubImageCount = (descriptor->IsCubeMap()) ? Texture::CUBE_FACE_COUNT : 1;
			for(int i = 0; i < stubImageCount; ++i)
			{
				resultImages.push_back(NULL);
			}
		}
	}
	
	return resultImages;
}


