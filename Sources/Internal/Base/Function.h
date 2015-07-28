#ifndef __DAVA_FUNCTION_H__
#define __DAVA_FUNCTION_H__

#include <new>
#include <memory>
#include <array>
#include <functional>
#include <type_traits>

#define FN_CALL_CONV /* __stdcall */

namespace DAVA {
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

    Closure(const Closure& c)
    {
        Copy(c);
    }

    Closure(Closure&& c)
    {
        Copy(c);

        c.shared = false;
        c.storage.fill(nullptr);
    }

    Closure& operator=(const Closure &c)
    {
        if (this != &c)
        {
            Clear();
            Copy(c);
        }

        return *this;
    }

    Closure& operator=(Closure &&c)
    {
        Clear();
        shared = c.shared;
        storage = c.storage;

        c.shared = false;
        c.storage.fill(nullptr);

        return *this;
    }

    Closure& operator=(std::nullptr_t)
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

struct Null {};

template<typename Fn, typename Ret, typename... Args>
class HolderFree
{
public:
    HolderFree(const Fn& _fn)
        : fn(_fn)
    { }

    static Ret FN_CALL_CONV invokeTrivial(const Closure& storage, Args&&... args)
    {
        HolderFree *holder = storage.GetTrivial<HolderFree>();
        return (Ret)holder->fn(std::forward<Args>(args)...);
    }

    static Ret FN_CALL_CONV invokeShared(const Closure& storage, Args&&... args)
    {
        HolderFree *holder = storage.GetShared<HolderFree>();
        return (Ret)holder->fn(std::forward<Args>(args)...);
    }

protected:
    Fn fn;
};

template<typename Obj, typename Ret, typename Cls, typename... ClsArgs>
class HolderClass
{
    static_assert(std::is_base_of<Cls, Obj>::value, "Specified class doesn't match class method");

public:
    using Fn = Ret(Cls::*)(ClsArgs...);

    HolderClass(const Fn& _fn)
        : fn(_fn)
    { }

    static Ret FN_CALL_CONV invokeTrivial(const Closure &storage, Obj*&& cls, ClsArgs&&... args)
    {
        HolderClass *holder = storage.GetTrivial<HolderClass>();
        return (Ret)(static_cast<Cls*>(cls)->*holder->fn)(std::forward<ClsArgs>(args)...);
    }

    static Ret FN_CALL_CONV invokeShared(const Closure &storage, Obj*&& cls, ClsArgs&&... args)
    {
        HolderClass *holder = storage.GetShared<HolderClass>();
        return (Ret)(static_cast<Cls*>(cls)->*holder->fn)(std::forward<ClsArgs>(args)...);
    }

protected:
    Fn fn;
};

template<typename Obj, typename Ret, typename Cls, typename... ClsArgs>
class HolderObject
{
    static_assert(std::is_base_of<Cls, Obj>::value, "Specified class doesn't match class method");

public:
    using Fn = Ret(Cls::*)(ClsArgs...);

    HolderObject(const Fn& _fn, Obj* _obj)
        : fn(_fn)
        , obj(_obj)
    { }

    static Ret FN_CALL_CONV invokeTrivial(const Closure &storage, ClsArgs&&... args)
    {
        HolderObject *holder = storage.GetTrivial<HolderObject>();
        return (Ret)(static_cast<Cls *>(holder->obj)->*holder->fn)(std::forward<ClsArgs>(args)...);
    }

    static Ret FN_CALL_CONV invokeShared(const Closure &storage, ClsArgs&&... args)
    {
        HolderObject *holder = storage.GetShared<HolderObject>();
        return (Ret)(static_cast<Cls *>(holder->obj)->*holder->fn)(std::forward<ClsArgs>(args)...);
    }

protected:
    Fn fn;
    Obj *obj;
};

template<typename Obj, typename Ret, typename Cls, typename... ClsArgs>
class HolderSharedObject
{
    static_assert(std::is_base_of<Cls, Obj>::value, "Specified class doesn't match class method");

public:
    using Fn = Ret(Cls::*)(ClsArgs...);

    HolderSharedObject(const Fn& _fn, const std::shared_ptr<Obj> _obj)
        : fn(_fn)
        , obj(_obj)
    { }

