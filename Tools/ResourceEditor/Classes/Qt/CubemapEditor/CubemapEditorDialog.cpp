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


#include "CubemapEditor/CubemapEditorDialog.h"
#include "CubemapEditor/ClickableQLabel.h"
#include "CubemapEditor/CubemapUtils.h"
#include "Qt/Settings/SettingsManager.h"
#include "Qt/Main/QtUtils.h"
#include "Tools/QtFileDialog/QtFileDialog.h"
#include "ui_cubemapeditordialog.h"
#include "Project/ProjectManager.h"

#include <QMouseEvent>
#include <QMessageBox>

using namespace DAVA;

const String CUBEMAP_LAST_FACE_DIR_KEY = "cubemap_last_face_dir";

CubemapEditorDialog::CubemapEditorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CubemapEditorDialog)
{
    ui->setupUi(this);

    ui->lblSaving->setVisible(false);

    faceHeight = -1.0f;
    faceWidth = -1.0f;
    for(int i = 0; i < CubemapUtils::GetMaxFaces(); ++i)
    {
        facePath << QString::null;
    }
    
    faceChanged = false;

    ConnectSignals();
    
    ui->labelPX->SetVisualRotation(90);
    ui->labelNX->SetVisualRotation(90);
    ui->labelPY->SetVisualRotation(90);
    ui->labelNY->SetVisualRotation(90);
    ui->labelPZ->SetVisualRotation(90);
    ui->labelNZ->SetVisualRotation(90);

    setMouseTracking(true);
}

CubemapEditorDialog::~CubemapEditorDialog()
{
    delete ui;
}

void CubemapEditorDialog::ConnectSignals()
{
    connect(ui->labelPX, &ClickableQLabel::OnLabelClicked, this, &CubemapEditorDialog::OnPXClicked);
    connect(ui->labelNX, &ClickableQLabel::OnLabelClicked, this, &CubemapEditorDialog::OnNXClicked);
    connect(ui->labelPY, &ClickableQLabel::OnLabelClicked, this, &CubemapEditorDialog::OnPYClicked);
    connect(ui->labelNY, &ClickableQLabel::OnLabelClicked, this, &CubemapEditorDialog::OnNYClicked);
    connect(ui->labelPZ, &ClickableQLabel::OnLabelClicked, this, &CubemapEditorDialog::OnPZClicked);
    connect(ui->labelNZ, &ClickableQLabel::OnLabelClicked, this, &CubemapEditorDialog::OnNZClicked);

    connect(ui->labelPX, &ClickableQLabel::OnRotationChanged, this, &CubemapEditorDialog::OnRotationChanged);
    connect(ui->labelNX, &ClickableQLabel::OnRotationChanged, this, &CubemapEditorDialog::OnRotationChanged);
    connect(ui->labelPY, &ClickableQLabel::OnRotationChanged, this, &CubemapEditorDialog::OnRotationChanged);
    connect(ui->labelNY, &ClickableQLabel::OnRotationChanged, this, &CubemapEditorDialog::OnRotationChanged);
    connect(ui->labelPZ, &ClickableQLabel::OnRotationChanged, this, &CubemapEditorDialog::OnRotationChanged);
    connect(ui->labelNZ, &ClickableQLabel::OnRotationChanged, this, &CubemapEditorDialog::OnRotationChanged);

    connect(ui->buttonSave, &QPushButton::clicked, this, &CubemapEditorDialog::OnSave);
    connect(ui->buttonClose, &QPushButton::clicked, this, &CubemapEditorDialog::close);
}

void CubemapEditorDialog::LoadImageFromUserFile(float rotation, int face)
{
    FilePath projectPath = CubemapUtils::GetDialogSavedPath("Internal/CubemapLastFaceDir",
                                                            ProjectManager::Instance()->CurProjectDataSourcePath().GetAbsolutePathname());

    QString fileName = QtFileDialog::getOpenFileName(this,
                                                     tr("Open Cubemap Face Image"),
                                                     QString::fromStdString(projectPath.GetAbsolutePathname()),
                                                     tr("Image Files (*.png)"));

    if(!fileName.isNull())
    {
        String stdFilePath = fileName.toStdString();
        FilePath path(stdFilePath);
        LoadImageTo(path, face, false);

        projectPath = stdFilePath;
        SettingsManager::SetValue(Settings::Internal_CubemapLastFaceDir, VariantType(projectPath.GetDirectory()));

        if(AllFacesLoaded())
        {
            ui->legend->setVisible(false);
        }
    }
}

