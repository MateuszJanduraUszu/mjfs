// path.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _MJFS_DETAILS_PATH_HPP_
#define _MJFS_DETAILS_PATH_HPP_
#include <cstddef>
#include <string_view>
#include <mjfs/tinywin.hpp>

namespace mjfs {
    namespace details {
        inline bool _Is_slash(const wchar_t _Ch) noexcept {
            return _Ch == L'\\' || _Ch == L'/';
        }

        inline bool _Is_dot_or_dot_dot(const ::std::wstring_view _Str) noexcept {
            return _Str == L"." || _Str == L"..";
        }

        inline size_t _Find_last_slash(const ::std::wstring_view _Str) noexcept {
            const wchar_t* const _First = _Str.data();
            const wchar_t* _Last        = _First + _Str.size();
            for (; _Last != _First - 1; --_Last) {
                if (_Is_slash(*_Last)) {
                    return _Last - _First;
                }
            }

            return ::std::wstring_view::npos; // not found
        }

        inline bool _Is_drive_prefix(const wchar_t _Ch) noexcept {
            return (_Ch >= L'C' && _Ch <= L'Z') || (_Ch >= L'c' && _Ch <= L'z');
        }

        inline bool _Has_drive(const ::std::wstring_view _Str) noexcept {
            if (_Str.size() < 2) { // must contains at least 2 elements (X:)
                return false;
            }

            return _Is_drive_prefix(_Str[0]) && _Str[1] == L':';
        }

        inline bool _Has_drive_and_slash(const ::std::wstring_view _Str) noexcept {
            if (_Str.size() < 3) { // must contains at least 3 elements (X:\)
                return false;
            }

            return _Is_drive_prefix(_Str[0]) && _Str[1] == L':' && _Is_slash(_Str[2]);
        }

        inline ::std::wstring_view _Get_root_name(const ::std::wstring_view _Str) noexcept {
            return _Has_drive(_Str) ? ::std::wstring_view{_Str.data(), 2} : ::std::wstring_view{};
        }

        inline ::std::wstring_view _Get_root_directory(const ::std::wstring_view _Str) noexcept {
            if (_Str.empty()) {
                return ::std::wstring_view{};
            }

            if (_Is_slash(_Str[0])) { // "\" or "/" at the begin is a root directory
                return ::std::wstring_view{_Str.data(), 1};
            }

            if (_Has_drive_and_slash(_Str)) { // get slash from "X:\" or "X:/"
                return ::std::wstring_view{_Str.data() + 2, 1};
            } else { // no root directory
                return ::std::wstring_view{};
            }
        }

        inline ::std::wstring_view _Get_root_path(const ::std::wstring_view _Str) noexcept {
            if (_Str.empty()) {
                return ::std::wstring_view{};
            }

            if (_Is_slash(_Str[0])) { // root directory only
                return ::std::wstring_view{_Str.data(), 1};
            }

            if (_Has_drive_and_slash(_Str)) { // get "X:\" from the path
                return ::std::wstring_view{_Str.data(), 3};
            } else if (_Has_drive(_Str)) { // get "X:" from the path
                return ::std::wstring_view{_Str.data(), 2};
            } else { // no root path
                return ::std::wstring_view{};
            }
        }

        inline ::std::wstring_view _Get_relative_path(const ::std::wstring_view _Str) noexcept {
            if (_Str.empty()) {
                return ::std::wstring_view{};
            }

            if (_Is_slash(_Str[0])) {
                return ::std::wstring_view{_Str.data() + 1}; // skip "\" or "/"
            } else if (_Has_drive_and_slash(_Str)) {
                return ::std::wstring_view{_Str.data() + 3}; // skip "X:\" or "X:/"
            } else if (_Has_drive(_Str)) {
                return ::std::wstring_view{_Str.data() + 2}; // skip "X:"
            } else { // the whole path is a relative path
                return _Str;
            }
        }

