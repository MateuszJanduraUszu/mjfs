// directory.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <cstdlib>
#include <mjfs/bitmask.hpp>
#include <mjfs/details/directory.hpp>
#include <mjfs/details/path.hpp>
#include <mjfs/details/status.hpp>
#include <mjfs/directory.hpp>

namespace mjfs {
    directory_entry::directory_entry(const path& _Target)
        : _Myattr(details::_Get_file_attributes(_Target.c_str())), _Mypath(_Target) {}

    void directory_entry::assign(const path& _New_target) {
        if (_New_target != _Mypath) {
            _Mypath = _New_target;
            refresh();
        }
    }

    void directory_entry::replace_filename(const path& _Replacement) {
        if (_Replacement != _Mypath.filename()) {
            _Mypath.replace_filename(_Replacement);
            refresh();
        }
    }

    void directory_entry::refresh() noexcept {
        _Myattr = details::_Get_file_attributes(_Mypath.c_str());
    }

    const path& directory_entry::absolute_path() const noexcept {
        return _Mypath;
    }

    bool directory_entry::exists() const noexcept {
        return _Myattr != file_attribute::unknown;
    }

    bool directory_entry::is_directory() const noexcept {
        return _Has_bits(_Myattr, file_attribute::directory)
            && !_Has_bits(_Myattr, file_attribute::reparse_point); // skip symbolic links and junctions
    }

    bool directory_entry::is_regular_file() const noexcept {
        return !_Has_bits(_Myattr, file_attribute::directory)
            && !_Has_bits(_Myattr, file_attribute::reparse_point); // skip symbolic links
    }

    bool directory_entry::is_symlink() const noexcept {
        if (!_Has_bits(_Myattr, file_attribute::reparse_point)) {
            return false;
        }

        return details::_Get_reparse_tag(_Mypath.c_str()) == details::_File_reparse_tag::_Symlink;
    }

    bool directory_entry::is_junction() const noexcept {
        if (!_Has_bits(_Myattr, file_attribute::reparse_point)) {
            return false;
        }

        return details::_Get_reparse_tag(_Mypath.c_str()) == details::_File_reparse_tag::_Mount_point;
    }

    directory_iterator::directory_iterator() noexcept : _Myimpl(nullptr) {}

    directory_iterator::directory_iterator(const path& _Target)
        : _Myimpl(::std::make_shared<details::_Dir_iter>(_Target)) {
        if (_Myimpl) {
            if (!_Myimpl->_Skip_dots()) {
                _Myimpl.reset();
            }
        }
    }

    directory_iterator::directory_iterator(const path& _Target, const directory_options)
        : _Myimpl(::std::make_shared<details::_Dir_iter>(_Target)) {
        // Note: Directory options have no effect on directory_iterator. The follow_directory_symlink
        //       applies only to recursive_directory_iterator, and the skip_permission_denied option
        //       resets the iterator instead of throwing an exception on access denied,
        //       but since we don't use exceptions, this option also has no effect.
        if (_Myimpl) {
            if (!_Myimpl->_Skip_dots()) {
                _Myimpl.reset();
            }
        }
    }

    directory_iterator::~directory_iterator() noexcept {}

    bool directory_iterator::operator==(const directory_iterator& _Other) const noexcept {
        return _Myimpl == _Other._Myimpl;
    }

    bool directory_iterator::operator!=(const directory_iterator& _Other) const noexcept {
        return _Myimpl != _Other._Myimpl;
    }

    directory_iterator::reference directory_iterator::operator*() const {
#ifdef _DEBUG
        if (!_Myimpl) {
            ::abort();
        }
#endif // _DEBUG

        return _Myimpl->_Entry;
    }

    directory_iterator::pointer directory_iterator::operator->() const {
        return &**this;
    }

    directory_iterator& directory_iterator::operator++() {
        if (_Myimpl) {
            if (!_Myimpl->_Advance()) {
                _Myimpl.reset();
            }
        }

        return *this;
    }

    directory_iterator begin(directory_iterator _Iter) noexcept {
        return _Iter;
    }

    directory_iterator end(directory_iterator) noexcept {
        return directory_iterator{};
    }

    recursive_directory_iterator::recursive_directory_iterator() noexcept : _Myimpl(nullptr) {}

    recursive_directory_iterator::recursive_directory_iterator(const path& _Target)
        : _Myimpl(::std::make_shared<details::_Recursive_dir_iter>(_Target, directory_options::none)) {
        if (_Myimpl) {
            if (!_Myimpl->_Skip_dots()) {
                _Myimpl.reset();
            }
        }
    }

    recursive_directory_iterator::recursive_directory_iterator(
        const path& _Target, const directory_options _Options)
        : _Myimpl(::std::make_shared<details::_Recursive_dir_iter>(_Target, _Options)) {
        if (_Myimpl) {
            if (!_Myimpl->_Skip_dots()) {
                _Myimpl.reset();
            }
        }
    }

    recursive_directory_iterator::~recursive_directory_iterator() noexcept {}

    bool recursive_directory_iterator::operator==(const recursive_directory_iterator& _Other) const noexcept {
        return _Myimpl == _Other._Myimpl;
    }

    bool recursive_directory_iterator::operator!=(const recursive_directory_iterator& _Other) const noexcept {
        return _Myimpl != _Other._Myimpl;
    }

    recursive_directory_iterator::reference recursive_directory_iterator::operator*() const {
#ifdef _DEBUG
        if (!_Myimpl) {
            ::abort();
        }
#endif // _DEBUG

        return _Myimpl->_Entry;
    }

    recursive_directory_iterator::pointer recursive_directory_iterator::operator->() const {
        return &**this;
    }

    recursive_directory_iterator& recursive_directory_iterator::operator++() {
        if (_Myimpl) {
            if (!_Myimpl->_Advance()) {
                _Myimpl.reset();
            }
        }

        return *this;
    }

    directory_options recursive_directory_iterator::options() const noexcept {
        return _Myimpl ? _Myimpl->_Options : directory_options::none;
    }

    int recursive_directory_iterator::depth() const noexcept {
        return _Myimpl ? static_cast<int>(_Myimpl->_Stack.size()) : 0;
    }

    bool recursive_directory_iterator::recursion_pending() const noexcept {
        return _Myimpl ? _Myimpl->_Recursion_pending : false;
    }

    void recursive_directory_iterator::disable_recursion_pending() noexcept {
        if (_Myimpl) {
            _Myimpl->_Recursion_pending = false;
        }
    }

    bool recursive_directory_iterator::pop() noexcept {
        return _Myimpl ? _Myimpl->_Pop() : false;
    }

    recursive_directory_iterator begin(recursive_directory_iterator _Iter) noexcept {
        return _Iter;
    }

    recursive_directory_iterator end(recursive_directory_iterator) noexcept {
        return recursive_directory_iterator{};
    }

    bool create_directory(const path& _Path) {
        return ::CreateDirectoryW(_Path.c_str(), nullptr) != 0;
    }

    bool remove_directory(const path& _Target) {
        return ::RemoveDirectoryW(_Target.c_str()) != 0;
    }
} // namespace mjfs