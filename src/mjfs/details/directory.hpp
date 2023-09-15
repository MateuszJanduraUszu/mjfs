// directory.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _MJFS_DETAILS_DIRECTORY_HPP_
#define _MJFS_DETAILS_DIRECTORY_HPP_
#include <mjfs/details/path.hpp>
#include <mjfs/directory.hpp>
#include <mjfs/path.hpp>
#include <mjfs/tinywin.hpp>
#include <type_traits>

namespace mjfs {
    namespace details {
        inline bool _Is_directory_iterator_handle_valid(void* const _Handle) noexcept {
            return _Handle != nullptr && _Handle != INVALID_HANDLE_VALUE;
        }

        inline void* _Open_directory_iterator(const wchar_t* const _Target, WIN32_FIND_DATAW& _Data) noexcept {
            return ::FindFirstFileExW(
                _Target, FindExInfoBasic, ::std::addressof(_Data), FindExSearchNameMatch, nullptr, 0);
        }

        inline void _Close_directory_iterator(void*& _Handle) noexcept {
            ::FindClose(_Handle);
            _Handle = nullptr;
        }

        inline bool _Advance_directory_iterator(void* const _Handle, WIN32_FIND_DATAW& _Data) noexcept {
            return ::FindNextFileW(_Handle, ::std::addressof(_Data)) != 0;
        }

        class _Dir_iter {
        public:
            directory_entry _Entry;

            _Dir_iter() = delete;

            explicit _Dir_iter(const path& _Target) : _Entry(), _Mydata(),
                _Myhandle(_Open_directory_iterator((_Target / L"*").c_str(), _Mydata)), _Mypath(_Target) {
                if (_Valid()) {
                    _Assign();
                }
            }

            _Dir_iter(const _Dir_iter&)     = default;
            _Dir_iter(_Dir_iter&&) noexcept = default;
            ~_Dir_iter() noexcept           = default;
            
            bool _Valid() const noexcept {
                return _Is_directory_iterator_handle_valid(_Myhandle);
            }

            bool _Skip_dots() noexcept {
                while (_Is_dot_or_dot_dot(_Mydata.cFileName)) {
                    if (!_Advance_directory_iterator(_Myhandle, _Mydata)) {
                        return false;
                    }
                }

                _Assign();
                return true;
            }

            bool _Advance() noexcept {
                do {
                    if (!_Advance_directory_iterator(_Myhandle, _Mydata)) {
                        return false;
                    }
                } while (_Is_dot_or_dot_dot(_Mydata.cFileName));
            
                _Assign();
                return true;
            }

        private:
            void _Assign() {
                _Entry._Myattr = static_cast<file_attribute>(_Mydata.dwFileAttributes);
                _Entry._Mypath = _Mypath / _Mydata.cFileName;
            }

            void* _Myhandle;
            WIN32_FIND_DATAW _Mydata;
            path _Mypath; // root path
        };
    } // namespace details
} // namespace mjfs

#endif // _MJFS_DETAILS_DIRECTORY_HPP_