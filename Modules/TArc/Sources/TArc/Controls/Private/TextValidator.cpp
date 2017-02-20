#include "TArc/Controls/Private/TextValidator.h"

#include <Debug/DVAssert.h>

namespace DAVA
{
namespace TArc
{
TextValidator::TextValidator(ValidatorDelegate* d_, QObject* parent)
    : QValidator(parent)
    , d(d_)
{
    DVASSERT(d != nullptr);
}

void TextValidator::fixup(QString& input) const
{
    Any inputValue(input.toStdString());
    M::ValidationResult result = d->FixUp(inputValue);
    if (!result.fixedValue.IsEmpty())
    {
        input = QString::fromStdString(result.fixedValue.Cast<String>());
    }
}

QValidator::State TextValidator::validate(QString& input, int& pos) const
{
    Any inputValue(input.toStdString());
    M::ValidationResult result = d->Validate(inputValue);
    if (!result.fixedValue.IsEmpty())
    {
        input = QString::fromStdString(result.fixedValue.Cast<String>());
    }

    if (!result.message.empty())
    {
        d->ShowHint(QString::fromStdString(result.message));
    }

    switch (result.state)
    {
    case M::ValidationResult::eState::Valid:
        return QValidator::Acceptable;
    case M::ValidationResult::eState::Intermediate:
        return QValidator::Intermediate;
    default:
        return QValidator::Invalid;
    }
}

} // namespace TArc
} // namespace DAVA