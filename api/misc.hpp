// Copyright 2023 The Jule Programming Language.
// Use of this source code is governed by a BSD 3-Clause
// license that can be found in the LICENSE file.

#ifndef __JULE_MISC_HPP
#define __JULE_MISC_HPP

#include "error.hpp"
#include "panic.hpp"
#include "ptr.hpp"

namespace jule
{
    template <typename T, typename Denominator>
    inline auto div(const T &x, const Denominator &denominator) noexcept;

    template <typename T, typename Denominator>
    inline auto mod(const T &x, const Denominator &denominator) noexcept;

    template <typename T, typename Denominator>
    inline auto unsafe_div(const T &x, const Denominator &denominator) noexcept;

    template <typename T, typename Denominator>
    inline auto unsafe_mod(const T &x, const Denominator &denominator) noexcept;

    template <typename T>
    jule::Ptr<T> new_struct(T *ptr) noexcept;
    template <typename T>
    jule::Ptr<T> new_struct_ptr(T *ptr) noexcept;

    // Dispose mask for implement dispose functionality.
    // It's also built-in Dispose trait.
    struct Dispose
    {
        virtual void _method_dispose(void) = 0;
    };

    template <typename T, typename Denominator>
    inline auto div(const T &x, const Denominator &denominator) noexcept
    {
#ifndef __JULE_DISABLE__SAFETY
        if (denominator == 0)
            jule::panic(__JULE_ERROR__DIVIDE_BY_ZERO "\nruntime: divide-by-zero occurred when division");
#endif
        return x / denominator;
    }

    template <typename T, typename Denominator>
    inline auto mod(const T &x, const Denominator &denominator) noexcept
    {
#ifndef __JULE_DISABLE__SAFETY
        if (denominator == 0)
            jule::panic(__JULE_ERROR__DIVIDE_BY_ZERO "\nruntime: divide-by-zero occurred when modulo");
#endif
        return x % denominator;
    }

    template <typename T, typename Denominator>
    inline auto unsafe_div(const T &x, const Denominator &denominator) noexcept
    {
        return x / denominator;
    }

    template <typename T, typename Denominator>
    inline auto unsafe_mod(const T &x, const Denominator &denominator) noexcept
    {
        return x % denominator;
    }

    template <typename T>
    jule::Ptr<T> new_struct(T *ptr) noexcept
    {
        if (!ptr)
            jule::panic(__JULE_ERROR__MEMORY_ALLOCATION_FAILED "\nruntime: allocation failed for structure");

#ifndef __JULE_DISABLE__REFERENCE_COUNTING
        return jule::Ptr<T>::make(ptr);
#endif

        return jule::Ptr<T>::make(ptr, nullptr);
    }

    template <typename T>
    jule::Ptr<T> new_struct_ptr(T *ptr) noexcept
    {
        if (!ptr)
            jule::panic(__JULE_ERROR__MEMORY_ALLOCATION_FAILED "\nruntime: allocation failed for structure");

#ifndef __JULE_DISABLE__REFERENCE_COUNTING
        ptr->self.ref = new (std::nothrow) jule::Uint;
        if (!ptr->self.ref)
            jule::panic(__JULE_ERROR__MEMORY_ALLOCATION_FAILED "\nruntime: allocation failed for structure");

        // Initialize with zero because return reference is counts 1 reference.
        *ptr->self.ref = 0; // ( jule::REFERENCE_DELTA - jule::REFERENCE_DELTA );
#endif

        return ptr->self;
    }
} // namespace jule

#endif // ifndef __JULE_MISC_HPP
