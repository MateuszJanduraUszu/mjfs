// status.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _MJFS_IMPL_STATUS_HPP_
#define _MJFS_IMPL_STATUS_HPP_
#include <cstdint>
#include <mjfs/bitmask.hpp>
#include <mjfs/directory.hpp>
#include <mjfs/impl/file.hpp>
#include <mjfs/impl/tinywin.hpp>
#include <mjfs/status.hpp>

namespace mjx {
    namespace mjfs_impl {
        inline file_attribute _Get_file_attributes(const wchar_t* const _Target) noexcept {
            return static_cast<file_attribute>(::GetFileAttributesW(_Target));
        }

        inline bool _Set_file_attributes(
            const wchar_t* const _Target, const file_attribute _New_attributes) noexcept {
            return ::SetFileAttributesW(_Target, static_cast<unsigned long>(_New_attributes)) != 0;
        }

        inline void* _Open_reparse_point(const wchar_t* const _Target) noexcept {
            return ::CreateFileW(_Target, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
        }

        enum class _File_reparse_tag : unsigned long {
            _Mount_point = 0xA000'0003, // IO_REPARSE_TAG_MOUNT_POINT
            _Symlink     = 0xA000'000C, // IO_REPARSE_TAG_SYMLINK
            _Unknown     = 0xFFFF'FFFF
        };

        inline _File_reparse_tag _Get_reparse_tag(const wchar_t* const _Target) noexcept {
            _Close_handle_guard _Guard = {_Open_reparse_point(_Target)};
            if (!_Guard._Holds_valid_handle()) {
                return _File_reparse_tag::_Unknown;
            }

            FILE_ATTRIBUTE_TAG_INFO _Info = {0};
            if (::GetFileInformationByHandleEx(
                _Guard._Handle, FileAttributeTagInfo, &_Info, sizeof(FILE_ATTRIBUTE_TAG_INFO)) != 0) {
                return static_cast<_File_reparse_tag>(_Info.ReparseTag);
            } else {
                return _File_reparse_tag::_Unknown;
            }
        }

        inline bool _Get_disk_space_info(const wchar_t* const _Disk, space_info& _Info) noexcept {
            ULARGE_INTEGER _Available;
            ULARGE_INTEGER _Capacity;
            ULARGE_INTEGER _Free;
            if (::GetDiskFreeSpaceExW(_Disk, &_Available, &_Capacity, &_Free) == 0) {
                return false;
            }

            _Info.available = _Available.QuadPart;
            _Info.capacity  = _Capacity.QuadPart;
            _Info.free      = _Free.QuadPart;
            return true;
        }
    } // namespace mjfs_impl
} // namespace mjx

#endif // _MJFS_IMPL_STATUS_HPP_