        inline ::std::wstring_view _Get_parent_path(const ::std::wstring_view _Str) noexcept {
            if (_Str.empty()) {
                return ::std::wstring_view{};
            }
        
            const size_t _Slash = _Find_last_slash(_Str);
            switch (_Slash) {
            case 0: // root directory is a parent path
                return ::std::wstring_view{_Str.data(), 1};
            case ::std::wstring_view::npos: // no parent path
                return _Str;
            default:
                return ::std::wstring_view{_Str.data(), _Slash};
            }
        }

        struct _Path_segment {
            size_t _Offset;
            size_t _Length;

            bool _Found() const noexcept {
                return _Offset != ::std::wstring_view::npos && _Length != 0;
            }
        };

        inline _Path_segment _Find_filename(const ::std::wstring_view _Str) noexcept {
            if (_Str.empty()) {
                return {::std::wstring_view::npos, 0};
            }

            const size_t _Slash = _Find_last_slash(_Str);
            if (_Slash == ::std::wstring_view::npos) { // the whole path is a filename
                return {0, _Str.size()};
            } else if (_Slash == _Str.size() - 1) { // slash at the end means that a path ends with a directory
                return {::std::wstring_view::npos, 0};
            } else { // found a valid filename
                return {_Slash + 1, _Str.size() - _Slash - 1};
            }
        }

        inline ::std::wstring_view _Get_filename(const ::std::wstring_view _Str) noexcept {
            const _Path_segment& _Filename = _Find_filename(_Str);
            return _Filename._Found()
                ? ::std::wstring_view{_Str.data() + _Filename._Offset, _Filename._Length}
                : ::std::wstring_view{};
        }

        inline ::std::wstring_view _Get_extension_from_filename(const ::std::wstring_view _Filename) noexcept {
            if (_Filename.empty()) {
                return ::std::wstring_view{};
            }
        
            if (_Is_dot_or_dot_dot(_Filename)) { // skip special filesystem elements
                return ::std::wstring_view{};
            }

            const size_t _Dot = _Filename.rfind(L'.');
            return _Dot != ::std::wstring_view::npos && _Dot != 0
                ? ::std::wstring_view{_Filename.data() + _Dot, _Filename.size() - _Dot}
                : ::std::wstring_view{};
        }

        inline ::std::wstring_view _Get_extension(const ::std::wstring_view _Str) noexcept {
            return _Get_extension_from_filename(_Get_filename(_Str));
        }

        inline ::std::wstring_view _Get_stem(const ::std::wstring_view _Str) noexcept {
            const ::std::wstring_view _Filename  = _Get_filename(_Str);
            const ::std::wstring_view _Extension = _Get_extension_from_filename(_Filename);
            if (_Filename.empty()) { // no filename so no stem
                return ::std::wstring_view{};
            } else if (_Extension.empty()) { // filename without extension (filename is a stem)
                return _Filename;
            } else { // found a valid stem
                return ::std::wstring_view{_Filename.data(), _Filename.size() - _Extension.size()};
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
#ifdef _M_X64
            return static_cast<size_t>(::GetFinalPathNameByHandleW(_Handle, nullptr, 0, 0));
#else // ^^^ _M_X64 ^^^ / vvv _M_IX86 vvv
            return ::GetFinalPathNameByHandleW(_Handle, nullptr, 0, 0);
#endif // _M_X64
        }

        inline bool _Get_final_path_by_handle(
            void* const _Handle, wchar_t* const _Buf, const size_t _Size) noexcept {
#ifdef _M_X64
            const unsigned long _USize = static_cast<unsigned long>(_Size);
            return ::GetFinalPathNameByHandleW(
                _Handle, _Buf, _USize, 0) == _USize - 1; // exclude null-terminator
#else // ^^^ _M_X64 ^^^ / vvv _M_IX86 vvv
            return ::GetFinalPathNameByHandleW(
                _Handle, _Buf, _Size, 0) == _Size - 1; // exclude null-terminator
#endif // _M_X64
        }
    } // namespace details
} // namespace mjfs

#endif // _MJFS_DETAILS_PATH_HPP_