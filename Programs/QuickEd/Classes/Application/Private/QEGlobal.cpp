#include "Application/QEGlobal.h"

namespace QEGlobal
{
DAVA::TArc::WindowKey windowKey(DAVA::FastName("QuickEd"));

IMPL_OPERATION_ID(OpenLastProject);
IMPL_OPERATION_ID(OpenDocumentByPath);
IMPL_OPERATION_ID(CloseAllDocuments);
IMPL_OPERATION_ID(SelectFile);
IMPL_OPERATION_ID(SelectControl);
IMPL_OPERATION_ID(FindInProject);
}
