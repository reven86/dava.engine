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


#include "QtTools/FileDialog/FileDialog.h"

QString FileDialog::getOpenFileName(QWidget *parent, const QString &caption, const QString &dir,
                                     const QString &filter, QString *selectedFilter, Options options)
{
    auto fileName = QFileDialog::getOpenFileName(parent, caption, dir, filter, selectedFilter, options);

    return fileName;
}


QUrl FileDialog::getOpenFileUrl(QWidget *parent, const QString &caption, const QUrl &dir,
                                 const QString &filter, QString *selectedFilter, Options options, const QStringList &supportedSchemes)
{
    auto fileUrl = QFileDialog::getOpenFileUrl(parent, caption, dir, filter, selectedFilter, options, supportedSchemes);
    
    return fileUrl;
}


QString FileDialog::getSaveFileName(QWidget *parent, const QString &caption, const QString &dir,
                               const QString &filter, QString *selectedFilter, Options options)
{
    auto fileName = QFileDialog::getSaveFileName(parent, caption, dir, filter, selectedFilter, options);
    
    return fileName;
}


QUrl FileDialog::getSaveFileUrl(QWidget *parent, const QString &caption, const QUrl &dir,
                           const QString &filter, QString *selectedFilter, Options options,
                           const QStringList &supportedSchemes)
{
    auto fileUrl = QFileDialog::getSaveFileUrl(parent, caption, dir, filter, selectedFilter, options, supportedSchemes);
    
    return fileUrl;
}


QString FileDialog::getExistingDirectory(QWidget *parent, const QString &caption, const QString &dir, Options options)
{
    auto directory = QFileDialog::getExistingDirectory(parent, caption, dir, options);
    
    return directory;
}


QUrl FileDialog::getExistingDirectoryUrl(QWidget *parent, const QString &caption, const QUrl &dir,
                                          Options options, const QStringList &supportedSchemes)
{
    auto dirrectoryUrl = QFileDialog::getExistingDirectoryUrl(parent, caption, dir, options, supportedSchemes);
    
    return dirrectoryUrl;
}


QStringList FileDialog::getOpenFileNames(QWidget *parent, const QString &caption, const QString &dir,
                                    const QString &filter, QString *selectedFilter, Options options)
{
    auto fileNames = QFileDialog::getOpenFileNames(parent, caption, dir, filter, selectedFilter, options);
    
    return fileNames;
}


QList<QUrl> FileDialog::getOpenFileUrls(QWidget *parent, const QString &caption, const QUrl &dir,
                                   const QString &filter, QString *selectedFilter, Options options,
                                   const QStringList &supportedSchemes)
{
    auto fileUrls = QFileDialog::getOpenFileUrls(parent, caption, dir, filter, selectedFilter, options, supportedSchemes);
    
    return fileUrls;
}
