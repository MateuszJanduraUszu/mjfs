// file.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _MJFS_IMPL_FILE_HPP_
#define _MJFS_IMPL_FILE_HPP_
#include <cstdint>
#include <cwchar>
#include <mjfs/bitmask.hpp>
#include <mjfs/file.hpp>
#include <mjfs/impl/tinywin.hpp>
#include <mjmem/allocator.hpp>
#include <mjstr/string_view.hpp>

namespace mjx {
    namespace mjfs_impl {
        class _Close_handle_guard {
        public:
            void* _Handle;

            ~_Close_handle_guard() noexcept {
                if (_Holds_valid_handle()) {
                    ::CloseHandle(_Handle);
                    _Handle = nullptr;
                }
            }

            bool _Holds_valid_handle() const noexcept {
                return _Handle != nullptr && _Handle != INVALID_HANDLE_VALUE;
            }

            void* _Release() noexcept { // release the stored handle
                void* const _Result = _Handle;
                _Handle             = nullptr;
                return _Result;
            }
        };

        [[nodiscard]] inline void* _Create_file(const wchar_t* const _Path, const file_share _Share,
            const file_attribute _Attributes, const file_flag _Flags) noexcept {
            return ::CreateFileW(_Path, GENERIC_ALL, static_cast<unsigned long>(_Share),
                nullptr, CREATE_NEW, static_cast<unsigned long>(_Attributes)
                    | static_cast<unsigned long>(_Flags), nullptr);
        }

        [[nodiscard]] inline void* _Open_file(const wchar_t* const _Target, const file_access _Access,
            const file_share _Share, const file_flag _Flags) noexcept {
            return ::CreateFileW(_Target, static_cast<unsigned long>(_Access),
                static_cast<unsigned long>(_Share), nullptr, OPEN_EXISTING,
                    static_cast<unsigned long>(_Flags) | FILE_ATTRIBUTE_NORMAL, nullptr);
        }

        inline file_attribute _Adjust_attributes(
            const file_attribute _Attributes, const file_perms _Perms) noexcept {
            if (_Perms == file_perms::readonly) {
                return _Attributes | file_attribute::readonly;
            } else {
                return _Attributes & ~file_attribute::readonly;
            }
        }

        inline FILE_BASIC_INFO _Get_file_basic_info(void* const _Handle) noexcept {
            FILE_BASIC_INFO _Info = {0};
            _Info.FileAttributes  = INVALID_FILE_ATTRIBUTES;
            return ::GetFileInformationByHandleEx(
                _Handle, FileBasicInfo, &_Info, sizeof(FILE_BASIC_INFO)) != 0 ? _Info : FILE_BASIC_INFO{};
        }

        inline file_attribute _Get_file_attributes(void* const _Handle) noexcept {
            return static_cast<file_attribute>(_Get_file_basic_info(_Handle).FileAttributes);
        }

        inline bool _Is_file_readonly(const file_attribute _Attributes) noexcept {
            return _Has_bits(_Attributes, file_attribute::readonly);
        }

        template <FILE_INFO_BY_HANDLE_CLASS _Class, class _Ty>
        inline bool _Set_file_information(void* const _Handle, _Ty& _Info) noexcept {
            return ::SetFileInformationByHandle(_Handle, _Class, &_Info, sizeof(_Ty)) != 0;
        }

        inline bool _Set_file_attributes(
            void* const _Handle, const file_attribute _New_attributes) noexcept {
            FILE_BASIC_INFO _Info = _Get_file_basic_info(_Handle);
            _Info.FileAttributes  = static_cast<unsigned long>(_New_attributes);
            return _Set_file_information<FileBasicInfo>(_Handle, _Info);
        }

        inline uint64_t _Get_file_size(void* const _Handle) noexcept {
            unsigned long _High = 0;
            return static_cast<uint64_t>(
                ::GetFileSize(_Handle, &_High)) | (static_cast<uint64_t>(_High) << 32);
        }

