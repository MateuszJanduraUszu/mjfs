// path.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <mjfs/details/path.hpp>
#include <mjfs/path.hpp>

namespace mjfs {
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

        if (!details::_Is_slash(_Mystr.back()) && !details::_Is_slash(_Other._Mystr.front())) {
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

    path& path::operator+=(const ::std::wstring_view _Str) {
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

    path& path::remove_filename() noexcept {
        const details::_Path_segment& _Filename = details::_Find_filename(_Mystr);
        if (_Filename._Found()) {
            // Note: Use std::wstring::erase(const_iterator, const_iterator) to maintain the
            //       noexcept specifier, as this function throws nothing and is marked noexcept.
            _Mystr.erase(_Mystr.begin() + _Filename._Offset, _Mystr.end());
        }

        return *this;
    }

    path& path::replace_filename(const path& _Replacement) {
        remove_filename();
        return *this /= _Replacement;
    }

    path& path::replace_extension(const path& _Replacement) {
        const size_t _Length = details::_Get_extension(_Mystr).size();
        if (_Length > 0) {
            // Note: Use std::wstring::erase(const_iterator, const_iterator) to maintain the
            //       noexcept specifier, as this function throws nothing and is marked noexcept.
            _Mystr.erase(_Mystr.begin() + (_Mystr.size() - _Length), _Mystr.end());
        }

        return *this /= _Replacement;
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

    path path::root_name() const {
        return details::_Get_root_name(_Mystr);
    }

    path path::root_directory() const {
        return details::_Get_root_directory(_Mystr);
    }

    path path::root_path() const {
        return details::_Get_root_path(_Mystr);
    }

    path path::relative_path() const {
        return details::_Get_relative_path(_Mystr);
    }

    path path::parent_path() const {
        return details::_Get_parent_path(_Mystr);
    }

    path path::filename() const {
        return details::_Get_filename(_Mystr);
    }

    path path::stem() const {
        return details::_Get_stem(_Mystr);
    }

    path path::extension() const {
        return details::_Get_extension(_Mystr);
    }

    bool path::has_root_name() const noexcept {
        return !details::_Get_root_name(_Mystr).empty();
    }

    bool path::has_root_directory() const noexcept {
        return !details::_Get_root_directory(_Mystr).empty();
    }

    bool path::has_root_path() const noexcept {
        return !details::_Get_root_path(_Mystr).empty();
    }

    bool path::has_relative_path() const noexcept {
        return !details::_Get_relative_path(_Mystr).empty();
    }

    bool path::has_parent_path() const noexcept {
        return !details::_Get_parent_path(_Mystr).empty();
    }

    bool path::has_filename() const noexcept {
        return !details::_Get_filename(_Mystr).empty();
    }

    bool path::has_stem() const noexcept {
        return !details::_Get_stem(_Mystr).empty();
    }

    bool path::has_extension() const noexcept {
        return !details::_Get_extension(_Mystr).empty();
    }

    bool path::is_absolute() const noexcept {
        return details::_Has_drive_and_slash(_Mystr);
    }

    bool path::is_relative() const noexcept {
        return !is_absolute();
    }

    path::iterator path::begin() const noexcept {
        return _Mystr.begin();
    }

    path::iterator path::end() const noexcept {
        return _Mystr.end();
    }

    bool operator==(const path& _Left, const path& _Right) noexcept {
        return _Left.native() == _Right.native();
    }

    bool operator!=(const path& _Left, const path& _Right) noexcept {
        return _Left.native() != _Right.native();
    }

    path operator/(const path& _Left, const path& _Right) {
        path _Result = _Left;
        _Result     /= _Right;
        return _Result;
    }

    path current_path() {
        const size_t _Buf_size = details::_Get_current_path_length();
        path::string_type _Buf(_Buf_size, L'\0');
        return details::_Get_current_path(_Buf.data(), _Buf_size) ? path{::std::move(_Buf)} : path{};
    }

    bool current_path(const path& _New_path) {
        return ::SetCurrentDirectoryW(_New_path.c_str()) != 0;
    }
} // namespace mjfs