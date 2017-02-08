#pragma once

#include <TArc/Core/OperationRegistrator.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/DataProcessing/DataContext.h>

namespace QEGlobal
{
extern DAVA::TArc::WindowKey windowKey;
using IDList = DAVA::Set<DAVA::TArc::DataContext::ContextID>;

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

//try to reload documents by it ContextID
//if document does not exist - do nothingN
//if document is exist but have changes - ask user for permissions for this operation
//if user cancel this operation - nothing will happen
DECLARE_OPERATION_ID(ReloadDocuments);

//try to close documents by it ContextID
//if document does not exist - do nothing
//if document is exist but have changes - ask user for permissions for this operation
//if user cancel this operation - nothing will happen
DECLARE_OPERATION_ID(CloseDocuments);
}
