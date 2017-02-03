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
    using ConnectionFn = Function<void(Args...)>;

    struct Connection
    {
        Token token;
        ConnectionFn fn;

        void* object;
        TrackedObject* tracked;

        bool blocked;
        bool deleted;
    };

    List<Connection> connections;

    template <typename Obj>
    SignalConnection AddSlot(Obj* obj, ConnectionFn&& fn);

    void RemSlot(Connection& slot);

    void OnTrackedObjectDestroyed(TrackedObject* object) override;
};

struct SignalConnection final
{
    SignalConnection(SignalBase*, Token);

    bool IsConnected() const;
    void Disconnect() const;
    void Track(TrackedObject*) const;

    operator Token() const;

private:
    mutable Token token;
    SignalBase* signal;
};

} // namespace DAVA

#define __DAVA_Signal__
#include "Functional/Private/Signal_impl.h"
