// file.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <memory>
#include <mjfs/details/file.hpp>
#include <mjfs/details/path.hpp>
#include <mjfs/details/status.hpp>
#include <mjfs/file.hpp>
#include <new>
#include <type_traits>

namespace mjfs {
    file::file() noexcept : _Myhandle(_Invalid_handle) {}

    file::file(file&& _Other) noexcept : _Myhandle(_Other._Myhandle) {
        _Other._Myhandle = _Invalid_handle;
    }

    file::file(const path& _Target, const file_access _Access, const file_share _Share,
        const file_flag _Flags) : _Myhandle(_Invalid_handle) {
        (void) open(_Target, _Access, _Share, _Flags);
    }

    file::~file() noexcept {
        close();
    }

    file& file::operator=(file&& _Other) noexcept {
        if (this != ::std::addressof(_Other)) {
            _Myhandle        = _Other._Myhandle;
            _Other._Myhandle = _Invalid_handle;
        }

        return *this;
    }

    bool file::is_open() const noexcept {
        return _Myhandle != nullptr && _Myhandle != _Invalid_handle;
    }

    const file::native_handle_type file::native_handle() const noexcept {
        return _Myhandle;
    }

    void file::close() noexcept {
        if (is_open()) {
            ::CloseHandle(_Myhandle);
            _Myhandle = _Invalid_handle;
        }
    }

    bool file::open(
        const path& _Target, const file_access _Access, const file_share _Share, const file_flag _Flags) {
        if (is_open()) { // some handle is currently held
            return false;
        }

        _Myhandle = details::_Open_file(_Target.c_str(), _Access, _Share, _Flags);
        return is_open();
    }

    bool file::set_handle(const native_handle_type _New_handle) noexcept {
        if (is_open()) { // some handle is currently held
            return false;
        }

        _Myhandle = _New_handle;
        return true;
    }

    uint64_t file::size() const noexcept {
        return is_open() ? details::_Get_file_size(_Myhandle) : 0;
    }

    bool file::resize(const uint64_t _New_size) noexcept {
        return is_open() ? details::_Resize_file(_Myhandle, _New_size) : false;
    }

    file_attribute file::attributes() const noexcept {
        return is_open() ? details::_Get_file_attributes(_Myhandle) : file_attribute::unknown;
    }

    bool file::attributes(const file_attribute _New_attributes) noexcept {
        return is_open() ? details::_Set_file_attributes(_Myhandle, _New_attributes) : false;
    }

    file_perms file::permissions() const noexcept {
        if (!is_open()) {
            return file_perms::unknown;
        }

        return details::_Is_file_readonly(details::_Get_file_attributes(_Myhandle))
            ? file_perms::readonly : file_perms::all;
    }

    bool file::permissions(const file_perms _New_perms) noexcept {
        if (!is_open()) {
            return false;
        }

        file_attribute _Attributes = details::_Get_file_attributes(_Myhandle);
        if (_New_perms == file_perms::readonly) {
            _Attributes |= file_attribute::readonly;
        } else {
            _Attributes &= ~file_attribute::readonly;
        }

        return details::_Set_file_attributes(_Myhandle, _Attributes);
    }

    bool create_file(const path& _Path, const file_share _Share, const file_attribute _Attributes,
        const file_flag _Flags, const file_perms _Perms, file* const _File) {
        const bool _Store_handle                  = _File != nullptr && !_File->is_open();
        const file_attribute _Adjusted_attributes = details::_Adjust_attributes(_Attributes, _Perms);
        details::_Close_handle_guard _Guard       = {details::_Create_file(
            _Path.c_str(), _Share, _Adjusted_attributes, _Flags)};
        if (!_Guard._Holds_valid_handle()) {
            return false;
        }

        if (!_Store_handle) {
            return true;
        }

        // Note: We can call file::set_handle() without any checks, because it only fails
        //       when any handle is currently held. We already checked it via _Store_handle.
        (void) _File->set_handle(_Guard._Release());
        return true;
    }

    bool create_file(const path& _Path, file* const _File) {
        return ::mjfs::create_file(_Path, file_share::none, file_attribute::normal,
            file_flag::none, file_perms::all, _File);
    }

    bool delete_file(const path& _Target) {
        return ::DeleteFileW(_Target.c_str()) != 0;
    }

    bool delete_file(file& _File) noexcept {
        if (!_File.is_open()) {
            return false;
        }

        switch (details::_Set_delete_flag(_File.native_handle())) {
        case details::_Set_delete_flag_result::_Success:
            _File.close(); // automatically deletes on close
            return true;
        case details::_Set_delete_flag_result::_Access_denied: // access denied, try another way
            break;
        default: // an error occured, break
            return false;
        }

        // Note: We couldn't set the delete flag due to insufficient access rights, but we can
        //       attempt to obtain the file's full path and then use DeleteFileW() to delete
        //       it based on its full path rather than its handle.
        const size_t _Buf_size = details::_Get_final_path_length(_File.native_handle());
        const ::std::unique_ptr<wchar_t[]> _Buf(new (::std::nothrow) wchar_t[_Buf_size]);
        if (!_Buf) { // allocation failed, break
            return false;
        }
        
        if (!details::_Get_final_path_by_handle(_File.native_handle(), _Buf.get(), _Buf_size)) {
            return false;
        }

        _File.close(); // must be closed
        return ::DeleteFileW(_Buf.get()) != 0;
    }

    bool rename(const path& _Old_path, const path& _New_path) {
        return ::MoveFileExW(_Old_path.c_str(), _New_path.c_str(),
            MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING) != 0;
    }
} // namespace mjfs