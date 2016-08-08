#pragma once

namespace tarc
{

class DataContext;
class ContextAccessor;

class ClientModule
{
protected:
    virtual ~ClientModule() {}

    virtual void OnContextCreated(DataContext& context) = 0;
    virtual void OnContextDeleted(DataContext& context) = 0;

    virtual void PostInit() = 0;
    ContextAccessor& GetAccessor();

private:
    void Init(ContextAccessor* contextAccessor);

private:
    friend class Core;

    ContextAccessor* contextAccessor = nullptr;
};

}
