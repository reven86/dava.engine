#include "ProjectView.h"
#include "QtTools/ReloadSprites/DialogReloadSprites.h"
#include "ui_mainwindow.h"
#include "Project/Project.h"

#include <QComboBox>
#include <QCheckBox>
#include <QLabel>

MainWindow::ProjectView::ProjectView(MainWindow* mainWindow_)
    : QObject(mainWindow_)
    , mainWindow(mainWindow_)
{
    InitPluginsToolBar();
    SetProjectActionsEnabled(false);

    connect(mainWindow->ui->actionReloadSprites, &QAction::triggered, this, &MainWindow::ProjectView::ReloadSprites);
    connect(mainWindow->ui->actionFindFileInProject, &QAction::triggered, this, &MainWindow::ProjectView::FindFileInProject);
    connect(mainWindow->ui->previewWidget, &PreviewWidget::SelectionChanged, mainWindow->ui->styleSheetInspectorWidget, &StyleSheetInspectorWidget::OnSelectionChanged);
    connect(mainWindow->ui->actionJumpToPrototype, &QAction::triggered, this, &MainWindow::ProjectView::JumpToPrototype);
    connect(mainWindow->ui->actionFindPrototypeInstances, &QAction::triggered, this, &MainWindow::ProjectView::FindPrototypeInstances);
    connect(mainWindow->ui->actionFindInDocument, &QAction::triggered, this, &MainWindow::ProjectView::ShowFindInDocument);
    connect(mainWindow->ui->actionFindNext, &QAction::triggered, mainWindow->ui->previewWidget, &PreviewWidget::OnFindNext);
    connect(mainWindow->ui->actionFindPrevious, &QAction::triggered, mainWindow->ui->previewWidget, &PreviewWidget::OnFindPrevious);

    connect(mainWindow->ui->previewWidget, &PreviewWidget::SelectionChanged, this, &MainWindow::ProjectView::OnSelectionChanged);

    connect(this, &MainWindow::ProjectView::ProjectChanged, mainWindow->ui->findResultsWidget, &FindResultsWidget::OnProjectChanged);

    mainWindow->ui->packageWidget->treeView->addAction(mainWindow->ui->actionJumpToPrototype);
    mainWindow->ui->packageWidget->treeView->addAction(mainWindow->ui->actionFindPrototypeInstances);
}

void MainWindow::ProjectView::SetLanguages(const QStringList& availableLangsCodes, const QString& currentLangCode)
{
    bool wasBlocked = comboboxLanguage->blockSignals(true); //performance fix
    comboboxLanguage->clear();

    if (!availableLangsCodes.isEmpty())
    {
        bool currentLangPresent = false;
        for (const QString& langCode : availableLangsCodes)
        {
            comboboxLanguage->addItem(ConvertLangCodeToString(langCode), langCode);
            if (langCode == currentLangCode)
            {
                currentLangPresent = true;
            }
        }
        DVASSERT(currentLangPresent);
        comboboxLanguage->setCurrentText(ConvertLangCodeToString(currentLangCode));
    }

    comboboxLanguage->blockSignals(wasBlocked);
}

void MainWindow::ProjectView::SetCurrentLanguage(const QString& currentLangCode)
{
    comboboxLanguage->setCurrentText(ConvertLangCodeToString(currentLangCode));
}

void MainWindow::ProjectView::SetProjectActionsEnabled(bool enabled)
{
    mainWindow->ui->actionCloseProject->setEnabled(enabled);
    mainWindow->ui->actionFindFileInProject->setEnabled(enabled);
    mainWindow->ui->actionJumpToPrototype->setEnabled(enabled);
    mainWindow->ui->actionFindPrototypeInstances->setEnabled(enabled);
    mainWindow->ui->actionFindInDocument->setEnabled(enabled);
    mainWindow->ui->actionFindNext->setEnabled(enabled);
    mainWindow->ui->actionFindPrevious->setEnabled(enabled);
    mainWindow->ui->toolBarPlugins->setEnabled(enabled);

    mainWindow->ui->fileSystemDockWidget->setEnabled(enabled);
}

MainWindow::DocumentGroupView* MainWindow::ProjectView::GetDocumentGroupView()
{
    return mainWindow->documentGroupView;
}

void MainWindow::ProjectView::InitPluginsToolBar()
{
    InitLanguageBox();
    InitGlobalClasses();
    InitRtlBox();
    InitBiDiSupportBox();
}

void MainWindow::ProjectView::InitLanguageBox()
{
    comboboxLanguage = new QComboBox(mainWindow);
    comboboxLanguage->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    QLabel* label = new QLabel(tr("Language"));
    label->setBuddy(comboboxLanguage);
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(label);
    layout->addWidget(comboboxLanguage);
    QWidget* wrapper = new QWidget();
    wrapper->setLayout(layout);
    mainWindow->ui->toolBarPlugins->addSeparator();
    mainWindow->ui->toolBarPlugins->addWidget(wrapper);

    void (QComboBox::*currentIndexChangedFn)(int) = &QComboBox::currentIndexChanged;
    connect(comboboxLanguage, currentIndexChangedFn, this, &MainWindow::ProjectView::OnCurrentLanguageChanged);
}

