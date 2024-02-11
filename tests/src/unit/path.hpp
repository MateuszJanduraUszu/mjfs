// path.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _MJFS_TEST_UNIT_PATH_HPP_
#define _MJFS_TEST_UNIT_PATH_HPP_
#include <gtest/gtest.h>
#include <mjfs/path.hpp>

namespace mjx {
    namespace test {
        TEST(path, append) {
            EXPECT_EQ(path(L"//host") / L"foo", L"//host/foo");
            EXPECT_EQ(path(L"//host/") / L"foo", L"//host/foo");
            EXPECT_EQ(path(L"foo") / L"C:/bar", L"C:/bar");
        }

        TEST(path, make_preferred) {
            EXPECT_EQ(path(L"a/b/c").make_preferred(), L"a\\b\\c");
            EXPECT_EQ(path(L"a\\b\\c").make_preferred(), L"a\\b\\c");
        }

        TEST(path, remove_filename) {
            EXPECT_EQ(path(L"foo/bar").remove_filename(), path(L"foo/"));
            EXPECT_FALSE(path(L"foo/bar").remove_filename().has_filename());

            EXPECT_EQ(path(L"foo/").remove_filename(), path(L"foo/"));
            EXPECT_FALSE(path(L"foo/").remove_filename().has_filename());

            EXPECT_EQ(path(L"/foo").remove_filename(), path(L"/"));
            EXPECT_FALSE(path(L"/foo").remove_filename().has_filename());

            EXPECT_EQ(path(L"/").remove_filename(), path(L"/"));
            EXPECT_FALSE(path(L"/").remove_filename().has_filename());

            EXPECT_EQ(path(L"").remove_filename(), path(L""));
            EXPECT_FALSE(path(L"").remove_filename().has_filename());
        }

        TEST(path, replace_filename) {
            EXPECT_EQ(path(L"/foo").replace_filename(L"bar"), L"/bar");
            EXPECT_EQ(path(L"/").replace_filename(L"bar"), L"/bar");
            EXPECT_EQ(path(L"").replace_filename(L"pub"), L"pub");
        }

        TEST(path, replace_extension) {
            EXPECT_EQ(path(L"/foo/bar.jpg").replace_extension(L".png"), L"/foo/bar.png");
            EXPECT_EQ(path(L"/foo/bar.jpg").replace_extension(L"png"), L"/foo/bar.png");
            EXPECT_EQ(path(L"/foo/bar.jpg").replace_extension(L"."), L"/foo/bar.");
            EXPECT_EQ(path(L"/foo/bar.jpg").replace_extension(L""), L"/foo/bar");
            EXPECT_EQ(path(L"/foo/bar").replace_extension(L".png"), L"/foo/bar.png");
            EXPECT_EQ(path(L"/foo/bar").replace_extension(L"png"), L"/foo/bar.png");
            EXPECT_EQ(path(L"/foo/bar").replace_extension(L"."), L"/foo/bar.");
            EXPECT_EQ(path(L"/foo/bar").replace_extension(L""), L"/foo/bar");
            EXPECT_EQ(path(L"/foo/.").replace_extension(L".png"), L"/foo/..png");
            EXPECT_EQ(path(L"/foo/.").replace_extension(L"png"), L"/foo/..png");
            EXPECT_EQ(path(L"/foo/.").replace_extension(L"."), L"/foo/..");
            EXPECT_EQ(path(L"/foo/.").replace_extension(L""), L"/foo/.");
            EXPECT_EQ(path(L"/foo/").replace_extension(L".png"), L"/foo/.png");
            EXPECT_EQ(path(L"/foo/").replace_extension(L"png"), L"/foo/.png");
        }

        TEST(path, root_name) {
            EXPECT_EQ(path(L"C:\\Windows\\System32\\").root_name(), path(L"C:"));
            EXPECT_FALSE(path(L"/foo/bar.txt").has_root_name());
        }

        TEST(path, root_directory) {
            EXPECT_EQ(path(L"C:\\Users\\Xyz").root_directory(), path(L"\\"));
            EXPECT_EQ(path(L"\\foo\\bar.txt").root_directory(), path(L"\\"));
            EXPECT_EQ(path(L"/foo/bar/baz.txt").root_directory(), path(L"/"));
            EXPECT_FALSE(path(L"foo/bar/baz/").has_root_directory());
        }

        TEST(path, root_path) {
            EXPECT_EQ(path(L"C:\\Users\\Xyz").root_path(), path(L"C:\\"));
            EXPECT_EQ(path(L"/foo/bar").root_path(), path(L"/"));
            EXPECT_FALSE(path(L"foo/bar/baz/").has_root_path());
        }

        TEST(path, relative_path) {
            EXPECT_EQ(path(L"C:\\Users\\Xyz").relative_path(), path(L"Users\\Xyz"));
            EXPECT_EQ(path(L"/foo/bar").relative_path(), path(L"foo/bar"));
            EXPECT_EQ(path(L"foo/bar.txt").relative_path(), path(L"foo/bar.txt"));
        }

        TEST(path, parent_path) {
            EXPECT_EQ(path(L"/var/tmp/example.txt").parent_path(), path(L"/var/tmp"));
            EXPECT_EQ(path(L"/").parent_path(), path(L"/"));
            EXPECT_EQ(path(L"/var/tmp/.").parent_path(), path(L"/var/tmp"));
        }

        TEST(path, filename) {
            EXPECT_EQ(path(L"/foo/bar.txt").filename(), path(L"bar.txt"));
            EXPECT_EQ(path(L"/foo/.bar").filename(), path(L".bar"));
            EXPECT_EQ(path(L"/foo/.").filename(), path(L"."));
            EXPECT_EQ(path(L"/foo/..").filename(), path(L".."));
            EXPECT_EQ(path(L".").filename(), path(L"."));
            EXPECT_EQ(path(L"..").filename(), path(L".."));
            EXPECT_EQ(path(L"//host").filename(), path(L"host"));
    
            EXPECT_FALSE(path(L"/foo/bar/").has_filename());
            EXPECT_FALSE(path(L"/").has_filename());
        }

        TEST(path, stem) {
            EXPECT_EQ(path(L"/foo/bar.txt").stem(), path(L"bar"));
            EXPECT_EQ(path(L"/foo/.bar").stem(), path(L".bar"));
            EXPECT_EQ(path(L"foo.bar.baz.tar").stem(), path(L"foo.bar.baz"));
        }

        TEST(path, extension) {
            EXPECT_EQ(path(L"/foo/bar.txt").extension(), path(L".txt"));
            EXPECT_EQ(path(L"/foo/bar.").extension(), path(L"."));
            EXPECT_EQ(path(L"/foo/bar.txt/bar.cc").extension(), path(L".cc"));
            EXPECT_EQ(path(L"/foo/bar.txt/bar.").extension(), path(L"."));
            EXPECT_EQ(path(L"/foo/..bar").extension(), path(L".bar"));
    
            EXPECT_FALSE(path(L"/foo/bar").has_extension());
            EXPECT_FALSE(path(L"/foo/bar.txt/bar").has_extension());
            EXPECT_FALSE(path(L"/foo/.").has_extension());
            EXPECT_FALSE(path(L"/foo/..").has_extension());
            EXPECT_FALSE(path(L"/foo/.hidden").has_extension());
        }
    } // namespace test
} // namespace mjx

#endif // _MJFS_TEST_UNIT_PATH_HPP_