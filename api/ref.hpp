// Copyright 2022-2023 The Jule Programming Language.
// Use of this source code is governed by a BSD 3-Clause
// license that can be found in the LICENSE file.

#ifndef __JULE_REF_HPP
#define __JULE_REF_HPP

#include <ostream>

#include "atomic.hpp"
#include "types.hpp"
#include "error.hpp"
#include "panic.hpp"

namespace jule {

    // The reference counting data delta value that must occur
    // per each reference counting operation.
    constexpr signed int REFERENCE_DELTA{ 1 };

    // Wrapper structure for raw pointer of JuleC.
    // This structure is the used by Jule references for reference-counting
    // and memory management.
    template<typename T>
    struct Ref;

    // Equavelent of Jule's new(T) call.
    template<typename T>
    inline jule::Ref<T> new_ref(void);

    // Equavelent of Jule's new(T, EXPR) call.
    template<typename T>
    inline jule::Ref<T> new_ref(const T &init);

    template<typename T>
    struct Ref {
        mutable T *alloc{ nullptr };
        mutable jule::Uint *ref{ nullptr };

        // Creates new reference from allocation and reference counting
        // allocation. Reference does not counted if reference count
        // allocation is null.
        static jule::Ref<T> make(T *ptr, jule::Uint *ref) {
            jule::Ref<T> buffer;
            buffer.alloc = ptr;
            buffer.ref = ref;
            return buffer;
        }

        // Creates new reference from allocation.
        // Allocates new allocation for reference counting data and
        // starts counting to jule::REFERENCE_DELTA.
        static jule::Ref<T> make(T *ptr) {
            jule::Ref<T> buffer;

#ifndef __JULE_DISABLE__REFERENCE_COUNTING
            buffer.ref = new (std::nothrow) jule::Uint;
            if (!buffer.ref)
                jule::panic(jule::ERROR_MEMORY_ALLOCATION_FAILED);

            *buffer.ref = jule::REFERENCE_DELTA;
#endif

            buffer.alloc = ptr;
            return buffer;
        }

        static jule::Ref<T> make(const T &instance, jule::Uint *ref) {
            jule::Ref<T> buffer;

            buffer.alloc = new (std::nothrow) T;
            if (!buffer.alloc)
                jule::panic(jule::ERROR_MEMORY_ALLOCATION_FAILED);

            *buffer.alloc = instance;
            buffer.ref = ref;
            return buffer;
        }

        static jule::Ref<T> make(const T &instance) {
#ifdef __JULE_DISABLE__REFERENCE_COUNTING
            return jule::Ref<T>::make(instance, nullptr);
#else
            jule::Uint *ref = new (std::nothrow) jule::Uint;
            if (!ref)
                jule::panic(jule::ERROR_MEMORY_ALLOCATION_FAILED);
            *ref = jule::REFERENCE_DELTA;

            return jule::Ref<T>::make(instance, ref);
#endif
        }

        Ref<T>(void) = default;
        Ref<T>(const std::nullptr_t&): Ref<T>() {}

        Ref<T> (const jule::Ref<T> &src)
        { this->__get_copy(src); }

        ~Ref<T>(void)
        { this->drop(); }

        // Copy content from source.
        void __get_copy(const jule::Ref<T> &src) {
            if (src.ref)
                src.add_ref();

            this->ref = src.ref;
            this->alloc = src.alloc;
        }

        inline jule::Int drop_ref(void) const {
            return __jule_atomic_add_explicit(
                this->ref,
                -jule::REFERENCE_DELTA,
                __JULE_ATOMIC_MEMORY_ORDER__RELAXED);
        }

        inline jule::Int add_ref(void) const {
            return __jule_atomic_add_explicit(
                this->ref,
                jule::REFERENCE_DELTA,
                __JULE_ATOMIC_MEMORY_ORDER__RELAXED);
        }

        inline jule::Uint get_ref_n(void) const {
            return __jule_atomic_load_explicit(
                this->ref, __JULE_ATOMIC_MEMORY_ORDER__RELAXED);
        }

        // Drops reference.
        // This function will destruct this instace for reference counting.
        // Frees memory if reference counting reaches to zero.
        void drop(void) const {
            if (!this->ref) {
                this->alloc = nullptr;
                return;
            }

            if (this->drop_ref() != jule::REFERENCE_DELTA) {
                this->ref = nullptr;
                this->alloc = nullptr;
                return;
            }

            delete this->ref;
            this->ref = nullptr;

            delete this->alloc;
            this->alloc = nullptr;
        }

        inline T *operator->(void) const {
#ifndef __JULE_DISABLE__SAFETY
            this->must_ok();
#endif
            return this->alloc;
        }

        inline T &operator*(void) const {
#ifndef __JULE_DISABLE__SAFETY
            this->must_ok();
#endif
            return *this->alloc;
        }

        inline operator T(void) const {
#ifndef __JULE_DISABLE__SAFETY
            this->must_ok();
#endif
            return *this->alloc;
        }

        inline operator T&(void) {
#ifndef __JULE_DISABLE__SAFETY
            this->must_ok();
#endif
            return *this->alloc;
        }

        // Returns data of allocation.
        inline T& get(void)
        { return this->operator T&(); }

        inline void must_ok(void) const {
            if (this->operator==(nullptr))
                jule::panic(jule::ERROR_INVALID_MEMORY);
        }

        void operator=(const jule::Ref<T> &src) {
            // Assignment to itself.
            if (this->alloc != nullptr && this->alloc == src.alloc)
                return;

            this->drop();
            this->__get_copy(src);
        }

        inline jule::Bool operator==(const std::nullptr_t&) const
        { return this->alloc == nullptr; }

        inline jule::Bool operator!=(const std::nullptr_t&) const
        { return !this->operator==(nullptr); }

        inline jule::Bool operator==(const jule::Ref<T> &ref) const
        { return this->alloc == ref.alloc; }

        inline jule::Bool operator!=(const jule::Ref<T> &ref) const
        { return !this->operator==(ref); }

        friend inline
        std::ostream &operator<<(std::ostream &stream,
                                 const jule::Ref<T> &ref) {
            if (ref == nullptr)
                stream << "nil";
            else
                stream << ref.operator T();
            return stream;
        }
    };

    template<typename T>
    inline jule::Ref<T> new_ref(void)
    { return jule::Ref<T>::make(T()); }

    template<typename T>
    inline jule::Ref<T> new_ref(const T &init) {
#ifdef __JULE_DISABLE__REFERENCE_COUNTING
        return jule::Ref<T>::make(init, nullptr);
#else
        return jule::Ref<T>::make(init);
#endif
    }

} // namespace jule

#endif // ifndef __JULE_REF_HPP