    static Ret FN_CALL_CONV invokeShared(const Closure &storage, ClsArgs&&... args)
    {
        HolderSharedObject *holder = storage.GetShared<HolderSharedObject>();
        return (static_cast<Cls*>(holder->obj.get())->*holder->fn)(std::forward<ClsArgs>(args)...);
    }

protected:
    Fn fn;
    std::shared_ptr<Obj> obj;
};

template<typename T, typename... Other>
struct first_pointer_class {
    typedef typename std::remove_pointer<T>::type type;
};

template<typename... Args>
using first_pointer_class_t = typename first_pointer_class<Args...>::type;

} // namespace Fn11

template<typename Fn>
class Function;

template<typename Ret, typename... Args>
class Function<Ret(Args...)>
{
    template<typename>
    friend class Function;

public:
    Function()
    { }

    Function(std::nullptr_t)
    { }

    template<typename Fn>
    Function(const Fn& fn)
    {
        static_assert(!std::is_member_function_pointer<Fn>::value, "There is no appropriate constructor for such Fn type.");
        using Holder = Fn11::HolderFree<Fn, Ret, Args...>;
        Init<Holder, Fn>(fn);
    }

    template<typename Cls, typename... ClsArgs>
    Function(Ret(Cls::* const &fn)(ClsArgs...))
    {
        using Holder = Fn11::HolderClass<Fn11::first_pointer_class_t<Args...>, Ret, Cls, ClsArgs...>;
        using Fn = Ret(Cls::*)(ClsArgs...);
        Init<Holder, Fn>(fn);
    }

    template<typename Cls, typename... ClsArgs>
    Function(Ret(Cls::* const &fn)(ClsArgs...) const)
    {
        using Holder = Fn11::HolderClass<Fn11::first_pointer_class_t<Args...>, Ret, Cls, ClsArgs...>;
        using Fn = Ret(Cls::*)(ClsArgs...);
        Init<Holder, Fn>(reinterpret_cast<Fn>(fn));
    }

    template<typename Obj, typename Cls, typename... ClsArgs>
    Function(Obj *obj, Ret(Cls::* const &fn)(ClsArgs...))
    {
        using Holder = Fn11::HolderObject<Obj, Ret, Cls, ClsArgs...>;
        using Fn = Ret(Cls::*)(ClsArgs...);
        Init<Holder, Fn>(fn, obj);
    }

    template<typename Obj, typename Cls, typename... ClsArgs>
    Function(Obj *obj, Ret(Cls::* const &fn)(ClsArgs...) const)
    {
        using Holder = Fn11::HolderObject<Obj, Ret, Cls, ClsArgs...>;
        using Fn = Ret(Cls::*)(ClsArgs...);
        Init<Holder, Fn>(reinterpret_cast<Fn>(fn), obj);
    }

    template<typename Obj, typename Cls, typename... ClsArgs>
    Function(const std::shared_ptr<Obj> &obj, Ret(Cls::* const &fn)(ClsArgs...))
    {
        using Holder = Fn11::HolderSharedObject<Obj, Ret, Cls, ClsArgs...>;
        using Fn = Ret(Cls::*)(ClsArgs...);

        // always use BindShared
        closure.template BindShared<Holder, Fn>(fn, obj);
        invoker = &Holder::invokeShared;
    }

    template<typename Obj, typename Cls, typename... ClsArgs>
    Function(const std::shared_ptr<Obj> &obj, Ret(Cls::* const &fn)(ClsArgs...) const)
    {
        using Holder = Fn11::HolderSharedObject<Obj, Ret, Cls, ClsArgs...>;
        using Fn = Ret(Cls::*)(ClsArgs...);

        // always use BindShared
        closure.template BindShared<Holder, Fn>(reinterpret_cast<Fn>(fn), obj);
        invoker = &Holder::invokeShared;
    }

    Function(const Function& fn) throw()
        : invoker(fn.invoker)
        , closure(fn.closure)
    { }

    Function(Function&& fn) throw()
        : invoker(fn.invoker)
        , closure(std::move(fn.closure))
    {
        fn.invoker = nullptr;
    }

    template<typename AnotherRet>
    Function(const Function<AnotherRet(Args...)> &fn)
        : invoker(fn.invoker)
        , closure(fn.closure)
    { }

    Function& operator=(std::nullptr_t) throw()
    {
        invoker = nullptr;
        closure = nullptr;
        return *this;
    }

