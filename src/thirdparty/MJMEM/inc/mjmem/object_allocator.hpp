// object_allocator.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _MJMEM_OBJECT_ALLOCATOR_HPP_
#define _MJMEM_OBJECT_ALLOCATOR_HPP_
#include <mjmem/allocator.hpp>
#include <new>
#include <type_traits>

namespace mjx {
    template <class _Ty>
    class object_allocator { // type-specific wrapper around global allocator
    public:
        using value_type      = _Ty;
        using size_type       = allocator::size_type;
        using difference_type = allocator::difference_type;
        using pointer         = _Ty*;
    
        object_allocator() noexcept                        = default;
        object_allocator(const object_allocator&) noexcept = default;
        object_allocator(object_allocator&&) noexcept      = default;
        ~object_allocator() noexcept                       = default;
    
        object_allocator& operator=(const object_allocator&) noexcept = default;
        object_allocator& operator=(object_allocator&&) noexcept      = default;

        pointer allocate(const size_type _Count) {
            return static_cast<pointer>(::mjx::get_allocator().allocate(_Count * sizeof(_Ty)));
        }

        pointer allocate_aligned(const size_type _Count, const size_type _Align) {
            return static_cast<pointer>(
                ::mjx::get_allocator().allocate_aligned(_Count * sizeof(_Ty), _Align));
        }

        void deallocate(pointer _Ptr, const size_type _Count) noexcept {
            ::mjx::get_allocator().deallocate(_Ptr, _Count * sizeof(_Ty));
        }

        size_type max_size() const noexcept {
            return ::mjx::get_allocator().max_size() / sizeof(_Ty);
        }

        bool is_equal(const object_allocator&) const noexcept {
            return true; // always equal
        }
    };

    template <class _Ty, class... _Types>
    inline _Ty* create_object(_Types&&... _Args) {
        object_allocator<_Ty> _Al;
        return ::new (static_cast<void*>(_Al.allocate(1))) _Ty(::std::forward<_Types>(_Args)...);
    }

    template <class _Ty>
    inline void delete_object(_Ty* const _Obj) noexcept(::std::is_nothrow_destructible_v<_Ty>) {
        if constexpr (!::std::is_trivially_destructible_v<_Ty>) {
            _Obj->~_Ty();
        }

        object_allocator<_Ty> _Al;
        _Al.deallocate(_Obj, 1);
    }
} // namespace mjx

#endif // _MJMEM_OBJECT_ALLOCATOR_HPP_