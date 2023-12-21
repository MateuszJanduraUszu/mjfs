// path_iterator.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _MJFS_TEST_UNIT_PATH_ITERATOR_HPP_
#define _MJFS_TEST_UNIT_PATH_ITERATOR_HPP_
#include <gtest/gtest.h>
#include <mjfs/path.hpp>
#include <vector>

namespace mjx {
    namespace test {
        TEST(path_iterator, absolute_path) {
            const path _Path(LR"(C:\foo\bar\meow)");
            ::std::vector<path> _Elements;
            for (const path& _Element : _Path) {
                _Elements.push_back(_Element);
            }

            const ::std::vector<path> _Expected_elements = {L"C:", L"\\", L"foo", L"bar", L"meow"};
            EXPECT_EQ(_Elements, _Expected_elements);
        }

        TEST(path_iterator, dirty_absolute_path) {
            const path _Path(LR"(C:\\\\foo\\bar\\\meow\\\\\)");
            ::std::vector<path> _Elements;
            for (const path& _Element : _Path) {
                _Elements.push_back(_Element);
            }

            const ::std::vector<path> _Expected_elements = {L"C:", L"\\", L"foo", L"bar", L"meow"};
            EXPECT_EQ(_Elements, _Expected_elements);
        }

        TEST(path_iterator, absolute_path_without_root_name) {
            const path _Path(LR"(C:foo\bar\meow)");
            ::std::vector<path> _Elements;
            for (const path& _Element : _Path) {
                _Elements.push_back(_Element);
            }

            const ::std::vector<path> _Expected_elements = {L"C:", L"foo", L"bar", L"meow"};
            EXPECT_EQ(_Elements, _Expected_elements);
        }

        TEST(path_iterator, absolute_path_without_root_directory) {
            const path _Path(LR"(\foo\bar\meow)");
            ::std::vector<path> _Elements;
            for (const path& _Element : _Path) {
                _Elements.push_back(_Element);
            }

            const ::std::vector<path> _Expected_elements = {L"\\", L"foo", L"bar", L"meow"};
            EXPECT_EQ(_Elements, _Expected_elements);
        }

        TEST(path_iterator, relative_path) {
            const path _Path(LR"(foo\bar\meow)");
            ::std::vector<path> _Elements;
            for (const path& _Element : _Path) {
                _Elements.push_back(_Element);
            }

            const ::std::vector<path> _Expected_elements = {L"foo", L"bar", L"meow"};
            EXPECT_EQ(_Elements, _Expected_elements);
        }

        TEST(path_iterator, dirty_relative_path) {
            const path _Path(LR"(foo\\\bar\\\\meow\\)");
            ::std::vector<path> _Elements;
            for (const path& _Element : _Path) {
                _Elements.push_back(_Element);
            }

            const ::std::vector<path> _Expected_elements = {L"foo", L"bar", L"meow"};
            EXPECT_EQ(_Elements, _Expected_elements);
        }
    } // namespace test
} // namespace mjx

#endif // _MJFS_TEST_UNIT_PATH_ITERATOR_HPP_