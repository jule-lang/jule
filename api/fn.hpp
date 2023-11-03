// Copyright 2022-2023 The Jule Programming Language.
// Use of this source code is governed by a BSD 3-Clause
// license that can be found in the LICENSE file.

#ifndef __JULE_FN_HPP
#define __JULE_FN_HPP

#include <stddef.h>
#include <functional>
#include <thread>

#include "types.hpp"
#include "error.hpp"
#include "panic.hpp"

#define __JULE_CO_SPAWN(ROUTINE) \
    (std::thread{ROUTINE})
#define __JULE_CO(EXPR) \
    (__JULE_CO_SPAWN([&](void) mutable -> void { EXPR; }).detach())

namespace jule
{

    // std::function wrapper of JuleC.
    template <typename>
    struct Fn;

    template <typename T, typename... U>
    jule::Uintptr addr_of_fn(std::function<T(U...)> f) noexcept;

    template <typename Function>
    struct Fn
    {
    public:
        std::function<Function> buffer;
        jule::Uintptr _addr;

        Fn(void) = default;
        Fn(const Fn<Function> &fn) = default;
        Fn(std::nullptr_t) : Fn() {}

        Fn(const std::function<Function> &function) noexcept
        {
            this->_addr = jule::addr_of_fn(function);
            if (this->_addr == 0)
                this->_addr = (jule::Uintptr)(&function);
            this->buffer = function;
        }

        Fn(const Function *function) noexcept
        {
            this->buffer = function;
            this->_addr = jule::addr_of_fn(this->buffer);
            if (this->_addr == 0)
                this->_addr = (jule::Uintptr)(function);
        }

        template <typename... Arguments>
        auto operator()(Arguments... arguments)
        {
#ifndef __JULE_DISABLE__SAFETY
            if (this->buffer == nullptr)
                jule::panic(__JULE_ERROR__INVALID_MEMORY "\nfile: api/fn.hpp");
#endif
            return this->buffer(arguments...);
        }

        jule::Uintptr addr(void) const noexcept
        {
            return this->_addr;
        }

        inline void operator=(std::nullptr_t) noexcept
        {
            this->buffer = nullptr;
        }

        inline void operator=(const std::function<Function> &function)
        {
            this->buffer = function;
        }

        inline void operator=(const Function &function)
        {
            this->buffer = function;
        }

        inline jule::Bool operator==(const Fn<Function> &fn) const noexcept
        {
            return this->addr() == fn.addr();
        }

        inline jule::Bool operator!=(const Fn<Function> &fn) const noexcept
        {
            return !this->operator==(fn);
        }

        inline jule::Bool operator==(std::nullptr_t) const noexcept
        {
            return this->buffer == nullptr;
        }

        inline jule::Bool operator!=(std::nullptr_t) const noexcept
        {
            return !this->operator==(nullptr);
        }

        friend std::ostream &operator<<(std::ostream &stream,
                                        const Fn<Function> &src) noexcept
        {
            return (stream << (void *)src._addr);
        }
    };

    template <typename T, typename... U>
    jule::Uintptr addr_of_fn(std::function<T(U...)> f) noexcept
    {
        typedef T(FnType)(U...);
        FnType **fn_ptr = f.template target<FnType *>();
        if (!fn_ptr)
            return 0;
        return (jule::Uintptr)(*fn_ptr);
    }

} // namespace jule

#endif // ifndef __JULE_FN_HPP
