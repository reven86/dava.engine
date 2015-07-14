#ifndef __DAVA_FUNCTION_H__
#define __DAVA_FUNCTION_H__

#include <new>
#include <memory>
#include <array>
#include <functional>
#include <type_traits>

#define FN_CALL_CONV __stdcall

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
        shared = c.shared;
        storage = c.storage;
        c.Clear();

        return *this;
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

    static Ret FN_CALL_CONV invokeTrivial(const Closure& storage, Args&&... args)
    {
        HolderFree *holder = storage.GetTrivial<HolderFree>();
        return (Ret) holder->fn(std::forward<Args>(args)...);
    }

    static Ret FN_CALL_CONV invokeShared(const Closure& storage, Args&&... args)
    {
        HolderFree *holder = storage.GetShared<HolderFree>();
        return (Ret) holder->fn(std::forward<Args>(args)...);
    }

protected:
    Fn fn;
};

template<typename Ret, typename Cls, typename... ClsArgs>
class HolderClass
{
public:
    using Fn = Ret(Cls::*)(ClsArgs...);

    template<typename T>
    HolderClass(Ret(T::* const &_fn)(ClsArgs...))
        : fn((Fn)_fn)
    { 
        static_assert(std::is_base_of<Cls, T>::value, "T should be derived from Cls");
    }

    static Ret FN_CALL_CONV invokeTrivial(const Closure &storage, Cls*&& cls, ClsArgs&&... args)
    {
        HolderClass *holder = storage.GetTrivial<HolderClass>();
        return (Ret) (cls->*holder->fn)(std::forward<ClsArgs>(args)...);
    }

    static Ret FN_CALL_CONV invokeShared(const Closure &storage, Cls*&& cls, ClsArgs&&... args)
    {
        HolderClass *holder = storage.GetShared<HolderClass>();
        return (Ret) (cls->*holder->fn)(std::forward<ClsArgs>(args)...);
    }

protected:
    Fn fn;
};

template<typename Ret, typename Obj, typename... ObjArgs>
class HolderObject
{
public:
    using Fn = Ret(Obj::*)(ObjArgs...);

    template<typename T>
    HolderObject(Ret(T::* const &_fn)(ObjArgs...), Obj* _obj)
        : fn((Fn)_fn)
        , obj(_obj)
    { 
        static_assert(std::is_base_of<Obj, T>::value, "T should be derived from Obj");
    }

    static Ret FN_CALL_CONV invokeTrivial(const Closure &storage, ObjArgs&&... args)
    {
        HolderObject *holder = storage.GetTrivial<HolderObject>();
        return (Ret) (holder->obj->*holder->fn)(std::forward<ObjArgs>(args)...);
    }

    static Ret FN_CALL_CONV invokeShared(const Closure &storage, ObjArgs&&... args)
    {
        HolderObject *holder = storage.GetShared<HolderObject>();
        return (Ret) (holder->obj->*holder->fn)(std::forward<ObjArgs>(args)...);
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

    template<typename T>
    HolderSharedObject(Ret(T::* const &_fn)(ObjArgs...), const std::shared_ptr<Obj> _obj)
        : fn(_fn)
        , obj(_obj)
    { 
        static_assert(std::is_base_of<Obj, T>::value, "T should be derived from Obj");
    }

    static Ret FN_CALL_CONV invokeShared(const Closure &storage, ObjArgs&&... args)
    {
        HolderSharedObject *holder = storage.GetShared<HolderSharedObject>();
        return (holder->obj.get()->*holder->fn)(std::forward<ObjArgs>(args)...);
    }

protected:
    Fn fn;
    std::shared_ptr<Obj> obj;
};

// checks if one list of arguments can be converted to another one
template<typename... AArgs>
struct ArgsConvertible : std::false_type { };

template<typename A0, typename B0>
struct ArgsConvertible < std::tuple<A0>, std::tuple<B0> >
{
    using A = std::conditional_t < std::is_pointer<A0>::value && std::is_pointer<B0>::value, std::remove_pointer_t<A0>, A0 > ;
    using B = std::conditional_t < std::is_pointer<A0>::value && std::is_pointer<B0>::value, std::remove_pointer_t<B0>, B0 > ;
    static const bool value = (std::is_same<A, B>::value || std::is_base_of<B, A>::value);
};

template<typename A0, typename... AOther, typename B0, typename... BOther>
struct ArgsConvertible < std::tuple<A0, AOther...>, std::tuple<B0, BOther...> >
{
    using A = std::conditional_t < std::is_pointer<A0>::value && std::is_pointer<B0>::value, std::remove_pointer_t<A0>, A0 > ;
    using B = std::conditional_t < std::is_pointer<A0>::value && std::is_pointer<B0>::value, std::remove_pointer_t<B0>, B0 > ;
    static const bool value = (std::is_same<A, B>::value || std::is_base_of<B, A>::value) && ArgsConvertible<std::tuple<AOther...>, std::tuple<BOther...>>::value;
};

} // namespace Fn11

