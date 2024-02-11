// path.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <mjfs/impl/path.hpp>
#include <mjfs/path.hpp>

namespace mjx {
    path::path() noexcept : _Mystr() {}

    path::path(const path& _Other) : _Mystr(_Other._Mystr) {}

    path::path(path&& _Other) noexcept : _Mystr(::std::move(_Other._Mystr)) {}

    path::path(const value_type* const _Str, format _Fmt) : _Mystr(_Str) {
        _Apply_format(_Fmt);
    }

    path::path(string_type&& _Str, format _Fmt) noexcept : _Mystr(::std::move(_Str)) {
        _Apply_format(_Fmt);
    }

    path::~path() noexcept {}

    path& path::operator=(const path& _Other) {
        if (this != ::std::addressof(_Other)) {
            _Mystr = _Other._Mystr;
        }

        return *this;
    }

    path& path::operator=(path&& _Other) noexcept {
        if (this != ::std::addressof(_Other)) {
            _Mystr = ::std::move(_Other._Mystr);
        }

        return *this;
    }

    void path::_Apply_format(const format _Fmt) noexcept {
        switch (_Fmt) {
        case generic_format:
            _Replace_slashes_with(L'\\', L'/');
            break;
        default:
            _Replace_slashes_with(L'/', L'\\');
            break;
        }
    }

    void path::_Replace_slashes_with(const wchar_t _Slash, const wchar_t _Replacement) noexcept {
        for (value_type& _Ch : _Mystr) {
            if (_Ch == _Slash) {
                _Ch = _Replacement;
            }
        }
    }
    
    path::operator string_type() const {
        return _Mystr;
    }

    path& path::assign(string_type&& _Str) noexcept {
        _Mystr = ::std::move(_Str);
        return *this;
    }

    path& path::operator/=(const path& _Other) {
        if (_Other.empty()) { // nothing to append, do nothing
            return *this;
        }

        if (_Mystr.empty()) { // replace with any path
            _Mystr = _Other._Mystr;
            return *this;
        }

        if (_Other.is_absolute()) { // replace with an absolute path
            _Mystr = _Other._Mystr;
            return *this;
        }

        if (!mjfs_impl::_Is_slash(_Mystr.back()) && !mjfs_impl::_Is_slash(_Other._Mystr.front())) {
            _Mystr.push_back(preferred_separator);
        }

        _Mystr += _Other._Mystr;
        return *this;
    }

    path& path::operator+=(const path& _Other) {
        _Mystr += _Other._Mystr;
        return *this;
    }

    path& path::operator+=(const string_type& _Str) {
        _Mystr += _Str;
        return *this;
    }

    path& path::operator+=(const unicode_string_view _Str) {
        _Mystr += _Str;
        return *this;
    }

    path& path::operator+=(const value_type* const _Str) {
        _Mystr += _Str;
        return *this;
    }

    path& path::operator+=(const value_type _Ch) {
        _Mystr.push_back(_Ch);
        return *this;
    }

    void path::clear() noexcept {
        _Mystr.clear();
    }

    path& path::make_preferred() noexcept {
        _Replace_slashes_with(L'/', preferred_separator);
        return *this;
    }

    path& path::remove_filename() {
        const mjfs_impl::_Path_segment& _Filename = mjfs_impl::_Find_filename(_Mystr);
        if (_Filename._Found()) {
            _Mystr.erase(_Filename._Offset);
        }

        return *this;
    }

    path& path::replace_filename(const path& _Replacement) {
        remove_filename();
        return *this /= _Replacement;
    }

    path& path::replace_extension(const path& _Replacement) {
        const size_t _Length = mjfs_impl::_Get_extension(_Mystr).size();
        if (_Length > 0) {
            _Mystr.erase(_Mystr.size() - _Length);
        }

        if (!_Replacement.empty()) { // append new extension
            const unicode_string_view _Str = _Replacement.native();
            if (!_Str.starts_with(L'.')) { // put a dot between filename and extension
                _Mystr.push_back(L'.');
            }

            _Mystr.append(_Str);
        }

        return *this;
    }

    void path::swap(path& _Other) noexcept {
        _Mystr.swap(_Other._Mystr);
    }

    const path::value_type* path::c_str() const noexcept {
        return _Mystr.c_str();
    }

    const path::string_type& path::native() const noexcept {
        return _Mystr;
    }

