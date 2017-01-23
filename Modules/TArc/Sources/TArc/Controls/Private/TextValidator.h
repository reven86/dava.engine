#pragma once

#include "TArc/Controls/Private/ValidatorDelegate.h"

#include <QValidator>

namespace DAVA
{
namespace TArc
{
class TextValidator : public QValidator
{
public:
    TextValidator(ValidatorDelegate* d, QObject* parent = nullptr);

    void fixup(QString& input) const override;
    State validate(QString& input, int& pos) const override;

private:
    ValidatorDelegate* d = nullptr;
};
} // namespace TArc
} // namespace DAVA