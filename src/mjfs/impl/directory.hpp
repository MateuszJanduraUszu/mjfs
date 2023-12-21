// directory.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _MJFS_IMPL_DIRECTORY_HPP_
#define _MJFS_IMPL_DIRECTORY_HPP_
#include <cwchar>
#include <mjfs/directory.hpp>
#include <mjfs/impl/path.hpp>
#include <mjfs/impl/tinywin.hpp>
#include <mjfs/path.hpp>
#include <type_traits>
#include <vector>

namespace mjx {
    namespace mjfs_impl {
        inline bool _Is_directory_iterator_handle_valid(void* const _Handle) noexcept {
            return _Handle != nullptr && _Handle != INVALID_HANDLE_VALUE;
        }

        inline void* _Open_directory_iterator(
            const wchar_t* const _Target, WIN32_FIND_DATAW* const _Data) noexcept {
            return ::FindFirstFileExW(_Target, FindExInfoBasic, _Data, FindExSearchNameMatch, nullptr, 0);
        }

        inline void _Close_directory_iterator(void*& _Handle) noexcept {
            ::FindClose(_Handle);
            _Handle = nullptr;
        }

        inline bool _Advance_directory_iterator(void* const _Handle, WIN32_FIND_DATAW* const _Data) noexcept {
            return ::FindNextFileW(_Handle, _Data) != 0;
        }

        inline bool _Assume_no_more_files() noexcept {
            return ::GetLastError() == ERROR_NO_MORE_FILES;
        }

        inline bool _Assume_access_denied() noexcept {
            return ::GetLastError() == ERROR_ACCESS_DENIED;
        }

        inline void _Remove_filename_and_slash(path& _Path) {
            path::string_type _Str = _Path.remove_filename().native();
            while (_Is_slash(_Str.back())) { // remove all trailing slashes
                _Str.pop_back();
            }

            _Path = ::std::move(_Str);
        }

        class _Copied_filename { // temporary buffer for filename
        public:
            wchar_t _Raw[260]; // must fit cFileName from WIN32_FIND_DATAW

            explicit _Copied_filename(const wchar_t* _Filename) noexcept : _Raw{0} {
#pragma warning(suppress : 4996) // C4996: consider using wcscpy_s() instead
                ::wcscpy(_Raw, _Filename);
            }
        };

        class _Dir_iter_base { // base class for all directory iterators
        public:
            WIN32_FIND_DATAW _Data;
            void* _Handle;
            directory_entry _Entry;
            path _Path;
            
            _Dir_iter_base() = delete;

            explicit _Dir_iter_base(const path& _Target)
                : _Data(), _Handle(_Open(_Target)), _Entry(), _Path(_Target) {
                if (_Valid()) {
                    _Assign();
                }
            }

            ~_Dir_iter_base() noexcept {
                if (_Valid()) {
                    _Close_directory_iterator(_Handle);
                }
            }

            _Dir_iter_base(const _Dir_iter_base&)     = default;
            _Dir_iter_base(_Dir_iter_base&&) noexcept = default;

            bool _Valid() const noexcept {
                return _Is_directory_iterator_handle_valid(_Handle);
            }

            void _Assign() {
                _Entry._Myattr = static_cast<file_attribute>(_Data.dwFileAttributes);
                _Entry._Mypath = _Path / _Data.cFileName;
            }

        protected:
            void* _Open(const path& _Target) {
                return _Open_directory_iterator((_Target / L"*").c_str(), &_Data);
            }
        };

        class _Dir_iter : public _Dir_iter_base {
        public:
            _Dir_iter() = delete;

            explicit _Dir_iter(const path& _Target) : _Dir_iter_base(_Target) {}

            ~_Dir_iter() noexcept {}

            _Dir_iter(const _Dir_iter&)     = default;
            _Dir_iter(_Dir_iter&&) noexcept = default;

            bool _Skip_dots() {
                while (_Is_dot_or_dot_dot(_Data.cFileName)) {
                    if (!_Advance_directory_iterator(_Handle, &_Data)) {
                        return false;
                    }
                }

                _Assign();
                return true;
            }

