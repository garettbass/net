#pragma once
#include <new>


namespace net {


    /*==========================================================================
    T& assign(T* target, const T& source)

    Safely delegates copy assignment to existing destructor/copy constructor.
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

    Safely delegates move assignment to existing destructor/move constructor.
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