    bool path::empty() const noexcept {
        return _Mystr.empty();
    }

    path path::root_name() const noexcept {
        return mjfs_impl::_Get_root_name(_Mystr);
    }

    path path::root_directory() const noexcept {
        return mjfs_impl::_Get_root_directory(_Mystr);
    }

    path path::root_path() const noexcept {
        return mjfs_impl::_Get_root_path(_Mystr);
    }

    path path::relative_path() const noexcept {
        return mjfs_impl::_Get_relative_path(_Mystr);
    }

    path path::parent_path() const noexcept {
        return mjfs_impl::_Get_parent_path(_Mystr);
    }

    path path::filename() const noexcept {
        return mjfs_impl::_Get_filename(_Mystr);
    }

    path path::stem() const noexcept {
        return mjfs_impl::_Get_stem(_Mystr);
    }

    path path::extension() const noexcept {
        return mjfs_impl::_Get_extension(_Mystr);
    }

    bool path::has_root_name() const noexcept {
        return !mjfs_impl::_Get_root_name(_Mystr).empty();
    }

    bool path::has_root_directory() const noexcept {
        return !mjfs_impl::_Get_root_directory(_Mystr).empty();
    }

    bool path::has_root_path() const noexcept {
        return !mjfs_impl::_Get_root_path(_Mystr).empty();
    }

    bool path::has_relative_path() const noexcept {
        return !mjfs_impl::_Get_relative_path(_Mystr).empty();
    }

    bool path::has_parent_path() const noexcept {
        return !mjfs_impl::_Get_parent_path(_Mystr).empty();
    }

    bool path::has_filename() const noexcept {
        return !mjfs_impl::_Get_filename(_Mystr).empty();
    }

    bool path::has_stem() const noexcept {
        return !mjfs_impl::_Get_stem(_Mystr).empty();
    }

    bool path::has_extension() const noexcept {
        return !mjfs_impl::_Get_extension(_Mystr).empty();
    }

    bool path::is_absolute() const noexcept {
        return mjfs_impl::_Has_drive_and_slash(_Mystr);
    }

    bool path::is_relative() const noexcept {
        return !is_absolute();
    }

    path::iterator path::begin() const {
        if (_Mystr.empty()) {
            return iterator{nullptr}; // equal to end()
        }
        
        if (mjfs_impl::_Has_drive(_Mystr)) { // extract root-name
            return iterator{this, _Mystr.view().substr(0, 2)};
        } else if (mjfs_impl::_Is_slash(_Mystr[0])) { // extract root-directory
            return iterator{this, _Mystr.view().substr(0, 1)};
        } else { // extract the first element
            // Note: When the path doesn't start with a root-name nor root-directory, the first
            //       element of the path is extracted. In this context, a path element is defined
            //       as the portion between slashes. For example, in the path "foo\bar\", the elements
            //       are "foo" and "bar". We skip the first character, as we've already checked it
            //       for the presence of a slash.
            const size_t _Slash = mjfs_impl::_Find_first_slash(_Mystr);
            if (_Slash != unicode_string_view::npos) {
                return iterator{this, _Mystr.view().substr(0, _Slash)};
            } else {
                return iterator{this, _Mystr.view()};
            }
        }
    }

    path::iterator path::end() const {
        return iterator{nullptr};
    }

    bool operator==(const path& _Left, const path& _Right) {
        return _Left.native() == _Right.native();
    }

    bool operator!=(const path& _Left, const path& _Right) {
        return _Left.native() != _Right.native();
    }

    path operator/(const path& _Left, const path& _Right) {
        path _Result = _Left;
        _Result     /= _Right;
        return _Result;
    }

    path_iterator::path_iterator() noexcept : _Mypath(nullptr), _Myelem(), _Myoff(0) {}

    path_iterator::path_iterator(const path_iterator& _Other)
        : _Mypath(_Other._Mypath), _Myelem(_Other._Myelem), _Myoff(_Other._Myoff) {}

    path_iterator::path_iterator(path_iterator&& _Other) noexcept
        : _Mypath(_Other._Mypath), _Myelem(::std::move(_Other._Myelem)), _Myoff(_Other._Myoff) {
        _Other._Mypath = nullptr; // mark as unusable
        _Other._Myoff  = 0;
    }

