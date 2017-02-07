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
Signal<Args...>::Signal()
{
    mediumBackPos = connections.end();
}

template <typename... Args>
Signal<Args...>::~Signal()
{
    DisconnectAll();
}

template <typename... Args>
template <typename Fn>
inline SignalConnection Signal<Args...>::ConnectDetached(const Fn& fn, Group group)
{
    return AddSlot<void>(nullptr, ConnectionFn(fn), group);
}

template <typename... Args>
template <typename Obj, typename Fn>
inline SignalConnection Signal<Args...>::Connect(Obj* obj, const Fn& fn, Group group)
{
    return AddSlot(obj, ConnectionFn(fn), group);
}

template <typename... Args>
template <typename Obj, typename Cls>
inline SignalConnection Signal<Args...>::Connect(Obj* obj, void (Cls::*const& fn)(Args...), Group group)
{
    return AddSlot(obj, ConnectionFn(obj, fn), group);
}

template <typename... Args>
template <typename Obj, typename Cls>
inline SignalConnection Signal<Args...>::Connect(Obj* obj, void (Cls::*const& fn)(Args...) const, Group group)
{
    return AddSlot(obj, ConnectionFn(obj, fn), group);
}

template <typename... Args>
void Signal<Args...>::OnTrackedObjectDestroyed(TrackedObject* obj)
{
    Disconnect(obj);
}

template <typename... Args>
template <typename Obj>
SignalConnection Signal<Args...>::AddSlot(Obj* obj, ConnectionFn&& fn, Group group)
{
    Token token = SignalTokenProvider::Generate();

    Signal::Connection c;
    c.token = token;
    c.fn = std::move(fn);
    c.object = obj;
    c.tracked = SignalDetail::TrackedObjectCaster<Obj, std::is_base_of<TrackedObject, Obj>::value>::Cast(obj);

    if (nullptr != c.tracked)
    {
        Watch(c.tracked);
    }

    // now search a place for connection
    // depending on given group
    if (Group::High == group)
    {
        // for High priority just place it front
        connections.emplace_front(std::move(c));
    }
    else if (Group::Medium == group)
    {
        // for Medium insert in special tracked position
        connections.insert(mediumBackPos, std::move(c));
    }
    else
    {
        // for Low priority just place it back
        connections.emplace_back(std::move(c));

        // set new position for medium priority
        // it should be less than first low
        if (mediumBackPos == connections.end())
        {
            mediumBackPos--;
        }
    }

    return SignalConnection(this, token);
}

template <typename... Args>
void Signal<Args...>::RemSlot(Connection& c)
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
            RemSlot(c);
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
            RemSlot(c);
        }
    }
}

template <typename... Args>
void Signal<Args...>::DisconnectAll()
{
    for (auto& c : connections)
    {
        RemSlot(c);
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
    connections.remove_if([](const Connection& c) { return c.flags.test(Connection::Deleted); });

    for (auto& c : connections)
    {
        if (!c.flags.test(Connection::Blocked))
        {
            c.fn(args...);
        }
    }
}

inline SignalConnection::SignalConnection(SignalBase* signal_, Token token_)
    : signal(signal_)
    , token(token_)
{
}

inline SignalConnection::SignalConnection(SignalConnection&& sc)
    : signal(sc.signal)
    , token(sc.token)
{
    sc.token.Reset();
    sc.signal = nullptr;
}

inline bool SignalConnection::IsConnected() const
{
    return token.IsValid();
}

inline void SignalConnection::Disconnect() const
{
    DVASSERT(token.IsValid());
    DVASSERT(nullptr != signal);

    signal->Disconnect(token);
    token.Reset();
}

inline void SignalConnection::Track(TrackedObject* object) const
{
    DVASSERT(token.IsValid());
    DVASSERT(nullptr != signal);

    signal->Track(token, object);
}

inline SignalConnection::operator Token() const
{
    return token;
}

} // namespace DAVA
