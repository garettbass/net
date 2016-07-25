#pragma once
#include <new>


namespace net {


    /*==========================================================================
    T& assign(T* target, const T& source)

    Delegates copy assignment to existing destructor/copy constructor.

    e.g. foo& operator=(const foo& lv) { return assign(this, lv); }
    --------------------------------------------------------------------------*/
    template <typename T>
    T& assign(T* target, const T& source) {
        if (target != &source) {
            target->~T();
            new(target)T(source);
        }
        return *target;
    }


    /*==========================================================================
    T& assign(T* target, T&& source)

    Delegates move assignment to existing destructor/move constructor.

    e.g. bar& operator=(bar&& rv) { return assign(this, std::move(rv)); }
    --------------------------------------------------------------------------*/
    template <typename T>
    T& assign(T* target, T&& source) {
        if (target != &source) {
            target->~T();
            new(target)T(std::move(source));
        }
        return *target;
    }


} // namespace net
