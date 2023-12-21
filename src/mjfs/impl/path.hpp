// path.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _MJFS_IMPL_PATH_HPP_
#define _MJFS_IMPL_PATH_HPP_
#include <cstddef>
#include <mjfs/impl/tinywin.hpp>
#include <mjstr/string_view.hpp>

namespace mjx {
    namespace mjfs_impl {
        inline bool _Is_slash(const wchar_t _Ch) noexcept {
            return _Ch == L'\\' || _Ch == L'/';
        }

        inline bool _Is_dot_or_dot_dot(const unicode_string_view _Str) noexcept {
            return _Str == L"." || _Str == L"..";
        }

        inline size_t _Find_first_slash(const unicode_string_view _Str) noexcept {
            const wchar_t* const _Data = _Str.data();
            const wchar_t* _First      = _Data;
            const wchar_t* const _Last = _First + _Str.size();
            for (; _First != _Last; ++_First) {
                if (_Is_slash(*_First)) {
                    return _First - _Data;
                }
            }

            return unicode_string_view::npos; // not found
        }

        inline size_t _Find_last_slash(const unicode_string_view _Str) noexcept {
            const wchar_t* const _First = _Str.data();
            const wchar_t* _Last        = _First + _Str.size();
            for (; _Last != _First - 1; --_Last) {
                if (_Is_slash(*_Last)) {
                    return _Last - _First;
                }
            }

            return unicode_string_view::npos; // not found
        }

        inline bool _Is_drive_prefix(const wchar_t _Ch) noexcept {
            return (_Ch >= L'C' && _Ch <= L'Z') || (_Ch >= L'c' && _Ch <= L'z');
        }

        inline bool _Has_drive(const unicode_string_view _Str) noexcept {
            if (_Str.size() < 2) { // must contains at least 2 elements (X:)
                return false;
            }

            return _Is_drive_prefix(_Str[0]) && _Str[1] == L':';
        }

        inline bool _Has_drive_and_slash(const unicode_string_view _Str) noexcept {
            if (_Str.size() < 3) { // must contains at least 3 elements (X:\)
                return false;
            }

            return _Is_drive_prefix(_Str[0]) && _Str[1] == L':' && _Is_slash(_Str[2]);
        }

        inline unicode_string_view _Get_root_name(const unicode_string_view _Str) noexcept {
            return _Has_drive(_Str) ? _Str.substr(0, 2) : unicode_string_view{};
        }

        inline unicode_string_view _Get_root_directory(const unicode_string_view _Str) noexcept {
            if (_Str.empty()) {
                return unicode_string_view{};
            }

            if (_Is_slash(_Str[0])) { // "\" or "/" at the begin is a root directory
                return _Str.substr(0, 1);
            }

            if (_Has_drive_and_slash(_Str)) { // get slash from "X:\" or "X:/"
                return _Str.substr(2, 1);
            } else { // no root directory
                return unicode_string_view{};
            }
        }

        inline unicode_string_view _Get_root_path(const unicode_string_view _Str) noexcept {
            if (_Str.empty()) {
                return unicode_string_view{};
            }

            if (_Is_slash(_Str[0])) { // root directory only
                return _Str.substr(0, 1);
            }

            if (_Has_drive_and_slash(_Str)) { // get "X:\" from the path
                return _Str.substr(0, 3);
            } else if (_Has_drive(_Str)) { // get "X:" from the path
                return _Str.substr(0, 2);
            } else { // no root path
                return unicode_string_view{};
            }
        }

        inline unicode_string_view _Get_relative_path(const unicode_string_view _Str) noexcept {
            if (_Str.empty()) {
                return unicode_string_view{};
            }

            if (_Is_slash(_Str[0])) {
                return _Str.substr(1); // skip "\" or "/"
            } else if (_Has_drive_and_slash(_Str)) {
                return _Str.substr(3); // skip "X:\" or "X:/"
            } else if (_Has_drive(_Str)) {
                return _Str.substr(2); // skip "X:"
            } else { // the whole path is a relative path
                return _Str;
            }
        }

        inline unicode_string_view _Get_parent_path(const unicode_string_view _Str) noexcept {
            if (_Str.empty()) {
                return unicode_string_view{};
            }
        
            const size_t _Slash = _Find_last_slash(_Str);
            switch (_Slash) {
            case 0: // root directory is a parent path
                return _Str.substr(0, 1);
            case unicode_string_view::npos: // no parent path
                return _Str;
            default:
                return _Str.substr(0, _Slash);
            }
        }

