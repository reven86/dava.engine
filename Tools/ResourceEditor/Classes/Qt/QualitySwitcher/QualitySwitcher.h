#ifndef __QUALITY_SWITCHER_H__
#define __QUALITY_SWITCHER_H__

#include <QDialog>
#include "Scene3D/Entity.h"

class QualitySwitcher : public QDialog
{
    Q_OBJECT

public:
    static void ShowDialog();

signals:
    void QualityChanged();
    void ParticlesQualityChanged();

protected:
    QualitySwitcher(QWidget* parent = nullptr);
    ~QualitySwitcher();

    void ApplyTx();
    void ApplyMa();

    void UpdateEntitiesToQuality(DAVA::Entity* e);
    void UpdateParticlesToQuality();
    void ReloadEntityEmitters(DAVA::Entity* e);
    void SetSettingsDirty(bool dirty);
    void ApplySettings();

protected slots:
    void OnTxQualitySelect(int index);
    void OnMaQualitySelect(int index);
    void OnOptionClick(bool);
    void OnParticlesQualityChanged(int index);
    void OnParticlesTagsCloudChanged(const QString& text);

    void OnOkPressed();
    void OnCancelPressed();
    void OnApplyPressed();

private:
    bool settingsDirty = false;
    static QualitySwitcher* switcherDialog;
};

#endif // __QUALITY_SWITCHER_H__
