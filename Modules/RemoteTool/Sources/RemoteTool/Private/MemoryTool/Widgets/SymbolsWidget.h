#pragma once

#include <Base/BaseTypes.h>

#include <QWidget>

class SymbolsListModel;
class SymbolsFilterModel;
class BacktraceSymbolTable;

class QListView;

class SymbolsWidget : public QWidget
{
    Q_OBJECT

public:
    SymbolsWidget(const BacktraceSymbolTable& symbolTable, QWidget* parent = nullptr);
    virtual ~SymbolsWidget();

    DAVA::Vector<const DAVA::String*> GetSelectedSymbols();

private:
    void Init();

private:
    const BacktraceSymbolTable& symbolTable;

    std::unique_ptr<SymbolsListModel> symbolListModel;
    std::unique_ptr<SymbolsFilterModel> symbolFilterModel;

    QListView* listWidget = nullptr;
};
