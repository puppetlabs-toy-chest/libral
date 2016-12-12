/**
 * @file
 * Declares the cast helpers.
 * Taken from puppetcpp
 */
#pragma once

#include <type_traits>

namespace libral {

    /**
     * Casts the given type to an rvalue reference.
     * Use this over std::move to check that move semantics are actually achieved.
     * Use of this function on a const reference will result in a compilation error.
     * @tparam T The underlying type.
     * @param t The reference to cast.
     * @return Returns the rvalue reference.
     */
    template <typename T>
    typename std::remove_reference<T>::type&& rvalue_cast(T&& t)
    {
        static_assert(
            !std::is_const<typename std::remove_reference<T>::type>::value,
            "Cannot rvalue-cast from a const reference as this will not invoke move semantics."
        );
        static_assert(
            std::is_move_constructible<typename std::remove_reference<T>::type>::value ||
            std::is_move_assignable<typename std::remove_reference<T>::type>::value,
            "Cannot rvalue-cast because the type is not move constructible or assignable."
        );
        return std::move(t);
    }

}  // libral
