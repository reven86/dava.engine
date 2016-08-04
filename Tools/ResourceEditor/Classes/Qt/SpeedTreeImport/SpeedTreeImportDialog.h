#ifndef __RESOURCEEDITORQT__SPEEDTREEIMPORTDIALOG__
#define __RESOURCEEDITORQT__SPEEDTREEIMPORTDIALOG__

#include "DAVAEngine.h"
#include <QDialog>

namespace Ui
{
class QtTreeImportDialog;
}

class GlobalOperations;
class SpeedTreeImportDialog : public QDialog
{
    Q_OBJECT

public:
    SpeedTreeImportDialog(const std::shared_ptr<GlobalOperations>& globalOperations, QWidget* parent = 0);
    ~SpeedTreeImportDialog();

public slots:
    int exec();

private slots:
    void OnCancel();
    void OnOk();

    void OnXMLSelect();
    void OnSc2Select();

private:
    void SetSC2FolderValue(const QString& path);

    Ui::QtTreeImportDialog* ui;

    DAVA::Vector<DAVA::FilePath> xmlFiles;
    DAVA::FilePath sc2FolderPath;
    std::shared_ptr<GlobalOperations> globalOperations;
};

#endif // __RESOURCEEDITORQT__SPEEDTREEIMPORTDIALOG__
