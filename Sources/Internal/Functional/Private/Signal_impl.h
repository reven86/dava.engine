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
    static const bool is_tracked = std::is_base_of<TrackedObject, Obj>::value;
    Token token = SignalTokenProvider::Generate();

    Signal::Connection c;
    c.token = token;
    c.slot = std::move(slot);
    c.object = obj;
    c.blocked = false;
    c.deleted = false;
    c.tracked = is_tracked;

    if (nullptr != obj && is_tracked)
    {
        TrackedObject* to = SignalDetail::TrackedObjectCaster<Obj, is_tracked>::Cast(obj);
        Watch(to);
    }

    connections.emplace_back(std::move(c));
    return token;
}

template <typename... Args>
void Signal<Args...>::RemoveConnection(Connection& c)
{
    if (!c.deleted)
    {
        if (c.tracked)
        {
            TrackedObject* to = static_cast<TrackedObject*>(c.object);
            Unwatch(to);
        }

        c.object = nullptr;
        c.tracked = false;
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
            break;
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
void Signal<Args...>::Track(Token token, TrackedObject* obj)
{
    DVASSERT(nullptr != obj);
    DVASSERT(SignalTokenProvider::IsValid(token));

    auto i = connections.rbegin();
    auto rend = connections.rend();
    for (; i != rend; ++i)
    {
        if (!i->deleted && i->token == token)
        {
            RemoveConnection(*i);

            i->object = obj;
            i->tracked = true;
            i->deleted = false;

            Watch(obj);
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
