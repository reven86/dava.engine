#pragma once

namespace DAVA
{
using SignalTokenProvider = TokenProvider<Signal<>>;

namespace SignalDetail
{
template <typename T, bool>
struct TrackedObjectCaster
{
    static TrackedObject* Cast(void* t)
    {
        return nullptr;
    }
};

template <typename T>
struct TrackedObjectCaster<T, true>
{
    static TrackedObject* Cast(T* obj)
    {
        return static_cast<TrackedObject*>(obj);
    }
};
} // namespace SignalDetail

template <typename... Args>
Signal<Args...>::~Signal()
{
    DisconnectAll();
}

template <typename... Args>
template <typename Fn>
inline Token Signal<Args...>::ConnectDetached(const Fn& fn)
{
    return AddConnection<void>(nullptr, Slot(fn));
}

template <typename... Args>
template <typename Obj, typename Fn>
inline Token Signal<Args...>::Connect(Obj* obj, const Fn& fn)
{
    return AddConnection(obj, Slot(fn));
}

template <typename... Args>
template <typename Obj, typename Cls>
inline Token Signal<Args...>::Connect(Obj* obj, void (Cls::*const& fn)(Args...))
{
    return AddConnection(obj, Slot(obj, fn));
}

template <typename... Args>
template <typename Obj, typename Cls>
inline Token Signal<Args...>::Connect(Obj* obj, void (Cls::*const& fn)(Args...) const)
{
    return AddConnection(obj, Slot(obj, fn));
}

template <typename... Args>
void Signal<Args...>::OnTrackedObjectDisconnect(TrackedObject* obj)
{
    Disconnect(obj);
}

template <typename... Args>
template <typename Obj>
Token Signal<Args...>::AddConnection(Obj* obj, Slot&& slot)
{
    Token token = SignalTokenProvider::Generate();

    Signal::Connection c;
    c.token = token;
    c.slot = std::move(slot);
    c.blocked = false;
    c.deleted = false;
    c.object = obj;
    c.tracked = SignalDetail::TrackedObjectCaster<Obj, std::is_base_of<TrackedObject, Obj>::value>::Cast(obj);

    if (nullptr != c.tracked)
    {
        Watch(c.tracked);
    }

    connections.emplace_back(std::move(c));
    return token;
}

template <typename... Args>
void Signal<Args...>::RemoveConnection(Connection& c)
{
    if (!c.deleted)
    {
        if (nullptr != c.tracked)
        {
            Unwatch(c.tracked);
            c.tracked = nullptr;
        }

        c.object = nullptr;
        c.deleted = true;
    }
}

template <typename... Args>
void Signal<Args...>::Disconnect(Token token)
{
    DVASSERT(SignalTokenProvider::IsValid(token));

    for (auto& c : connections)
    {
        if (c.token == token)
        {
            RemoveConnection(c);
            break;
        }
    }
}

template <typename... Args>
void Signal<Args...>::Disconnect(void* obj)
{
    DVASSERT(nullptr != obj);

    for (auto& c : connections)
    {
        if (c.object == obj)
        {
            RemoveConnection(c);
        }
    }
}

template <typename... Args>
void Signal<Args...>::DisconnectAll()
{
    for (auto& c : connections)
    {
        RemoveConnection(c);
    }
}

template <typename... Args>
void Signal<Args...>::Track(Token token, TrackedObject* tracked)
{
    DVASSERT(SignalTokenProvider::IsValid(token));
    DVASSERT(nullptr != tracked);

    auto i = connections.rbegin();
    auto rend = connections.rend();
    for (; i != rend; ++i)
    {
        if (i->token == token && i->tracked != tracked)
        {
            Unwatch(i->tracked);
            i->tracked = tracked;
            Watch(tracked);
        }
    }
}

template <typename... Args>
void Signal<Args...>::Block(Token token, bool block)
{
    DVASSERT(SignalTokenProvider::IsValid(token));

    for (auto& c : connections)
    {
        if (c.token == token)
        {
            c.blocked = block;
            break;
        }
    }
}

template <typename... Args>
void Signal<Args...>::Block(void* obj, bool block)
{
    for (auto& c : connections)
    {
        if (c.object == obj)
        {
            c.blocked = block;
        }
    }
}

template <typename... Args>
bool Signal<Args...>::IsBlocked(Token token) const
{
    DVASSERT(SignalTokenProvider::IsValid(token));

    for (auto& c : connections)
    {
        if (c.token == token)
        {
            return c.blocked;
        }
    }

    return false;
}

template <typename... Args>
void Signal<Args...>::Emit(Args... args)
{
    connections.remove_if([](const Connection& c) { return c.deleted; });

    for (auto& c : connections)
    {
        if (!c.blocked)
        {
            c.slot(args...);
        }
    }
}

} // namespace DAVA
