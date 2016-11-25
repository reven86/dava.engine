#include "QtTools/FileDialogs/FileDialog.h"

QString FileDialog::getOpenFileName(QWidget* parent, const QString& caption, const QString& dir,
                                    const QString& filter, QString* selectedFilter, Options options)
{
    auto fileName = QFileDialog::getOpenFileName(parent, caption, dir, filter, selectedFilter, options);

    return fileName;
}

QUrl FileDialog::getOpenFileUrl(QWidget* parent, const QString& caption, const QUrl& dir,
                                const QString& filter, QString* selectedFilter, Options options, const QStringList& supportedSchemes)
{
    auto fileUrl = QFileDialog::getOpenFileUrl(parent, caption, dir, filter, selectedFilter, options, supportedSchemes);

    return fileUrl;
}

QString FileDialog::getSaveFileName(QWidget* parent, const QString& caption, const QString& dir,
                                    const QString& filter, QString* selectedFilter, Options options)
{
    auto fileName = QFileDialog::getSaveFileName(parent, caption, dir, filter, selectedFilter, options);

    return fileName;
}

QUrl FileDialog::getSaveFileUrl(QWidget* parent, const QString& caption, const QUrl& dir,
                                const QString& filter, QString* selectedFilter, Options options,
                                const QStringList& supportedSchemes)
{
    auto fileUrl = QFileDialog::getSaveFileUrl(parent, caption, dir, filter, selectedFilter, options, supportedSchemes);

    return fileUrl;
}

QString FileDialog::getExistingDirectory(QWidget* parent, const QString& caption, const QString& dir, Options options)
{
    auto directory = QFileDialog::getExistingDirectory(parent, caption, dir, options);

    return directory;
}

QUrl FileDialog::getExistingDirectoryUrl(QWidget* parent, const QString& caption, const QUrl& dir,
                                         Options options, const QStringList& supportedSchemes)
{
    auto dirrectoryUrl = QFileDialog::getExistingDirectoryUrl(parent, caption, dir, options, supportedSchemes);

    return dirrectoryUrl;
}

QStringList FileDialog::getOpenFileNames(QWidget* parent, const QString& caption, const QString& dir,
                                         const QString& filter, QString* selectedFilter, Options options)
{
    auto fileNames = QFileDialog::getOpenFileNames(parent, caption, dir, filter, selectedFilter, options);

    return fileNames;
}

QList<QUrl> FileDialog::getOpenFileUrls(QWidget* parent, const QString& caption, const QUrl& dir,
                                        const QString& filter, QString* selectedFilter, Options options,
                                        const QStringList& supportedSchemes)
{
    auto fileUrls = QFileDialog::getOpenFileUrls(parent, caption, dir, filter, selectedFilter, options, supportedSchemes);

    return fileUrls;
}