        inline bool _Seek_file(void* const _Handle, const uint64_t _New_pos) noexcept {
            const long _Low = static_cast<long>(_New_pos & 0x0000'0000'FFFF'FFFF);
            long _High      = static_cast<long>((_New_pos & 0xFFFF'FFFF'0000'0000) >> 32);
            return ::SetFilePointer(_Handle, _Low, &_High, FILE_BEGIN) != INVALID_SET_FILE_POINTER;
        }

        inline bool _Seek_file_to_end(void* const _Handle) noexcept {
            return ::SetFilePointer(_Handle, 0, nullptr, FILE_END) != INVALID_SET_FILE_POINTER;
        }

        inline uint64_t _Resize_file(void* const _Handle, const uint64_t _New_size) noexcept {
            return _Seek_file(_Handle, _New_size) && ::SetEndOfFile(_Handle) != 0;
        }

        inline bool _Is_file_temporary(void* const _Handle) noexcept {
            FILE_STANDARD_INFO _Info = {0};
            if (::GetFileInformationByHandleEx(
                _Handle, FileStandardInfo, &_Info, sizeof(FILE_STANDARD_INFO)) == 0) {
                return false;
            }

            return _Info.DeletePending == 1
                && _Has_bits(_Get_file_attributes(_Handle), file_attribute::temporary);
        }

        enum class _Set_delete_flag_result : unsigned char {
            _Success,
            _Access_denied,
            _Error
        };

        inline _Set_delete_flag_result _Set_delete_flag(void* const _Handle) noexcept {
            FILE_DISPOSITION_INFO _Info;
            _Info.DeleteFile = true;
            if (_Set_file_information<FileDispositionInfo>(_Handle, _Info)) {
                return _Set_delete_flag_result::_Success;
            } else {
                return ::GetLastError() == ERROR_ACCESS_DENIED ?
                    _Set_delete_flag_result::_Access_denied : _Set_delete_flag_result::_Error;
            }
        }

        inline bool _Clear_delete_flag(void* const _Handle) noexcept {
            FILE_DISPOSITION_INFO _Info;
            _Info.DeleteFile = false;
            return _Set_file_information<FileDispositionInfo>(_Handle, _Info);
        }

        inline bool _Make_file_regular(void* const _Handle) noexcept {
            const file_attribute _Attributes = _Get_file_attributes(_Handle);
            if (_Has_bits(_Attributes, file_attribute::temporary)) { // remove attribute
                if (!_Set_file_attributes(_Handle, _Attributes & ~file_attribute::temporary)) {
                    return false;
                }
            }

            return _Clear_delete_flag(_Handle);
        }

        template <class _Ty>
        struct _Allocated_object {
            _Ty* _Obj;
            size_t _Size;

            constexpr _Ty* operator->() const noexcept {
                return _Obj;
            }

            constexpr _Allocated_object(const size_t _Size)
                : _Obj(static_cast<_Ty*>(::mjx::get_allocator().allocate(_Size))), _Size(_Size) {}

            constexpr ~_Allocated_object() noexcept {
                ::mjx::get_allocator().deallocate(_Obj, _Size);
            }
        };

        inline bool _Rename_file_by_handle(void* const _Handle, const unicode_string_view _New_path) {
            _Allocated_object<FILE_RENAME_INFO> _Info(
                sizeof(FILE_RENAME_INFO) + ((_New_path.size() + 1) * sizeof(wchar_t)));
            _Info->ReplaceIfExists = true;
            _Info->RootDirectory   = nullptr;
#ifdef _M_X64
            _Info->FileNameLength  = static_cast<unsigned long>(_New_path.size() * sizeof(wchar_t));
#else // ^^^ _M_X64 ^^^ / vvv _M_IX86 vvv
            _Info->FileNameLength  = _New_path.size() * sizeof(wchar_t);
#endif // _M_X64
            ::wmemcpy(_Info->FileName, _New_path.data(), _New_path.size() + 1);
            return _Set_file_information<FileRenameInfo>(_Handle, *_Info._Obj);
        }
    } // namespace mjfs_impl
} // namespace mjx

#endif // _MJFS_IMPL_FILE_HPP_