bool CubemapEditorDialog::LoadImageTo(const DAVA::FilePath& filePath, int face, bool silent)
{
    bool result = true;

    QString fileName = filePath.GetAbsolutePathname().c_str();
    QString errorString;
    if (VerifyImage(filePath, face, errorString))
    {
        QImage faceImage(fileName);

        ClickableQLabel* label = GetLabelForFace(face);
        QImage scaledFace = faceImage.scaled(label->width(), label->height());
        label->setPixmap(QPixmap::fromImage(scaledFace));
        label->SetFaceLoaded(true);
        label->SetRotation(0);

        facePath.replace(face, fileName);

        if(faceHeight != faceImage.height())
        {
            faceHeight = faceImage.height();
            faceWidth = faceImage.width();
            UpdateFaceInfo();
        }

        faceChanged = true;

        UpdateButtonState();
    }
    else
    {
        if(!silent)
        {
            QString message = QString("%1\n is not suitable as current cubemap face!\n%2").
                                     arg(fileName).
                                     arg(errorString);
            ShowErrorDialog(message.toStdString());
        }

        result = false;
    }

    return result;
}

ClickableQLabel* CubemapEditorDialog::GetLabelForFace(int face)
{
    ClickableQLabel* labels[] =
    {
        ui->labelPX,
        ui->labelNX,
        ui->labelPY,
        ui->labelNY,
        ui->labelPZ,
        ui->labelNZ
    };

    return labels[face];
}

bool CubemapEditorDialog::VerifyImage(const DAVA::FilePath& path, int faceIndex, QString &errorString)
{
    ImageInfo info = ImageSystem::Instance()->GetImageInfo(path);
    if (!IsFormatValid(info))
    {
        errorString = QString("Incorrect format.");
        return false;
    }
    else if (GetLoadedFaceCount() > 1 ||
             (GetLoadedFaceCount() == 1 && QString::null == facePath.at(faceIndex)))
    {
        if (info.width != faceWidth ||
            info.height != faceHeight)
        {
            errorString = QString("Image size not equal face size.");
            return false;
        }
    }

    if (info.width != info.height)
    {
        errorString = QString("Width and height are not equal");
        return false;
    }
    else if (!IsPowerOf2(info.width))
    {
        errorString = QString("Width or height are not power of two");
        return false;
    }

    return true;
}

bool CubemapEditorDialog::IsFormatValid(const DAVA::ImageInfo &info)
{
    switch (info.format)
    {
        case FORMAT_RGBA4444:
        case FORMAT_RGBA5551:
        case FORMAT_RGBA8888:
        case FORMAT_RGB565:
        case FORMAT_RGB888:
        case FORMAT_A8:
        case FORMAT_A16:
            return true;
        default:
            return false;
    }
}

void CubemapEditorDialog::UpdateFaceInfo()
{
    ui->labelFaceHeight->setText(QString::number(faceHeight));
    ui->labelFaceWidth->setText(QString::number(faceWidth));
}

void CubemapEditorDialog::UpdateButtonState()
{
	//check if all files are present.
	//while file formats specs allow to specify cubemaps partially actual implementations don't allow that
	bool enableSave = AllFacesLoaded();
	
	if(enableSave)
	{
		enableSave = IsCubemapEdited();
	}
	
	ui->buttonSave->setEnabled(enableSave);
}

bool CubemapEditorDialog::AnyFaceLoaded()
{
	bool faceLoaded = false;
	for(int i = 0; i < CubemapUtils::GetMaxFaces(); ++i)
	{
		if(QString::null != facePath.at(i))
		{
			faceLoaded = true;
			break;
		}
	}

	return faceLoaded;
}

bool CubemapEditorDialog::AllFacesLoaded()
{
	bool faceLoaded = true;
	for(int i = 0; i < CubemapUtils::GetMaxFaces(); ++i)
	{
		if(QString::null == facePath.at(i))
		{
			faceLoaded = false;
			break;
		}
	}
	
	return faceLoaded;
}

int CubemapEditorDialog::GetLoadedFaceCount()
{
	int faceLoaded = 0;
	for(int i = 0; i < CubemapUtils::GetMaxFaces(); ++i)
	{
		if(QString::null != facePath.at(i))
		{
			faceLoaded++;
		}
	}
	
	return faceLoaded;
}