            bool _Advance() {
                do {
                    if (!_Advance_directory_iterator(_Handle, &_Data)) {
                        return false;
                    }
                } while (_Is_dot_or_dot_dot(_Data.cFileName));

                _Assign();
                return true;
            }
        };

        class _Recursive_dir_iter : public _Dir_iter_base {
        public:
            ::std::vector<void*> _Stack;
            directory_options _Options;
            bool _Recursion_pending;

            _Recursive_dir_iter() = delete;

            explicit _Recursive_dir_iter(const path& _Target, const directory_options _Options)
                : _Dir_iter_base(_Target), _Stack(), _Options(_Options), _Recursion_pending(false) {}

            ~_Recursive_dir_iter() noexcept {}

            _Recursive_dir_iter(const _Recursive_dir_iter&)     = default;
            _Recursive_dir_iter(_Recursive_dir_iter&&) noexcept = default;

            bool _Should_recurse() const noexcept {
                // Note: An entry marked with the FILE_ATTRIBUTE_DIRECTORY attribute can represent
                //       either a standard directory or a symbolic link or junction to a directory.
                //       When processing such entries, we always perform recursion on standard directores,
                //       but the behavior for links is optional. If the follow_directory_symlink option
                //       was specified, we perform recursion, otherwise we treat them as standard entries.
                if ((_Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                    return false;
                }

                return (_Data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0
                    ? _Has_bits(_Options, directory_options::follow_directory_symlink) : true;
            }

            bool _Skip_dots() {
                while (_Is_dot_or_dot_dot(_Data.cFileName)) {
                    if (!_Advance_directory_iterator(_Handle, &_Data)) {
                        return _Assume_no_more_files() ? _Pop() : false;
                    }
                }

                _Assign();
                if (_Should_recurse()) {
                    _Recursion_pending = true; // recurse on next iteration
                }

                return true;
            }

            bool _Advance() {
                if (_Recursion_pending) {
                    _Copied_filename _Filename(_Data.cFileName); // use separate buffer to avoid data loss
                    void* const _New_handle = _Open(_Path / _Filename._Raw);
                    if (_Is_directory_iterator_handle_valid(_New_handle)) { // success, recurse
                        _Stack.push_back(_Handle);
                        _Handle            = _New_handle;
                        _Path             /= _Filename._Raw;
                        _Recursion_pending = false;
                        return _Skip_dots();
                    } else { // failure, skip directory or report an error
                        if (_Assume_access_denied()
                            && _Has_bits(_Options, directory_options::skip_permission_denied)) {
                            _Recursion_pending = false;
                            return _Advance();
                        } else {
                            return false;
                        }
                    }
                }

                do {
                    if (!_Advance_directory_iterator(_Handle, &_Data)) {
                        // Note: The _Advance_directory_iterator() function may encounter failure due to
                        //       various reasons, but we are specifically interested in two scenarios.
                        //       In the first case, if an error occurs, we should simply report the failure.
                        //       In the second case, when the failure reason is ERROR_NO_MORE_FILES,
                        //       we can deduce that we have reached the end of the current directory,
                        //       and therefore, we should navigate back to the parent directory if any exists.
                        if (!_Assume_no_more_files()) {
                            return false;
                        }

                        if (_Stack.empty()) {
                            return false;
                        }

                        _Handle = _Stack.back();
                        _Stack.pop_back();
                        _Remove_filename_and_slash(_Path);
                        return _Advance();
                    }
                } while (_Is_dot_or_dot_dot(_Data.cFileName));

                if (_Should_recurse()) {
                    _Recursion_pending = true; // recurse on next iterator
                }

                _Assign();
                return true;
            }

            bool _Pop() {
                if (_Stack.empty()) { // no more levels, do nothing
                    return false;
                }

                _Close_directory_iterator(_Handle);
                _Handle = _Stack.back();
                _Stack.pop_back();
                _Remove_filename_and_slash(_Path);
                _Recursion_pending = false; // reset recursion flag
                return _Advance(); // skip current entry (always the directory we were in)
            }
        };
    } // namespace mjfs_impl
} // namespace mjx

#endif // _MJFS_IMPL_DIRECTORY_HPP_