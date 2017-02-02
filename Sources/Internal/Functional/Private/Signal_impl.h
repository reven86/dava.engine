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
inline SignalConnection Signal<Args...>::ConnectDetached(const Fn& fn)
{
    return AddSlot<void>(nullptr, SlotFn(fn));
}

template <typename... Args>
template <typename Obj, typename Fn>
inline SignalConnection Signal<Args...>::Connect(Obj* obj, const Fn& fn)
{
    return AddSlot(obj, SlotFn(fn));
}

template <typename... Args>
template <typename Obj, typename Cls>
inline SignalConnection Signal<Args...>::Connect(Obj* obj, void (Cls::*const& fn)(Args...))
{
    return AddSlot(obj, SlotFn(obj, fn));
}

template <typename... Args>
template <typename Obj, typename Cls>
inline SignalConnection Signal<Args...>::Connect(Obj* obj, void (Cls::*const& fn)(Args...) const)
{
    return AddSlot(obj, SlotFn(obj, fn));
}

template <typename... Args>
void Signal<Args...>::OnTrackedObjectDestroyed(TrackedObject* obj)
{
    Disconnect(obj);
}

template <typename... Args>
template <typename Obj>
SignalConnection Signal<Args...>::AddSlot(Obj* obj, SlotFn&& fn)
{
    Token token = SignalTokenProvider::Generate();

    Signal::Slot slot;
    slot.token = token;
    slot.fn = std::move(fn);
    slot.blocked = false;
    slot.deleted = false;
    slot.object = obj;
    slot.tracked = SignalDetail::TrackedObjectCaster<Obj, std::is_base_of<TrackedObject, Obj>::value>::Cast(obj);

    if (nullptr != slot.tracked)
    {
        Watch(slot.tracked);
    }

    slots.emplace_back(std::move(slot));
    return SignalConnection({ token, this });
}

template <typename... Args>
void Signal<Args...>::RemSlot(Slot& slot)
{
    if (!slot.deleted)
    {
        if (nullptr != slot.tracked)
        {
            Unwatch(slot.tracked);
            slot.tracked = nullptr;
        }

        slot.object = nullptr;
        slot.deleted = true;
    }
}

template <typename... Args>
void Signal<Args...>::Disconnect(Token token)
{
    DVASSERT(SignalTokenProvider::IsValid(token));

    for (auto& slot : slots)
    {
        if (slot.token == token)
        {
            RemSlot(slot);
            break;
        }
    }
}

template <typename... Args>
void Signal<Args...>::Disconnect(void* obj)
{
    DVASSERT(nullptr != obj);

    for (auto& slot : slots)
    {
        if (slot.object == obj || slot.tracked == obj)
        {
            RemSlot(slot);
        }
    }
}

template <typename... Args>
void Signal<Args...>::DisconnectAll()
{
    for (auto& slot : slots)
    {
        RemSlot(slot);
    }
}

template <typename... Args>
void Signal<Args...>::Track(Token token, TrackedObject* tracked)
{
    DVASSERT(SignalTokenProvider::IsValid(token));
    DVASSERT(nullptr != tracked);

    auto i = slots.rbegin();
    auto rend = slots.rend();
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

    for (auto& slot : slots)
    {
        if (slot.token == token)
        {
            slot.blocked = block;
            break;
        }
    }
}

template <typename... Args>
void Signal<Args...>::Block(void* obj, bool block)
{
    for (auto& slot : slots)
    {
        if (slot.object == obj)
        {
            slot.blocked = block;
        }
    }
}

template <typename... Args>
bool Signal<Args...>::IsBlocked(Token token) const
{
    DVASSERT(SignalTokenProvider::IsValid(token));

    for (auto& slot : slots)
    {
        if (slot.token == token)
        {
            return slot.blocked;
        }
    }

    return false;
}

template <typename... Args>
void Signal<Args...>::Emit(Args... args)
{
    slots.remove_if([](const Slot& slot) { return slot.deleted; });

    for (auto& slot : slots)
    {
        if (!slot.blocked)
        {
            slot.fn(args...);
        }
    }
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

} // namespace DAVA