void CubemapEditorDialog::LoadCubemap(const QString& path)
{
	FilePath filePath(path.toStdString());
	TextureDescriptor* texDescriptor = TextureDescriptor::CreateFromFile(filePath);
	
	if(NULL != texDescriptor &&
	   texDescriptor->IsCubeMap())
	{
		String fileNameWithoutExtension = filePath.GetFilename();
		String extension = filePath.GetExtension();
		fileNameWithoutExtension.replace(fileNameWithoutExtension.find(extension), extension.size(), "");

		bool cubemapLoadResult = true;
		for(int i = 0; i < CubemapUtils::GetMaxFaces(); ++i)
		{
			if(texDescriptor->dataSettings.faceDescription & (1 << CubemapUtils::MapUIToFrameworkFace(i)))
			{
				FilePath faceFilePath = filePath;
				faceFilePath.ReplaceFilename(fileNameWithoutExtension +
											 CubemapUtils::GetFaceNameSuffix(CubemapUtils::MapUIToFrameworkFace(i)) + "." +
											 CubemapUtils::GetDefaultFaceExtension());

				bool faceLoadResult = LoadImageTo(faceFilePath.GetAbsolutePathname(), i, true);
				cubemapLoadResult = cubemapLoadResult && faceLoadResult;
			}
		}
		
		if(!cubemapLoadResult)
		{
			ShowErrorDialog("This cubemap texture seems to be damaged.\nPlease repair it by setting image(s) to empty face(s) and save to disk.");
		}
	}
	else
	{
		if(NULL == texDescriptor)
		{
			ShowErrorDialog("Failed to load cubemap texture " + path.toStdString());
		}
		else
		{
			ShowErrorDialog("Failed to load cubemap texture " + path.toStdString() + ". Seems this is not a cubemap texture.");
		}
	}

	SafeDelete(texDescriptor);
}

void CubemapEditorDialog::SaveCubemap(const QString& path)
{
	FilePath filePath(path.toStdString());
	DAVA::uint8 faceMask = GetFaceMask();
		
	//copy file to the location where .tex will be put. Add suffixes to file names to distinguish faces
	String fileNameWithoutExtension = filePath.GetFilename();
	String extension = filePath.GetExtension();
	fileNameWithoutExtension.replace(fileNameWithoutExtension.find(extension), extension.size(), "");
	for(int i = 0 ; i < CubemapUtils::GetMaxFaces(); ++i)
	{
		if(!facePath.at(i).isNull())
		{
			FilePath faceFilePath = filePath;
			faceFilePath.ReplaceFilename(fileNameWithoutExtension +
										 CubemapUtils::GetFaceNameSuffix(CubemapUtils::MapUIToFrameworkFace(i)) + "." +
										 CubemapUtils::GetDefaultFaceExtension());

			DAVA::String targetFullPath = faceFilePath.GetAbsolutePathname().c_str();
			if(facePath.at(i) != targetFullPath.c_str())
			{
				if(QFile::exists(targetFullPath.c_str()))
				{
					int answer = ShowQuestion("File overwrite",
											  "File " + targetFullPath + " already exist. Do you want to overwrite it with " + facePath.at(i).toStdString(),
											  MB_FLAG_YES | MB_FLAG_NO, MB_FLAG_NO);
					
					if(MB_FLAG_YES == answer)
					{
						bool removeResult = QFile::remove(targetFullPath.c_str());
						
						if(!removeResult)
						{
							ShowErrorDialog("Failed to copy texture " + facePath.at(i).toStdString() + " to " + targetFullPath.c_str());
							return;
						}

					}
					else
					{
						continue;
					}
				}
				
				bool copyResult = QFile::copy(facePath.at(i), targetFullPath.c_str());
				
				if(!copyResult)
				{
					ShowErrorDialog("Failed to copy texture " + facePath.at(i).toStdString() + " to " + targetFullPath);
					return;
				}
			}
			
			ClickableQLabel* faceLabel = GetLabelForFace(i);
			if(faceLabel->GetRotation() != 0)
			{
				QTransform transform;
				transform.rotate(faceLabel->GetRotation());
				QImage qimg(targetFullPath.c_str());
				QImage rotatedImage = qimg.transformed(transform);
				rotatedImage.save(targetFullPath.c_str());
                faceLabel->SetRotation(0);
			}
		}
	}
	
	TextureDescriptor* descriptor = new TextureDescriptor();
    bool descriptorReady = false;
    if(filePath.Exists())
    {
        descriptorReady = descriptor->Load(filePath);
    }
    
    if(!descriptorReady)
    {
        descriptor->SetDefaultValues();
        descriptor->drawSettings.wrapModeS = descriptor->drawSettings.wrapModeT = rhi::TEXADDR_CLAMP;
    }
    
	descriptor->dataSettings.faceDescription = faceMask;

    descriptor->Save(filePath);
	SafeDelete(descriptor);
	
	QMessageBox::information(this, "Cubemap texture save result", "Cubemap texture was saved successfully!");
}

DAVA::uint8 CubemapEditorDialog::GetFaceMask()
{
	DAVA::uint8 mask = 0;
	for(int i = 0 ; i < CubemapUtils::GetMaxFaces(); ++i)
	{
		if(!facePath.at(i).isNull())
		{
			mask |= 1 << CubemapUtils::MapUIToFrameworkFace(i);
		}
	}
	
	return mask;
}

