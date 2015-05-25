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


#include "ApplicationSettings.h"

#include "FileSystem/FileSystem.h"

void ApplicationSettings::Save()
{
    DAVA::FileSystem::Instance()->CreateDirectory("~doc:/AssetServer", true);

    static DAVA::FilePath path("~doc:/AssetServer/ACS_settings.dat");
    
    DAVA::ScopedPtr<DAVA::File> file(DAVA::File::Create(path, DAVA::File::CREATE | DAVA::File::WRITE));
    
    if(static_cast<DAVA::File *>(file))
    
    
    DAVA::File *f =
    if (f == nullptr)
    {
        DAVA::Logger::Error("File not open. %s. %s", DAVA::String("MainWindow::ReadSettings").c_str(), path.GetAbsolutePathname().c_str());
        return;
    }
    
    DAVA::KeyedArchive *arch = new DAVA::KeyedArchive();
    
    arch->SetString(DAVA::String("FolderPath"), DAVA::String(ui->cachFolderLineEdit->text().toStdString()));
    arch->SetFloat(DAVA::String("FolderSize"), static_cast<DAVA::float32>(ui->cachSizeSpinBox->value()));
    arch->SetUInt32(DAVA::String("NumberOfFiles"), static_cast<DAVA::uint32>(ui->numberOfFilesSpinBox->value()));
    //    arch->SetUInt32(DAVA::String("Port"), static_cast<DAVA::uint32>(ui->portSpinBox->value()));
    
    auto size = servers.size();
    arch->SetUInt32("ServersSize", size);
    
    for (int i = 0; i < size; ++i)
    {
        auto sData = servers.at(i)->GetServerData();
        //        arch->SetString(DAVA::Format("Server_%d_ip", i), DAVA::String(sData.ip.toStdString()));
        arch->SetUInt32(DAVA::Format("Server_%d_port", i), static_cast<DAVA::uint32>(sData.port));
    }
    
    arch->Save(f);
    f->Release();
    arch->Release();

}

void ApplicationSettings::Load()
{
    
}

void ApplicationSettings::Serialize(DAVA::KeyedArchive * archieve) const
{
    DVASSERT(nullptr != archieve);
    
    arch->SetString("FolderPath", folder.GetStringValue());
    arch->SetFloat6(DAVA::String("FolderSize"), static_cast<DAVA::float32>(ui->cachSizeSpinBox->value()));
    arch->SetUInt32(DAVA::String("NumberOfFiles"), static_cast<DAVA::uint32>(ui->numberOfFilesSpinBox->value()));
    //    arch->SetUInt32(DAVA::String("Port"), static_cast<DAVA::uint32>(ui->portSpinBox->value()));
    
    auto size = servers.size();
    arch->SetUInt32("ServersSize", size);
    
    for (int i = 0; i < size; ++i)
    {
        auto sData = servers.at(i)->GetServerData();
        //        arch->SetString(DAVA::Format("Server_%d_ip", i), DAVA::String(sData.ip.toStdString()));
        arch->SetUInt32(DAVA::Format("Server_%d_port", i), static_cast<DAVA::uint32>(sData.port));
    }

}

void ApplicationSettings::Deserialize(DAVA::KeyedArchive * archieve)
{
    DVASSERT(nullptr != archieve);
}