    Function& operator=(const Function &fn) throw()
    {
        if (this != &fn)
        {
            closure = fn.closure;
            invoker = fn.invoker;
        }
        return *this;
    }

    Function& operator=(Function&& fn) throw()
    {
        closure = std::move(fn.closure);
        invoker = fn.invoker;
        fn.invoker = nullptr;
        return *this;
    }

    bool operator==(std::nullptr_t) const throw()
    {
        return (nullptr == invoker);
    }

    bool operator!=(std::nullptr_t) const throw()
    {
        return !operator==(nullptr);
    }

    operator bool() const throw()
    {
        return !operator==(nullptr);
    }

    inline Ret FN_CALL_CONV operator()(Args... args) const throw()
    {
        //return Fn11::HolderObject::invokeTrivial(*this, std::forward<Args>(args)...);
        return invoker(closure, std::forward<Args>(args)...);
    }

protected:
    using Invoker = Ret(FN_CALL_CONV *)(const Fn11::Closure &, Args&&...);

    Invoker invoker = nullptr;
    Fn11::Closure closure;

    template<typename Hldr, typename Fn, typename... Prms, bool trivial = true>
    void Init(const Fn& fn, Prms&&... params)
    {
        Detail<(trivial && sizeof(Hldr) <= sizeof(Fn11::Closure::Storage)
            && std::is_trivially_destructible<Fn>::value
            && std::is_trivially_copy_constructible<Fn>::value
            && std::is_trivially_copy_assignable<Fn>::value),
            Hldr, Fn, Prms... >::Init(this, fn, std::forward<Prms>(params)...);
    }

private:
    template<bool trivial, typename Hldr, typename... Prms>
    struct Detail;

    // trivial specialization
    template<typename Hldr, typename Fn, typename... Prms>
    struct Detail<true, Hldr, Fn, Prms...>
    {
        static void Init(Function *that, const Fn& fn, Prms&&... params)
        {
            that->closure.template BindTrivial<Hldr, Fn>(fn, std::forward<Prms>(params)...);
            that->invoker = &Hldr::invokeTrivial;
        }
    };

    // shared specialization
    template<typename Hldr, typename Fn, typename... Prms>
    struct Detail<false, Hldr, Fn, Prms...>
    {
        static void Init(Function *that, const Fn& fn, Prms&&... params)
        {
            that->closure.template BindShared<Hldr, Fn>(fn, std::forward<Prms>(params)...);
            that->invoker = &Hldr::invokeShared;
        }
    };
};

template<typename Ret, typename... Args>
Function<Ret(Args...)> MakeFunction(Ret(*const &fn)(Args...)) { return Function<Ret(Args...)>(fn); }

template<typename Cls, typename Ret, typename... Args>
Function<Ret(Cls*, Args...)> MakeFunction(Ret(Cls::* const &fn)(Args...)) { return Function<Ret(Cls*, Args...)>(fn); }

template<typename Cls, typename Ret, typename... Args>
Function<Ret(Cls*, Args...)> MakeFunction(Ret(Cls::* const &fn)(Args...) const) { return Function<Ret(Cls*, Args...)>(fn); }

template<typename Obj, typename Ret, typename... Args>
Function<Ret(Args...)> MakeFunction(Obj *obj, Ret(Obj::* const &fn)(Args...)) { return Function<Ret(Args...)>(obj, fn); }

template<typename Obj, typename Ret, typename... Args>
Function<Ret(Args...)> MakeFunction(Obj *obj, Ret(Obj::* const &fn)(Args...) const) { return Function<Ret(Args...)>(obj, fn); }

template<typename Obj, typename Ret, typename... Args>
Function<Ret(Args...)> MakeFunction(const std::shared_ptr<Obj> &obj, Ret(Obj::* const &fn)(Args...)) { return Function<Ret(Args...)>(obj, fn); }

template<typename Obj, typename Ret, typename... Args>
Function<Ret(Args...)> MakeFunction(const std::shared_ptr<Obj> &obj, Ret(Obj::* const &fn)(Args...) const) { return Function<Ret(Args...)>(obj, fn); }

template<typename Ret, typename... Args>
Function<Ret(Args...)> MakeFunction(const Function<Ret(Args...)>& fn) { return Function<Ret(Args...)>(fn); }

} // namespace DAVA

#endif // __DAVA_FUNCTION_H__
