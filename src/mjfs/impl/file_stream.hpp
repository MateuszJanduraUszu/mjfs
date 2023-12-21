// file_stream.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _MJFS_IMPL_FILE_STREAM_HPP_
#define _MJFS_IMPL_FILE_STREAM_HPP_
#include <cstddef>
#include <cstdint>
#include <mjfs/impl/file.hpp>
#include <mjfs/impl/tinywin.hpp>
#include <mjstr/char_traits.hpp>

namespace mjx {
    namespace mjfs_impl {
        inline uint64_t _Tell_file(void* const _Handle) noexcept {
            LARGE_INTEGER _Pos = {0};
            return ::SetFilePointerEx(_Handle, _Pos, &_Pos, FILE_CURRENT) != 0
                ? static_cast<uint64_t>(_Pos.QuadPart) : 0;
        }

        inline size_t _Read_file(void* const _Handle, byte_t* const _Buf, const size_t _Count) noexcept {
            unsigned long _Read = 0;
#ifdef _M_X64
            return ::ReadFile(
                _Handle, _Buf, static_cast<unsigned long>(_Count), &_Read, nullptr) != 0 ? _Read : 0;
#else // ^^^ _M_X64 ^^^ / vvv _M_IX86 vvv
            return ::ReadFile(_Handle, _Buf, _Count, &_Read, nullptr) != 0 ? _Read : 0;
#endif // _M_X64
        }

        inline bool _Write_file(void* const _Handle, const byte_t* const _Data, const size_t _Count) noexcept {
            unsigned long _Written      = 0;
#ifdef _M_X64
            const unsigned long _UCount = static_cast<unsigned long>(_Count);
            return ::WriteFile(
                _Handle, _Data, _UCount, &_Written, nullptr) != 0 ? _Written == _UCount : false;
#else // ^^^ _M_X64 ^^^ / vvv _M_IX86 vvv
            return ::WriteFile(
                _Handle, _Data, _Count, &_Written, nullptr) != 0 ? _Written == _Count : false;
#endif // _M_X64
        }
    } // namespace mjfs_impl
} // namespace mjx

#endif // _MJFS_IMPL_FILE_STREAM_HPP_