void MainWindow::ProjectView::InitRtlBox()
{
    QCheckBox* rtlBox = new QCheckBox(tr("Right-to-left"));
    rtlBox->setLayoutDirection(Qt::RightToLeft);
    mainWindow->ui->toolBarPlugins->addSeparator();
    mainWindow->ui->toolBarPlugins->addWidget(rtlBox);
    connect(rtlBox, &QCheckBox::stateChanged, this, &MainWindow::ProjectView::OnRtlChanged);
}

void MainWindow::ProjectView::InitBiDiSupportBox()
{
    QCheckBox* bidiSupportBox = new QCheckBox(tr("BiDi Support"));
    bidiSupportBox->setLayoutDirection(Qt::RightToLeft);
    mainWindow->ui->toolBarPlugins->addSeparator();
    mainWindow->ui->toolBarPlugins->addWidget(bidiSupportBox);
    connect(bidiSupportBox, &QCheckBox::stateChanged, this, &MainWindow::ProjectView::OnBiDiSupportChanged);
}

void MainWindow::ProjectView::InitGlobalClasses()
{
    QLineEdit* classesEdit = new QLineEdit();
    classesEdit->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    QLabel* label = new QLabel(tr("Global classes"));
    label->setBuddy(classesEdit);
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(label);
    layout->addWidget(classesEdit);
    QWidget* wrapper = new QWidget();
    wrapper->setLayout(layout);
    mainWindow->ui->toolBarPlugins->addSeparator();
    mainWindow->ui->toolBarPlugins->addWidget(wrapper);
    connect(classesEdit, &QLineEdit::textChanged, this, &MainWindow::ProjectView::OnGlobalClassesChanged);
}

void MainWindow::ProjectView::SetProjectPath(const QString& projectPath)
{
    mainWindow->SetProjectPath(projectPath);
}

void MainWindow::ProjectView::OnProjectChanged(Project* project)
{
    emit ProjectChanged(project);
}

void MainWindow::ProjectView::OnRtlChanged(int arg)
{
    emit RtlChanged(arg == Qt::Checked);
}

void MainWindow::ProjectView::OnBiDiSupportChanged(int arg)
{
    emit BiDiSupportChanged(arg == Qt::Checked);
}

void MainWindow::ProjectView::OnGlobalClassesChanged(const QString& str)
{
    emit GlobalStyleClassesChanged(str);
}

void MainWindow::ProjectView::OnCurrentLanguageChanged(int newLanguageIndex)
{
    QString langCode = comboboxLanguage->itemData(newLanguageIndex).toString();
    emit CurrentLanguageChanged(langCode);
}

void MainWindow::ProjectView::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    emit SelectionChanged(selected, deselected);
}

void MainWindow::ProjectView::SelectFile(const QString& filePath)
{
    mainWindow->ui->fileSystemDockWidget->SelectFile(filePath);
}

void MainWindow::ProjectView::SelectControl(const DAVA::String& controlPath)
{
    mainWindow->ui->previewWidget->SelectControl(controlPath);
}

void MainWindow::ProjectView::FindControls(std::unique_ptr<FindFilter>&& filter)
{
    mainWindow->ui->findResultsWidget->Find(std::move(filter));
}

void MainWindow::ProjectView::ShowFindInDocument()
{
    mainWindow->ui->previewWidget->OnFindInDocument();
}

void MainWindow::ProjectView::CancelFindInDocument()
{
    mainWindow->ui->previewWidget->OnCancelFind();
}

void MainWindow::ProjectView::SetResourceDirectory(const QString& path)
{
    mainWindow->ui->fileSystemDockWidget->SetResourceDirectory(path);
}

void MainWindow::ProjectView::ExecDialogReloadSprites(SpritesPacker* packer)
{
    DVASSERT(nullptr != packer);
    auto lastFlags = mainWindow->acceptableLoggerFlags;
    mainWindow->acceptableLoggerFlags = (1 << DAVA::Logger::LEVEL_ERROR) | (1 << DAVA::Logger::LEVEL_WARNING);
    DialogReloadSprites dialogReloadSprites(packer, mainWindow);
    dialogReloadSprites.exec();
    mainWindow->acceptableLoggerFlags = lastFlags;
}

QString MainWindow::ProjectView::ConvertLangCodeToString(const QString& langCode)
{
    QLocale locale(langCode);
    switch (locale.script())
    {
    case QLocale::SimplifiedChineseScript:
    {
        return "Chinese simpl.";
    }

    case QLocale::TraditionalChineseScript:
    {
        return "Chinese trad.";
    }

    default:
        return QLocale::languageToString(locale.language());
    }
}
