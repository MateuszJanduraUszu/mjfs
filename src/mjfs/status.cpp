// status.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <mjfs/details/status.hpp>
#include <mjfs/status.hpp>
#include <mjfs/tinywin.hpp>

namespace mjfs {
    bool exists(const path& _Target) {
        return details::_Get_file_attributes(_Target.c_str()) != file_attribute::unknown;
    }

    bool is_directory(const path& _Target) {
        return _Has_bits(details::_Get_file_attributes(_Target.c_str()), file_attribute::directory);
    }

    bool is_regular_file(const path& _Target) {
        const file_attribute _Attributes = details::_Get_file_attributes(_Target.c_str());
        return _Attributes != file_attribute::unknown && !_Has_bits(_Attributes, file_attribute::directory)
            && !_Has_bits(_Attributes, file_attribute::reparse_point);
    }
    
    bool is_symlink(const path& _Target) {
        const file_attribute _Attributes = details::_Get_file_attributes(_Target.c_str());
        if (_Attributes == file_attribute::unknown || !_Has_bits(_Attributes, file_attribute::reparse_point)) {
            return false;
        }

        return details::_Get_reparse_tag(_Target.c_str()) == details::_File_reparse_tag::_Symlink;
    }

    bool is_junction(const path& _Target) {
        const file_attribute _Attributes = details::_Get_file_attributes(_Target.c_str());
        if (_Attributes == file_attribute::unknown || !_Has_bits(_Attributes, file_attribute::reparse_point)) {
            return false;
        }

        return details::_Get_reparse_tag(_Target.c_str()) == details::_File_reparse_tag::_Mount_point;
    }

    space_info space(const path& _Target) {
        if (!_Target.has_root_path()) {
            return space_info{0, 0, 0};
        }
        
        space_info _Result;
        return details::_Get_disk_space_info(_Target.c_str(), _Result) ? _Result : space_info{0, 0, 0};
    }

    bool is_hidden(const path& _Target) {
        return _Has_bits(details::_Get_file_attributes(_Target.c_str()), file_attribute::hidden);
    }

    bool make_hidden(const path& _Target) {
        const file_attribute _Attributes = details::_Get_file_attributes(_Target.c_str());
        if (_Has_bits(_Attributes, file_attribute::hidden)) { // target is already hidden
            return false;
        }

        return details::_Set_file_attributes(_Target.c_str(), _Attributes | file_attribute::hidden);
    }

    bool make_visible(const path& _Target) {
        const file_attribute _Attributes = details::_Get_file_attributes(_Target.c_str());
        if (!_Has_bits(_Attributes, file_attribute::hidden)) { // target is already visible
            return false;
        }

        return details::_Set_file_attributes(_Target.c_str(), _Attributes & ~file_attribute::hidden);
    }

    bool is_readonly(const path& _Target) {
        return _Has_bits(details::_Get_file_attributes(_Target.c_str()), file_attribute::readonly);
    }

    bool make_readonly(const path& _Target) {
        const file_attribute _Attributes = details::_Get_file_attributes(_Target.c_str());
        if (_Has_bits(_Attributes, file_attribute::readonly)) { // target is already read-only
            return false;
        }

        return details::_Set_file_attributes(_Target.c_str(), _Attributes | file_attribute::readonly);
    }

    bool make_writable(const path& _Target) {
        const file_attribute _Attributes = details::_Get_file_attributes(_Target.c_str());
        if (!_Has_bits(_Attributes, file_attribute::readonly)) { // target is already writable
            return false;
        }

        return details::_Set_file_attributes(_Target.c_str(), _Attributes & ~file_attribute::readonly);
    }
} // namespace mjfs