template<typename Fn>
class Function;

template<typename Ret, typename... Args>
class Function<Ret (Args...)>
{
    template<typename>
    friend class Function;

public:
    Function()
    { }

    Function(nullptr_t)
    { }

    template<typename Fn>
    Function(const Fn& fn)
    {
        static_assert(!std::is_member_function_pointer<Fn>::value, "There is no appropriate constructor for such Fn type.");
        using Holder = Fn11::HolderFree<Fn, Ret, Args...>;
        Init<Holder, Fn>(fn);
    }

    template<typename Cls, typename... ClsArgs>
    Function(Ret (Cls::* const &fn)(ClsArgs...))
    {
        static_assert(Fn11::ArgsConvertible<std::tuple<Args...>, std::tuple<Cls*, ClsArgs...>>::value, "Unacceptable function arguments");

        using Holder = Fn11::HolderClass<Ret, Cls, ClsArgs...>;
        using Fn = Ret(Cls::*)(ClsArgs...);
        Init<Holder, Fn>(fn);
    }

    template<typename Cls, typename... ClsArgs>
    Function(Ret(Cls::* const &fn)(ClsArgs...) const)
    {
        static_assert(Fn11::ArgsConvertible<std::tuple<Args...>, std::tuple<Cls*, ClsArgs...>>::value, "Unacceptable function arguments");

        using Holder = Fn11::HolderClass<Ret, Cls, ClsArgs...>;
        using Fn = Ret(Cls::*)(ClsArgs...);
        Init<Holder, Fn>(reinterpret_cast<Fn>(fn));
    }

    template<typename Obj, typename Cls, typename... ClsArgs>
    Function(Obj *obj, Ret(Cls::* const &fn)(ClsArgs...))
    {
        static_assert(Fn11::ArgsConvertible<std::tuple<Obj*, Args...>, std::tuple<Cls*, ClsArgs...>>::value, "Unacceptable function arguments");

        using Holder = Fn11::HolderObject<Ret, Cls, ClsArgs...>;
        using Fn = Ret(Cls::*)(ClsArgs...);
        Init<Holder, Fn>(fn, obj);
    }

    template<typename Obj, typename Cls, typename... ClsArgs>
    Function(Obj *obj, Ret(Cls::* const &fn)(ClsArgs...) const)
    {
        static_assert(Fn11::ArgsConvertible<std::tuple<Obj*, Args...>, std::tuple<Cls*, ClsArgs...>>::value, "Unacceptable function arguments");

        using Holder = Fn11::HolderObject<Ret, Cls, ClsArgs...>;
        using Fn = Ret(Cls::*)(ClsArgs...);
        Init<Holder, Fn>(reinterpret_cast<Fn>(fn), obj);
    }

    template<typename Obj, typename... ObjArgs>
    Function(const std::shared_ptr<Obj> &obj, Ret(Obj::* const &fn)(ObjArgs...))
    {
        using Holder = Fn11::HolderSharedObject<Ret, Obj, ObjArgs...>;
        using Fn = Ret(Obj::*)(ObjArgs...);

        // always use BindShared
        closure.template BindShared<Holder, Fn>(fn, obj);
        invoker = &Holder::invokeShared;
    }

    template<typename Obj, typename... ObjArgs>
    Function(const std::shared_ptr<Obj> &obj, Ret(Obj::* const &fn)(ObjArgs...) const)
    {
        using Holder = Fn11::HolderSharedObject <Ret, Obj, ObjArgs...>;
        using Fn = Ret(Obj::*)(ObjArgs...);

        // always use BindShared
        closure.template BindShared<Holder, Fn>(fn, obj);
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

    Function& operator=(nullptr_t) throw()
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
        static void Init(Function *that, const Fn& fn, Prms... params)
        {
            that->closure.template BindTrivial<Hldr, Fn>(fn, std::forward<Prms>(params)...);
            that->invoker = reinterpret_cast<Invoker>(&Hldr::invokeTrivial);
        }
    };

    // shared specialization
    template<typename Hldr, typename Fn, typename... Prms>
    struct Detail<false, Hldr, Fn, Prms...>
    {
        static void Init(Function *that, const Fn& fn, Prms... params)
        {
            that->closure.template BindShared<Hldr, Fn>(fn, std::forward<Prms>(params)...);
            that->invoker = reinterpret_cast<Invoker>(&Hldr::invokeShared);
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