        struct _Path_segment {
            size_t _Offset;
            size_t _Length;

            bool _Found() const noexcept {
                return _Offset != unicode_string_view::npos && _Length != 0;
            }
        };

        inline _Path_segment _Find_filename(const unicode_string_view _Str) noexcept {
            if (_Str.empty()) {
                return {unicode_string_view::npos, 0};
            }

            const size_t _Slash = _Find_last_slash(_Str);
            if (_Slash == unicode_string_view::npos) { // the whole path is a filename
                return {0, _Str.size()};
            } else if (_Slash == _Str.size() - 1) { // slash at the end means that a path ends with a directory
                return {unicode_string_view::npos, 0};
            } else { // found a valid filename
                return {_Slash + 1, _Str.size() - _Slash - 1};
            }
        }

        inline unicode_string_view _Get_filename(const unicode_string_view _Str) noexcept {
            const _Path_segment& _Filename = _Find_filename(_Str);
            return _Filename._Found()
                ? _Str.substr(_Filename._Offset, _Filename._Length) : unicode_string_view{};
        }

        inline unicode_string_view _Get_extension_from_filename(const unicode_string_view _Filename) noexcept {
            if (_Filename.empty()) {
                return unicode_string_view{};
            }
        
            if (_Is_dot_or_dot_dot(_Filename)) { // skip special filesystem elements
                return unicode_string_view{};
            }

            const size_t _Dot = _Filename.rfind(L'.');
            return _Dot != unicode_string_view::npos && _Dot != 0
                ? _Filename.substr(_Dot, _Filename.size() - _Dot) : unicode_string_view{};
        }

        inline unicode_string_view _Get_extension(const unicode_string_view _Str) noexcept {
            return _Get_extension_from_filename(_Get_filename(_Str));
        }

        inline unicode_string_view _Get_stem(const unicode_string_view _Str) noexcept {
            const unicode_string_view _Filename  = _Get_filename(_Str);
            const unicode_string_view _Extension = _Get_extension_from_filename(_Filename);
            if (_Filename.empty()) { // no filename so no stem
                return unicode_string_view{};
            } else if (_Extension.empty()) { // filename without extension (filename is a stem)
                return _Filename;
            } else { // found a valid stem
                return _Filename.substr(0, _Filename.size() - _Extension.size());
            }
        }

        inline size_t _Get_current_path_length() noexcept {
#ifdef _M_X64
            return static_cast<size_t>(::GetCurrentDirectoryW(0, nullptr) - 1); // exclude null-terminator
#else // ^^^ _M_X64 ^^^ / vvv _M_IX86 vvv
            return ::GetCurrentDirectoryW(0, nullptr) - 1; // exclude null-terminator
#endif // _M_X64
        }

        inline bool _Get_current_path(wchar_t* const _Buf, const size_t _Size) noexcept {
#ifdef _M_X64
            const unsigned long _USize = static_cast<unsigned long>(_Size);
            return ::GetCurrentDirectoryW(_USize + 1, _Buf) == _USize; // include null-terminator
#else // ^^^ _M_X64 ^^^ / vvv _M_IX86 vvv
            return ::GetCurrentDirectoryW(_Size + 1, _Buf) == _Size; // include null-terminator
#endif // _M_X64
        }

        inline size_t _Get_final_path_length(void* const _Handle) noexcept {
            // get final path length (null-terminator not included)
#ifdef _M_X64
            return static_cast<size_t>(::GetFinalPathNameByHandleW(_Handle, nullptr, 0, 0) - 1);
#else // ^^^ _M_X64 ^^^ / vvv _M_IX86 vvv
            return ::GetFinalPathNameByHandleW(_Handle, nullptr, 0, 0) - 1;
#endif // _M_X64
        }

        inline bool _Get_final_path_by_handle(
            void* const _Handle, wchar_t* const _Buf, const size_t _Size) noexcept {
            // get final path (size must include null-terminator)
#ifdef _M_X64
            const unsigned long _USize = static_cast<unsigned long>(_Size);
            return ::GetFinalPathNameByHandleW(_Handle, _Buf, _USize + 1, 0) == _USize;
#else // ^^^ _M_X64 ^^^ / vvv _M_IX86 vvv
            return ::GetFinalPathNameByHandleW(_Handle, _Buf, _Size + 1, 0) == _Size;
#endif // _M_X64
        }
    } // namespace mjfs_impl
} // namespace mjx

#endif // _MJFS_IMPL_PATH_HPP_