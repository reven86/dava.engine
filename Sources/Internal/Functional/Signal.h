#pragma once

#include "Debug/DVAssert.h"

#include "Base/List.h"
#include "Base/Token.h"
#include "Functional/Function.h"
#include "Functional/TrackedObject.h"
#include "Functional/Private/TrackedWatcher.h"

namespace DAVA
{
template <typename... Args>
class Signal final : protected TrackedWatcher
{
public:
    using Slot = Function<void(Args...)>;

    Signal() = default;
    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;

    ~Signal();

    template <typename Obj, typename Fn>
    Token Connect(Obj* obj, const Fn& fn);

    template <typename Obj, typename Cls>
    Token Connect(Obj* obj, void (Cls::*const& fn)(Args...));

    template <typename Obj, typename Cls>
    Token Connect(Obj* obj, void (Cls::*const& fn)(Args...) const);

    template <typename Fn>
    Token ConnectDetached(const Fn& fn);

    void Disconnect(void* obj);

    void Disconnect(Token token);

    void DisconnectAll();

    void Track(Token token, TrackedObject* tracked);

    void Block(Token token, bool block);

    void Block(void* obj, bool block);

    bool IsBlocked(Token token) const;

    void Emit(Args... args);

private:
    void OnTrackedObjectDisconnect(TrackedObject*) override final;

    struct Connection
    {
        Token token;

        void* object;
        TrackedObject* tracked;

        Slot slot;

        bool blocked;
        bool deleted;
    };

    List<Connection> connections;

    template <typename Obj>
    Token AddConnection(Obj* obj, Slot&& slot);

    void RemoveConnection(Connection& c);
};

} // namespace DAVA

#define __DAVA_Signal__
#include "Functional/Private/Signal_impl.h"