void CubemapEditorDialog::InitForEditing(DAVA::FilePath& textureDescriptorPath, DAVA::FilePath& root)
{
	targetFile = textureDescriptorPath;
	editorMode = CubemapEditorDialog::eEditorModeEditing;
	rootPath = QString::fromStdString(root.GetAbsolutePathname());
	
	LoadCubemap(targetFile.GetAbsolutePathname().c_str());
	
	ui->buttonSave->setEnabled(false);
	ui->legend->setVisible(false);
	
	faceChanged = false;
}

void CubemapEditorDialog::InitForCreating(DAVA::FilePath& textureDescriptorPath, DAVA::FilePath& root)
{
	targetFile = textureDescriptorPath;
	editorMode = CubemapEditorDialog::eEditorModeCreating;
	rootPath = QString::fromStdString(root.GetAbsolutePathname());
	ui->legend->setVisible(true);
	
	faceChanged = false;
}

////////////////////////////////////////////////////

void CubemapEditorDialog::OnPXClicked()
{
	LoadImageFromUserFile(0, CUBEMAPEDITOR_FACE_PX);
}

void CubemapEditorDialog::OnNXClicked()
{
	LoadImageFromUserFile(0, CUBEMAPEDITOR_FACE_NX);
}

void CubemapEditorDialog::OnPYClicked()
{
	LoadImageFromUserFile(0, CUBEMAPEDITOR_FACE_PY);
}

void CubemapEditorDialog::OnNYClicked()
{
	LoadImageFromUserFile(0, CUBEMAPEDITOR_FACE_NY);
}

void CubemapEditorDialog::OnPZClicked()
{
	LoadImageFromUserFile(0, CUBEMAPEDITOR_FACE_PZ);
}

void CubemapEditorDialog::OnNZClicked()
{
	LoadImageFromUserFile(0, CUBEMAPEDITOR_FACE_NZ);
}

void CubemapEditorDialog::OnLoadTexture()
{
	int answer = MB_FLAG_YES;
	if(AnyFaceLoaded())
	{
		answer = ShowQuestion("Warning",
								  "Do you really want to load new cubemap texture over currently loaded one?",
								  MB_FLAG_YES | MB_FLAG_NO,
								  MB_FLAG_NO);		
	}
	
	if(MB_FLAG_YES == answer)
	{
		QString fileName = QtFileDialog::getOpenFileName(this,
														tr("Open Cubemap Texture"),
														rootPath,
														tr("Tex File (*.tex)"));
		
		if(!fileName.isNull())
		{
			LoadCubemap(fileName);
		}
	}
}

void CubemapEditorDialog::OnSave()
{
	//check if all files are present.
	//while file formats specs allows to specify cubemaps partially actual implementations don't allow that
	if(!AllFacesLoaded())
	{
		ShowErrorDialog("Please specify ALL cubemap faces.");
		return;
	}
	
	ui->lblSaving->setVisible(true);
	
	this->paintEvent(NULL);
	ui->lblSaving->update();
	QApplication::processEvents();
	QApplication::flush();
	
	this->setUpdatesEnabled(false);
	
	SaveCubemap(targetFile.GetAbsolutePathname().c_str());
	
	faceChanged = false;
	
	this->setUpdatesEnabled(true);
	ui->lblSaving->setVisible(false);
    QDialog::accept();
}

void CubemapEditorDialog::done(int result)
{
	if(IsCubemapEdited())
	{
	    int answer = ShowQuestion("Warning",
							  "Cubemap texture was edited. Do you want to close it without saving?",
							  MB_FLAG_YES | MB_FLAG_NO,
							  MB_FLAG_NO);

       if(answer != MB_FLAG_YES)
       {
           return;
       }
	}
    QDialog::done(QDialog::Accepted);
}

bool CubemapEditorDialog::IsCubemapEdited()
{
	bool edited = faceChanged;
	
	if(!edited)
	{
		ClickableQLabel* labels[] =
		{
			ui->labelPX,
			ui->labelNX,
			ui->labelPY,
			ui->labelNY,
			ui->labelPZ,
			ui->labelNZ
		};

		for(int i = 0; i < CubemapUtils::GetMaxFaces(); ++i)
		{
			if(labels[i]->GetRotation() != 0)
			{
				edited = true;
				break;
			}
		}
	}
	
	return edited;
}

void CubemapEditorDialog::OnRotationChanged()
{
	UpdateButtonState();
}

void CubemapEditorDialog::mouseMoveEvent(QMouseEvent *ev)
{
	ClickableQLabel* labels[] =
	{
		ui->labelPX,
		ui->labelNX,
		ui->labelPY,
		ui->labelNY,
		ui->labelPZ,
		ui->labelNZ
	};

	for(int i = 0; i < CubemapUtils::GetMaxFaces(); ++i)
	{
		labels[i]->OnParentMouseMove(ev);
	}

	QDialog::mouseMoveEvent(ev);
}

