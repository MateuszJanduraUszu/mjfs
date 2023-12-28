// directory.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <cstdlib>
#include <mjfs/bitmask.hpp>
#include <mjfs/directory.hpp>
#include <mjfs/impl/directory.hpp>
#include <mjfs/impl/path.hpp>
#include <mjfs/impl/status.hpp>
#include <mjfs/impl/utils.hpp>
#include <mjmem/object_allocator.hpp>

namespace mjx {
    directory_entry::directory_entry(const path& _Target)
        : _Myattr(mjfs_impl::_Get_file_attributes(_Target.c_str())), _Mypath(_Target) {}

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
        _Myattr = mjfs_impl::_Get_file_attributes(_Mypath.c_str());
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

        return mjfs_impl::_Get_reparse_tag(_Mypath.c_str()) == mjfs_impl::_File_reparse_tag::_Symlink;
    }

    bool directory_entry::is_junction() const noexcept {
        if (!_Has_bits(_Myattr, file_attribute::reparse_point)) {
            return false;
        }

        return mjfs_impl::_Get_reparse_tag(_Mypath.c_str()) == mjfs_impl::_File_reparse_tag::_Mount_point;
    }

    _Any_dir_iter::~_Any_dir_iter() noexcept {
        // overrided by _Dir_iter and _Recursive_dir_iter
    }

    directory_iterator::directory_iterator() noexcept : _Myimpl(nullptr) {}

    directory_iterator::directory_iterator(const path& _Target)
        : _Myimpl(::mjx::create_object<mjfs_impl::_Dir_iter>(_Target)) {
        if (!_Myimpl->_Normal()->_Skip_dots()) {
            _Myimpl.reset();
        }
    }

    directory_iterator::directory_iterator(const path& _Target, const directory_options)
        : _Myimpl(::mjx::create_object<mjfs_impl::_Dir_iter>(_Target)) {
        if (!_Myimpl->_Normal()->_Skip_dots()) {
            _Myimpl.reset();
        }
    }

    directory_iterator::~directory_iterator() noexcept {}

    bool directory_iterator::operator==(const directory_iterator& _Other) const noexcept {
        return _Myimpl == _Other._Myimpl;
    }

    bool directory_iterator::operator!=(const directory_iterator& _Other) const noexcept {
        return _Myimpl != _Other._Myimpl;
    }

    directory_iterator::reference directory_iterator::operator*() const noexcept {
#ifdef _DEBUG
        _INTERNAL_ASSERT(_Myimpl != nullptr, "attempt to dereference invalid iterator");
#endif // _DEBUG
        return _Myimpl->_Normal()->_Entry;
    }

    directory_iterator::pointer directory_iterator::operator->() const noexcept {
        return &**this;
    }

    directory_iterator& directory_iterator::operator++() {
#ifdef _DEBUG
        _INTERNAL_ASSERT(_Myimpl != nullptr, "attempt to advance invalid iterator");
#endif // _DEBUG
        if (!_Myimpl->_Normal()->_Advance()) {
            _Myimpl.reset();
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
        : _Myimpl(::mjx::create_object<mjfs_impl::_Recursive_dir_iter>(_Target, directory_options::none)) {
        if (!_Myimpl->_Recursive()->_Skip_dots()) {
            _Myimpl.reset();
        }
    }

    recursive_directory_iterator::recursive_directory_iterator(
        const path& _Target, const directory_options _Options)
        : _Myimpl(::mjx::create_object<mjfs_impl::_Recursive_dir_iter>(_Target, _Options)) {
        if (!_Myimpl->_Recursive()->_Skip_dots()) {
            _Myimpl.reset();
        }
    }

    recursive_directory_iterator::~recursive_directory_iterator() noexcept {}

    bool recursive_directory_iterator::operator==(const recursive_directory_iterator& _Other) const noexcept {
        return _Myimpl == _Other._Myimpl;
    }

    bool recursive_directory_iterator::operator!=(const recursive_directory_iterator& _Other) const noexcept {
        return _Myimpl != _Other._Myimpl;
    }

    recursive_directory_iterator::reference recursive_directory_iterator::operator*() const noexcept {
#ifdef _DEBUG
        _INTERNAL_ASSERT(_Myimpl != nullptr, "attempt to dereference invalid iterator");
#endif // _DEBUG
        return _Myimpl->_Recursive()->_Entry;
    }

    recursive_directory_iterator::pointer recursive_directory_iterator::operator->() const noexcept {
        return &**this;
    }

    recursive_directory_iterator& recursive_directory_iterator::operator++() {
#ifdef _DEBUG
        _INTERNAL_ASSERT(_Myimpl != nullptr, "attempt to advance invalid iterator");
#endif // _DEBUG
        if (!_Myimpl->_Recursive()->_Advance()) {
            _Myimpl.reset();
        }

        return *this;
    }

    directory_options recursive_directory_iterator::options() const noexcept {
#ifdef _DEBUG
        _INTERNAL_ASSERT(_Myimpl != nullptr, "attempt to use invalid iterator");
#endif // _DEBUG
        return _Myimpl->_Recursive()->_Options;
    }

    int recursive_directory_iterator::depth() const noexcept {
#ifdef _DEBUG
        _INTERNAL_ASSERT(_Myimpl != nullptr, "attempt to use invalid iterator");
#endif // _DEBUG
        return static_cast<int>(_Myimpl->_Recursive()->_Stack.size());
    }

    bool recursive_directory_iterator::recursion_pending() const noexcept {
#ifdef _DEBUG
        _INTERNAL_ASSERT(_Myimpl != nullptr, "attempt to use invalid iterator");
#endif // _DEBUG
        return _Myimpl->_Recursive()->_Recursion_pending;
    }

    void recursive_directory_iterator::disable_recursion_pending() noexcept {
#ifdef _DEBUG
        _INTERNAL_ASSERT(_Myimpl != nullptr, "attempt to use invalid iterator");
#endif // _DEBUG
        _Myimpl->_Recursive()->_Recursion_pending = false;
    }

    void recursive_directory_iterator::pop() {
#ifdef _DEBUG
        _INTERNAL_ASSERT(_Myimpl != nullptr, "attempt to use invalid iterator");
#endif // _DEBUG
        _Myimpl->_Recursive()->_Pop();
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
} // namespace mjx