    path_iterator::path_iterator(const path* const _Path) noexcept : _Mypath(_Path), _Myelem(), _Myoff(0) {}

    path_iterator::path_iterator(const path* const _Path, path&& _Element, const size_t _Off) noexcept
        : _Mypath(_Path), _Myelem(::std::move(_Element)), _Myoff(_Off) {}

    path_iterator::~path_iterator() noexcept {}

    path_iterator& path_iterator::operator=(const path_iterator& _Other) {
        if (this != ::std::addressof(_Other)) {
            _Mypath = _Other._Mypath;
            _Myelem = _Other._Myelem;
            _Myoff  = _Other._Myoff;
        }

        return *this;
    }

    path_iterator& path_iterator::operator=(path_iterator&& _Other) noexcept {
        if (this != ::std::addressof(_Other)) {
            _Mypath        = _Other._Mypath;
            _Myelem        = ::std::move(_Other._Myelem);
            _Myoff         = _Other._Myoff;
            _Other._Mypath = nullptr;
            _Other._Myoff  = 0;
        }

        return *this;
    }

    bool path_iterator::_Has_more_elements() const noexcept {
        const size_t _Path_size    = _Mypath->native().size();
        const wchar_t* _First      = _Mypath->c_str() + _Myoff;
        const wchar_t* const _Last = _First + (_Path_size - _Myoff);
        for (; _First != _Last; ++_First) {
            if (!mjfs_impl::_Is_slash(*_First)) { // search for non-slash characters
                return true;
            }
        }

        return false;
    }

    path_iterator& path_iterator::operator++() {
        if (!_Mypath) {
            return *this;
        }

        const unicode_string_view _Path_str = _Mypath->native().view();
        const unicode_string_view _Elem_str = _Myelem.native().view();
        const size_t _Path_size             = _Path_str.size();
        const size_t _Elem_size             = _Elem_str.size();
        if (mjfs_impl::_Has_drive(_Elem_str) && _Elem_size == 2) { // current element is root-name
            _Myoff = 2; // skip root-name (fixed length)
            if (_Path_size > 2 && mjfs_impl::_Is_slash(_Path_str[2])) { // advance to root-directory
                _Myelem = _Path_str.substr(_Myoff, 1);
                return *this;
            }
        } else if (mjfs_impl::_Is_slash(_Elem_str[0])
            && _Elem_size == 1) { // current element is root-directory
            ++_Myoff; // skip root-directory (fixed length)
        } else {
            _Myoff += _Elem_size; // skip the current element
        }

        if (_Myoff >= _Path_size || !_Has_more_elements()) { // we reached the end of the path
            _Mypath = nullptr;
            _Myoff  = 0;
            _Myelem.clear();
            return *this;
        }

        while (mjfs_impl::_Is_slash(_Path_str[_Myoff])) { // skip optional slashes between elements
            ++_Myoff;
        }

        const unicode_string_view _Path_substr = _Path_str.substr(_Myoff, _Path_size - _Myoff);
        const size_t _Slash                    = mjfs_impl::_Find_first_slash(_Path_substr);
        if (_Slash != unicode_string_view::npos) { // advance to the last element
            _Myelem = _Path_substr.substr(0, _Slash);
        } else { // advance to the next element
            _Myelem = _Path_substr;
        }

        return *this;
    }

    path_iterator path_iterator::operator++(int) {
        path_iterator _Temp = *this;
        ++*this;
        return _Temp;
    }

    path_iterator::reference path_iterator::operator*() const noexcept {
        return _Myelem;
    }

    path_iterator::pointer path_iterator::operator->() const noexcept {
        return ::std::addressof(_Myelem);
    }

    bool operator==(const path_iterator& _Left, const path_iterator& _Right) {
        return _Left._Myelem == _Right._Myelem && _Left._Myoff == _Right._Myoff;
    }

    bool operator!=(const path_iterator& _Left, const path_iterator& _Right) {
        return !(_Left == _Right);
    }

    path current_path() {
        const size_t _Buf_size = mjfs_impl::_Get_current_path_length();
        path::string_type _Buf(_Buf_size, L'\0');
        return mjfs_impl::_Get_current_path(_Buf.data(), _Buf_size) ? path{::std::move(_Buf)} : path{};
    }

    bool current_path(const path& _New_path) {
        return ::SetCurrentDirectoryW(_New_path.c_str()) != 0;
    }
} // namespace mjx