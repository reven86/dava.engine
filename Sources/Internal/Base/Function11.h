#ifndef __DAVA_FUNCTION11_H__
#define __DAVA_FUNCTION11_H__

#include <new>
#include <memory>
#include <array>
#include <type_traits>

#define FN11CC //__thiscall

namespace Fn11 {

class Closure
{
public:
    using Storage = std::array<void *, 4>;

    Closure() 
    { }

    ~Closure() 
    { 
        Clear();
    }

    template<typename Hldr, typename Fn, typename... Prms>
    void BindTrivial(const Fn& fn, Prms... params)
    {
        Clear();
        static_assert(sizeof(Hldr) <= sizeof(Storage), "Fn can't be trivially bind");
        new (storage.data()) Hldr(fn, std::forward<Prms>(params)...);
    }

    template<typename Hldr, typename Fn, typename... Prms>
    void BindShared(const Fn& fn, Prms... params)
    {
        Clear();
        Hldr *holder = new Hldr(fn, std::forward<Prms>(params)...);
        new (storage.data()) std::shared_ptr<void>(holder);
        shared = true;
    }

    template<typename Hldr>
    inline Hldr* GetShared() const
    {
        return static_cast<Hldr*>(SharedPtr()->get());
    }

    template<typename Hldr>
    inline Hldr* GetTrivial() const
    {
        static_assert(sizeof(Hldr) <= sizeof(Storage), "Fn can't be trivially get");
        return reinterpret_cast<Hldr *>((void *)storage.data());
    }

    bool IsEmpty() const
    {
        return (shared == false && storage[0] == nullptr);
    }

    Closure(const Closure& c)
    {
        Clear();
        Copy(c);
    }

    Closure(Closure&& c)
    {
        Clear();
        Copy(c);
        c.Clear();
    }

    Closure& operator=(const Closure &c)
    {
        if (this != &c)
            Copy(c);

        return *this;
    }

    Closure& operator=(Closure &&c)
    {
        if (this != &c)
        {
            shared = c.shared;
            storage = c.storage;
            c.Clear();
        }
    }

    Closure& operator=(nullptr_t)
    {
        Clear();
        return *this;
    }

protected:    
    bool shared = false;
    Storage storage;

    inline std::shared_ptr<void>* SharedPtr() const
    {
        return reinterpret_cast<std::shared_ptr<void>*>((void *)storage.data());
    }

    void Clear()
    {
        if (shared)
        {
            shared = false;
            SharedPtr()->reset();
        }
        storage.fill(nullptr);
    }

    void Copy(const Closure& c)
    {
        shared = c.shared;
        if (shared)
            new (storage.data()) std::shared_ptr<void>(*c.SharedPtr());
        else
            storage = c.storage;
    }
};

template<typename Fn, typename Ret, typename... Args>
class HolderFree
{
public:
    HolderFree(const Fn& _fn)
        : fn(_fn)
    { }

    static Ret FN11CC invokeTrivial(Closure& storage, Args&&... args)
    {
        HolderFree *holder = storage.GetTrivial<HolderFree>();
        return holder->fn(std::forward<Args>(args)...);
    }

    static Ret FN11CC invokeShared(Closure& storage, Args&&... args)
    {
        HolderFree *holder = storage.GetShared<HolderFree>();
        return holder->fn(std::forward<Args>(args)...);
    }

protected:
    Fn fn;
};

template<typename Ret, typename Cls, typename... ClsArgs>
class HolderClass
{
public:
    using Fn = Ret(Cls::*)(ClsArgs...);

    HolderClass(const Fn &_fn)
        : fn(_fn)
    { }

    static Ret FN11CC invokeTrivial(Closure &storage, Cls*&& cls, ClsArgs&&... args)
    {
        HolderClass *holder = storage.GetTrivial<HolderClass>();
        return (cls->*holder->fn)(std::forward<ClsArgs>(args)...);
    }

    static Ret FN11CC invokeShared(Closure &storage, Cls*&& cls, ClsArgs&&... args)
    {
        HolderClass *holder = storage.GetShared<HolderClass>();
        return (cls->*holder->fn)(std::forward<ClsArgs>(args)...);
    }

protected:
    Fn fn;
};

template<typename Ret, typename Obj, typename... ObjArgs>
class HolderObject
{
public:
    using Fn = Ret(Obj::*)(ObjArgs...);

    HolderObject(const Fn &_fn, Obj* _obj)
        : fn(_fn)
        , obj(_obj)
    { }

    static Ret FN11CC invokeTrivial(Closure &storage, ObjArgs&&... args)
    {
        HolderObject *holder = storage.GetTrivial<HolderObject>();
        return (holder->obj->*holder->fn)(std::forward<ObjArgs>(args)...);
    }

    static Ret FN11CC invokeShared(Closure &storage, ObjArgs&&... args)
    {
        HolderObject *holder = storage.GetShared<HolderObject>();
        return (holder->obj->*holder->fn)(std::forward<ObjArgs>(args)...);
    }

protected:
    Fn fn;
    Obj *obj;
};

template<typename Ret, typename Obj, typename... ObjArgs>
class HolderSharedObject
{
public:
    using Fn = Ret(Obj::*)(ObjArgs...);

    HolderSharedObject(const Fn& _fn, const std::shared_ptr<Obj> _obj)
        : fn(_fn)
        , obj(_obj)
    { }

    static Ret FN11CC invokeShared(Closure &storage, ObjArgs&&... args)
    {
        HolderSharedObject *holder = storage.GetShared<HolderSharedObject>();
        return (holder->obj->get()->*holder->fn)(std::forward<ObjArgs>(args)...);
    }

protected:
    Fn fn;
    std::shared_ptr<Obj> obj;
};

} // namespace Fn11

template<typename Fn>
class Function11;

