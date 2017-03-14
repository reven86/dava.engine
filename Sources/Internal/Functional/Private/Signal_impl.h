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

template <typename T>
TrackedObject* GetTrackedObject(T* obj)
{
    return TrackedObjectCaster<T, std::is_base_of<TrackedObject, T>::value>::Cast(obj);
}
} // namespace SignalDetail

template <typename... Args>
Signal<Args...>::Signal()
{
    connectionsMediumPos = connections.end();
}

template <typename... Args>
Signal<Args...>::~Signal()
{
    DisconnectAll();
}

template <typename... Args>
template <typename Fn>
inline Token Signal<Args...>::Connect(const Fn& fn, Group group)
{
    Token token = SignalTokenProvider::Generate();

    Signal::Connection c;
    c.fn = ConnectionFn(fn);
    c.object = nullptr;
    c.token = token;
    c.tracked = nullptr;
    AddSlot(std::move(c), group);

    return token;
}

template <typename... Args>
template <typename Obj, typename Fn>
inline Token Signal<Args...>::Connect(Obj* obj, const Fn& fn, Group group)
{
    Token token = SignalTokenProvider::Generate();

    Signal::Connection c;
    c.fn = ConnectionFn(fn);
    c.object = obj;
    c.token = token;
    c.tracked = SignalDetail::GetTrackedObject(obj);
    AddSlot(std::move(c), group);

    return token;
}

template <typename... Args>
template <typename Obj, typename Cls>
inline Token Signal<Args...>::Connect(Obj* obj, void (Cls::*const& fn)(Args...), Group group)
{
    Token token = SignalTokenProvider::Generate();

    Signal::Connection c;
    c.fn = ConnectionFn(obj, fn);
    c.object = obj;
    c.token = token;
    c.tracked = SignalDetail::GetTrackedObject(obj);
    AddSlot(std::move(c), group);

    return token;
}

template <typename... Args>
template <typename Obj, typename Cls>
inline Token Signal<Args...>::Connect(Obj* obj, void (Cls::*const& fn)(Args...) const, Group group)
{
    Token token = SignalTokenProvider::Generate();

    Signal::Connection c;
    c.fn = ConnectionFn(obj, fn);
    c.object = obj;
    c.token = token;
    c.tracked = SignalDetail::GetTrackedObject(obj);
    AddSlot(std::move(c), group);

    return token;
}

template <typename... Args>
void Signal<Args...>::OnTrackedObjectDestroyed(TrackedObject* obj)
{
    Disconnect(obj);
}

template <typename... Args>
void Signal<Args...>::AddSlot(Connection&& c, Group group)
{
    if (nullptr != c.tracked)
    {
        Watch(c.tracked);
    }

    // now search a place for connection
    // depending on given group
    if (Group::High == group)
    {
        // for High priority just place it front
        connections.push_front(std::move(c));
    }
    else if (Group::Medium == group)
    {
        // for Medium insert in special tracked position
        connections.insert(connectionsMediumPos, std::move(c));
    }
    else
    {
        // for Low priority just place it back
        connections.push_back(std::move(c));

        // set new position for medium priority
        // it should be less than first low
        if (connectionsMediumPos == connections.end())
        {
            connectionsMediumPos--;
        }
    }
}

template <typename... Args>
void Signal<Args...>::RemoveSlot(Connection& c)
{
    if (!c.flags.test(Connection::Deleted))
    {
        if (nullptr != c.tracked)
        {
            Unwatch(c.tracked);
            c.tracked = nullptr;
        }

        c.object = nullptr;
        c.flags.set(Connection::Deleted, true);
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
            RemoveSlot(c);
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
        if (c.object == obj || c.tracked == obj)
        {
            RemoveSlot(c);
        }
    }
}

template <typename... Args>
void Signal<Args...>::DisconnectAll()
{
    for (auto& c : connections)
    {
        RemoveSlot(c);
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
            if (nullptr != i->tracked)
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
            c.flags.set(Connection::Blocked, block);
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
        return c.flags.test(Connection::Blocked);
    }

    return false;
}

template <typename... Args>
void Signal<Args...>::Emit(Args... args)
{
    bool hasDeletedSlots = false;

    for (auto& c : connections)
    {
        if (!c.flags.test(Connection::Deleted))
        {
            if (!c.flags.test(Connection::Blocked))
            {
                c.fn(args...);
            }
        }
        else
        {
            hasDeletedSlots = true;
        }
    }

    if (hasDeletedSlots)
    {
        connections.remove_if([](const Connection& c) { return c.flags.test(Connection::Deleted); });
    }
}
} // namespace DAVA
