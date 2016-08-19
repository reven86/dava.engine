#pragma once

namespace tarc
{

class UI;
class DataContext;
class ContextAccessor;

class ClientModule
{
public:
    virtual ~ClientModule() {}

protected:
    virtual void OnContextCreated(DataContext& context) = 0;
    virtual void OnContextDeleted(DataContext& context) = 0;

    virtual void PostInit() = 0;
    ContextAccessor& GetAccessor();
    UI& GetUI();

private:
    void Init(ContextAccessor* contextAccessor, UI* ui);

private:
    friend class Core;

    ContextAccessor* contextAccessor = nullptr;
    UI* ui = nullptr;
};

}
