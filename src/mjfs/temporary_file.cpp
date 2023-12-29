// temporary_file.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <mjfs/impl/file.hpp>
#include <mjfs/temporary_file.hpp>
#include <type_traits>

namespace mjx {
    temporary_file::temporary_file() noexcept : file() {}

    temporary_file::temporary_file(temporary_file&& _Other) noexcept : file(::std::move(_Other)) {}

    temporary_file::~temporary_file() noexcept {}

    temporary_file& temporary_file::operator=(temporary_file&& _Other) noexcept {
        file::operator=(::std::move(_Other));
        return *this;
    }

    bool temporary_file::is_temporary() const noexcept {
        return mjfs_impl::_Is_file_temporary(native_handle());
    }

    bool temporary_file::make_regular() noexcept {
        const native_handle_type _Handle = native_handle();
        if (!mjfs_impl::_Is_file_temporary(_Handle)) { // already regular, do nothing
            return true;
        }

        return mjfs_impl::_Make_file_regular(_Handle);
    }

    bool create_temporary_file(const path& _Path, const file_share _Share, const file_attribute _Attributes,
        const file_flag _Flags, const file_perms _Perms, temporary_file& _File) {
        // Note: To mark a file for deletion upon closing, we could simply set the delete_on_close flag.
        //       However, this approach doesn't allow us to reverse the operation using the
        //       temporary_file::make_regular() method. To achive both - marking the file for deletion
        //       upon the closing and retaining the ability to undo this action,
        //       we can set the delete flag. This marks the file for deletion upon closing,
        //       but still allows us to reverse the action if needed.
        if (!::mjx::create_file(_Path, _Share, _Attributes | file_attribute::temporary,
            _Flags & ~file_flag::delete_on_close, _Perms, ::std::addressof(_File))) {
            return false;
        }

        return mjfs_impl::_Set_delete_flag(
            _File.native_handle()) == mjfs_impl::_Set_delete_flag_result::_Success;
    }

    bool create_temporary_file(const path& _Path, temporary_file& _File) {
        return ::mjx::create_temporary_file(_Path, file_share::all, file_attribute::normal
            | file_attribute::temporary, file_flag::none, file_perms::all, _File);
    }
} // namespace mjx