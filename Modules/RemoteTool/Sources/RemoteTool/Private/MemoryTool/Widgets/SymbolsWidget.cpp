#include "RemoteTool/Private/MemoryTool/Widgets/SymbolsWidget.h"
#include "RemoteTool/Private/MemoryTool/BacktraceSymbolTable.h"
#include "RemoteTool/Private/MemoryTool/Models/SymbolsListModel.h"

#include <QListView>
#include <QLineEdit>
#include <QVBoxLayout>

using namespace DAVA;

SymbolsWidget::SymbolsWidget(const BacktraceSymbolTable& symbolTable_, QWidget* parent)
    : QWidget(parent)
    , symbolTable(symbolTable_)
{
    Init();
}

SymbolsWidget::~SymbolsWidget() = default;

Vector<const String*> SymbolsWidget::GetSelectedSymbols()
{
    Vector<const String*> result;
    QItemSelectionModel* selectionModel = listWidget->selectionModel();
    if (selectionModel->hasSelection())
    {
        QModelIndexList indexList = selectionModel->selectedRows();
        result.reserve(indexList.size());
        for (const QModelIndex& i : indexList)
        {
            QVariant v = symbolFilterModel->data(i, SymbolsListModel::ROLE_SYMBOL_POINTER);
            const String* name = static_cast<const String*>(v.value<void*>());
            Q_ASSERT(name != nullptr);
            result.push_back(name);
        }
    }
    return result;
}

void SymbolsWidget::Init()
{
    symbolListModel.reset(new SymbolsListModel(symbolTable));
    symbolFilterModel.reset(new SymbolsFilterModel(symbolListModel.get()));
    symbolFilterModel->sort(0);

    listWidget = new QListView;
    listWidget->setFont(QFont("Consolas", 10, 500));
    listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    listWidget->setModel(symbolFilterModel.get());

    QLineEdit* filterWidget = new QLineEdit;

    connect(filterWidget, &QLineEdit::textChanged, symbolFilterModel.get(), &SymbolsFilterModel::SetFilterString);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(filterWidget);
    layout->addWidget(listWidget);

    setLayout(layout);
}
