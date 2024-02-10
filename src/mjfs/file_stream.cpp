// file_stream.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <mjfs/file_stream.hpp>
#include <mjfs/impl/file_stream.hpp>
#include <type_traits>

namespace mjx {
    file_stream::file_stream() noexcept : _Myfile(nullptr) {}

    file_stream::file_stream(file_stream&& _Other) noexcept : _Myfile(_Other._Myfile) {
        _Other._Myfile = nullptr;
    }

    file_stream::file_stream(file& _File) noexcept : _Myfile(::std::addressof(_File)) {}

    file_stream::~file_stream() noexcept {}

    file_stream& file_stream::operator=(file_stream&& _Other) noexcept {
        if (this != ::std::addressof(_Other)) {
            _Myfile        = _Other._Myfile;
            _Other._Myfile = nullptr;
        }
        
        return *this;
    }

    bool file_stream::is_open() const noexcept {
        return _Myfile != nullptr && _Myfile->is_open();
    }

    void file_stream::close() noexcept {
        _Myfile = nullptr;
    }

    void file_stream::bind_file(file& _New_file) noexcept {
        _Myfile = ::std::addressof(_New_file);
    }

    file_stream::pos_type file_stream::tell() const noexcept {
        return is_open() ? mjfs_impl::_Tell_file(_Myfile->native_handle()) : false;
    }

    bool file_stream::seek(const pos_type _New_pos) noexcept {
        return is_open() ? mjfs_impl::_Seek_file(_Myfile->native_handle(), _New_pos) : false;
    }

    bool file_stream::seek_to_end() noexcept {
        return is_open() ? mjfs_impl::_Seek_file_to_end(_Myfile->native_handle()) : false;
    }

    bool file_stream::move(const off_type _Off, const move_direction _Direction) noexcept {
        if (!is_open()) {
            return false;
        }

        if (_Off == 0) { // no movement, do nothing
            return true;
        }

        const pos_type _Old_pos = mjfs_impl::_Tell_file(_Myfile->native_handle());
        const pos_type _New_pos = _Direction == move_direction::backward
            ? _Old_pos - _Off : _Old_pos + _Off;
        return mjfs_impl::_Seek_file(_Myfile->native_handle(), _New_pos);
    }

    file_stream::int_type file_stream::read(char_type* const _Buf, const int_type _Count) noexcept {
        if (!is_open()) {
            return 0;
        }

        if (_Count == 0 || !_Buf) { // nothing to read or invalid buffer, break
            return 0;
        }

        return mjfs_impl::_Read_file(_Myfile->native_handle(), _Buf, _Count);
    }

    file_stream::int_type file_stream::read(byte_string& _Buf) noexcept {
        return read(_Buf.data(), _Buf.size());
    }

    bool file_stream::read_exactly(char_type* const _Buf, const int_type _Count) noexcept {
        return read(_Buf, _Count) == _Count;
    }

    bool file_stream::read_exactly(byte_string& _Buf) noexcept {
        return read(_Buf) == _Buf.size();
    }

    bool file_stream::write(const char_type* const _Data, const int_type _Count) noexcept {
        if (!is_open()) {
            return false;
        }

        if (_Count == 0) { // no data to write, do nothing
            return true;
        }

        if (!_Data) { // invalid buffer, break
            return false;
        }

        return mjfs_impl::_Write_file(_Myfile->native_handle(), _Data, _Count);
    }

    bool file_stream::write(const byte_string_view _Data) noexcept {
        return write(_Data.data(), _Data.size());
    }

    bool file_stream::flush() noexcept {
        return is_open() ? ::FlushFileBuffers(_Myfile->native_handle()) != 0 : false;
    }
} // namespace mjx