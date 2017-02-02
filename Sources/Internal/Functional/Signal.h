#pragma once

#include "Debug/DVAssert.h"

#include "Base/List.h"
#include "Base/Token.h"
#include "Functional/Function.h"
#include "Functional/TrackedObject.h"
#include "Functional/Private/SignalBase.h"

namespace DAVA
{
struct SignalConnection;

template <typename... Args>
class Signal final : protected SignalBase
{
public:
    using SlotFn = Function<void(Args...)>;

    Signal() = default;
    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;

    ~Signal();

    template <typename Obj, typename Fn>
    SignalConnection Connect(Obj* obj, const Fn& fn);

    template <typename Obj, typename Cls>
    SignalConnection Connect(Obj* obj, void (Cls::*const& fn)(Args...));

    template <typename Obj, typename Cls>
    SignalConnection Connect(Obj* obj, void (Cls::*const& fn)(Args...) const);

    template <typename Fn>
    SignalConnection ConnectDetached(const Fn& fn);

    void Disconnect(void* obj);

    void Disconnect(Token token) override;

    void DisconnectAll();

    void Track(Token token, TrackedObject* tracked) override;

    void Block(Token token, bool block);

    void Block(void* obj, bool block);

    bool IsBlocked(Token token) const;

    void Emit(Args... args);

private:
    struct Slot
    {
        Token token;
        SlotFn fn;

        void* object;
        TrackedObject* tracked;

        bool blocked;
        bool deleted;
    };

    List<Slot> slots;

    template <typename Obj>
    SignalConnection AddSlot(Obj* obj, SlotFn&& slotFn);

    void RemSlot(Slot& slot);

    void OnTrackedObjectDestroyed(TrackedObject* object) override;
};

struct SignalConnection final
{
    bool IsConnected() const;
    void Disconnect() const;
    void Track(TrackedObject*) const;

    mutable Token token;
    SignalBase* signal;
};

} // namespace DAVA

#define __DAVA_Signal__
#include "Functional/Private/Signal_impl.h"