template<typename Ret, typename... Args>
class Function11<Ret (Args...)>
{
public:
    Function11()
    { }

    Function11(nullptr_t)
    { }

    template<typename Fn>
    Function11(const Fn& fn)
    {
        static_assert(!std::is_member_function_pointer<Fn>::value, "There is no appropriate constructor for such Fn type.");
        using Holder = Fn11::HolderFree<Fn, Ret, Args...>;
        Init<Holder, Fn>(fn);
    }

    template<typename Cls, typename... ClsArgs>
    Function11(Ret (Cls::* const &fn)(ClsArgs...))
    {
        using Holder = Fn11::HolderClass<Ret, Cls, ClsArgs...>;
        using Fn = Ret(Cls::*)(ClsArgs...);
        Init<Holder, Fn>(fn);
    }

    template<typename Cls, typename... ClsArgs>
    Function11(Ret(Cls::* const &fn)(ClsArgs...) const)
    {
        using Holder = Fn11::HolderClass<Ret, Cls, ClsArgs...>;
        using Fn = Ret(Cls::*)(ClsArgs...);
        Init<Holder, Fn>(reinterpret_cast<Fn>(fn));
    }

    template<typename Obj, typename... ObjArgs>
    Function11(Obj *obj, Ret(Obj::* const &fn)(ObjArgs...))
    {
        using Holder = Fn11::HolderObject<Ret, Obj, ObjArgs...>;
        using Fn = Ret(Obj::*)(ObjArgs...);
        Init<Holder, Fn>(fn, obj);
    }

    template<typename Obj, typename... ObjArgs>
    Function11(Obj *obj, Ret(Obj::* const &fn)(ObjArgs...) const)
    {
        using Holder = Fn11::HolderObject<Ret, Obj, ObjArgs...>;
        using Fn = Ret(Obj::*)(ObjArgs...);
        Init<Holder, Fn>(reinterpret_cast<Fn>(fn), obj);
    }

    template<typename Obj, typename... ObjArgs>
    Function11(const std::shared_ptr<Obj> &obj, Ret(Obj::* const &fn)(ObjArgs...))
    {
        using Holder = Fn11::HolderSharedObject<Ret, Obj, ObjArgs...>;
        using Fn = Ret(Obj::*)(ObjArgs...);
        Init<Holder, Fn, false>(fn, obj);
    }

    template<typename Obj, typename... ObjArgs>
    Function11(const std::shared_ptr<Obj> &obj, Ret(Obj::* const &fn)(ObjArgs...) const)
    {
        using Holder = Fn11::HolderSharedObject < Ret, Obj, ObjArgs... > ;
        using Fn = Ret(Obj::*)(ObjArgs...);
        Init<Holder, Fn, false>(reinterpret_cast<Fn>(fn), obj);
    }

    Function11(const Function11& fn) throw()
        : invoker(fn.invoker)
        , closure(fn.closure)
    { }

    Function11(Function11&& fn) throw()
        : invoker(fn.invoker)
        , closure(std::move(fn.closure))
    { 
        fn.invoker = nullptr;
    }

    Function11& operator=(nullptr_t) throw()
    {
        invoker = nullptr;
        closure = nullptr;
        return *this;
    }

    Function11& operator=(const Function11 &fn) throw()
    {
        if (this != &fn)
        {
            closure = fn.closure;
            invoker = fn.invoker;
        }
        return *this;
    }

    Function11& operator=(Function11&& fn) throw()
    {
        if (this != fn)
        {
            closure = std::move(fn.closure);
            invoker = fn.invoker;
            fn.invoker = nullptr;
        }
        return *this;
    }

    bool operator==(nullptr_t) const throw()
    {
        return (closure.IsEmpty() && nullptr == invoker);
    }

    bool operator!=(nullptr_t) const throw()
    {
        return !operator==(nullptr);
    }

    operator bool() const throw()
    {
        return !operator==(nullptr);
    }

    inline Ret FN11CC operator()(Args... args) throw()
    {
        //return Fn11::HolderObject::invokeTrivial(*this, std::forward<Args>(args)...);
        return invoker(closure, std::forward<Args>(args)...);
    }

protected:
    using Invoker = Ret(FN11CC *)(Fn11::Closure &, Args&&...);

    Invoker invoker = nullptr;
    Fn11::Closure closure;

    template<typename Hldr, typename Fn, typename... Prms, bool trivial = true>
    void Init(const Fn& fn, Prms... params)
    {
        Detail<(trivial && sizeof(Hldr) <= sizeof(Fn11::Closure::Storage)
            && std::is_trivially_destructible<Fn>::value
            && std::is_trivially_copy_constructible<Fn>::value
            && std::is_trivially_copy_assignable<Fn>::value),
            Hldr, Fn, Prms... > ::Init(this, fn, std::forward<Prms>(params)...);
    }

private:
    template<bool trivial, typename Hldr, typename... Prms>
    struct Detail;

    // trivial specialization
    template<typename Hldr, typename Fn, typename... Prms>
    struct Detail<true, Hldr, Fn, Prms...>
    {
        static void Init(Function11 *that, const Fn& fn, Prms... params)
        {
            that->closure.template BindTrivial<Hldr, Fn>(fn, std::forward<Prms>(params)...);
            that->invoker = &Hldr::invokeTrivial;
        }
    };

    // shared specialization
    template<typename Hldr, typename Fn, typename... Prms>
    struct Detail<false, Hldr, Fn, Prms...>
    {
        static void Init(Function11 *that, const Fn& fn, Prms... params)
        {
            that->closure.template BindShared<Hldr, Fn>(fn, std::forward<Prms>(params)...);
            that->invoker = &Hldr::invokeShared;
        }
    };
};


#endif // __DAVA_FUNCTION11_H__
