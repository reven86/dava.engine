#pragma once

#include "Functional/Function.h"

namespace DAVA
{
/** function that calls member function only if corresponding object is still alive
    `memberFn` is a functor to object member function
    `objectWeak` is a weak pointer to object

    Usage:

    \code
    class A
    {
        void F(int);
    }
    shared_ptr<A> a;
    weak_ptr<A> wptr(a);
    Function<void(A*> fn = Bind(&A::F, _1, 10); // binding all params except pointer to object
    ....
    ....
    SafeMemberFnCaller(fn, wptr); // inside of it A::F will be called if A is still alive
    \endcode
*/
template <typename T>
void SafeMemberFnCaller(Function<void(T*)> memberFn, std::weak_ptr<T> objectWeak)
{
    std::shared_ptr<T> objectShared = objectWeak.lock();
    if (objectShared)
    {
        memberFn(objectShared.get());
    }
}
}
