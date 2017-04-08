#pragma once

#include <TArc/Core/OperationRegistrator.h>
#include <TArc/WindowSubSystem/UI.h>

namespace QEGlobal
{
extern DAVA::TArc::WindowKey windowKey;

//open las opened project. If no one project was opened - do nothing
DECLARE_OPERATION_ID(OpenLastProject);

// if document by this path i already open -make it active
// if no opened documents with current path - creates new one and make it active
DECLARE_OPERATION_ID(OpenDocumentByPath);

// try to close all documents.
// if one or more documents has unsaved changes - request user for permissions of this operation
// if user cancel this operation- nothing will happen, so after this operation you always need to check contexts count
// ask y_rakhuba for extra details
DECLARE_OPERATION_ID(CloseAllDocuments);

//later we will make button to highlight file in fileSystem widget and this operation will be removed
//select file in FileSystemDockWidget by it path
DECLARE_OPERATION_ID(SelectFile);

//Select control by document name and control name
//If document was not open - opens it and than select control
DECLARE_OPERATION_ID(SelectControl);
}
