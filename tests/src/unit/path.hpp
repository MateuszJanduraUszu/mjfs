// path.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _MJFS_UNIT_PATH_HPP_
#define _MJFS_UNIT_PATH_HPP_
#include <gtest/gtest.h>
#include <mjfs/path.hpp>

TEST(path, remove_filename) {
    EXPECT_EQ(::mjfs::path(L"foo/bar").remove_filename(), ::mjfs::path(L"foo/"));
    EXPECT_FALSE(::mjfs::path(L"foo/bar").remove_filename().has_filename());

    EXPECT_EQ(::mjfs::path(L"foo/").remove_filename(), ::mjfs::path(L"foo/"));
    EXPECT_FALSE(::mjfs::path(L"foo/").remove_filename().has_filename());

    EXPECT_EQ(::mjfs::path(L"/foo").remove_filename(), ::mjfs::path(L"/"));
    EXPECT_FALSE(::mjfs::path(L"/foo").remove_filename().has_filename());

    EXPECT_EQ(::mjfs::path(L"/").remove_filename(), ::mjfs::path(L"/"));
    EXPECT_FALSE(::mjfs::path(L"/").remove_filename().has_filename());

    EXPECT_EQ(::mjfs::path(L"").remove_filename(), ::mjfs::path(L""));
    EXPECT_FALSE(::mjfs::path(L"").remove_filename().has_filename());
}

TEST(path, root_name) {
    EXPECT_EQ(::mjfs::path(L"C:\\Windows\\System32\\").root_name(), ::mjfs::path(L"C:"));
    EXPECT_FALSE(::mjfs::path(L"/foo/bar.txt").has_root_name());
}

TEST(path, root_directory) {
    EXPECT_EQ(::mjfs::path(L"C:\\Users\\Xyz").root_directory(), ::mjfs::path(L"\\"));
    EXPECT_EQ(::mjfs::path(L"\\foo\\bar.txt").root_directory(), ::mjfs::path(L"\\"));
    EXPECT_EQ(::mjfs::path(L"/foo/bar/baz.txt").root_directory(), ::mjfs::path(L"/"));
    EXPECT_FALSE(::mjfs::path(L"foo/bar/baz/").has_root_directory());
}

TEST(path, root_path) {
    EXPECT_EQ(::mjfs::path(L"C:\\Users\\Xyz").root_path(), ::mjfs::path(L"C:\\"));
    EXPECT_EQ(::mjfs::path(L"/foo/bar").root_path(), ::mjfs::path(L"/"));
    EXPECT_FALSE(::mjfs::path(L"foo/bar/baz/").has_root_path());
}

TEST(path, relative_path) {
    EXPECT_EQ(::mjfs::path(L"C:\\Users\\Xyz").relative_path(), ::mjfs::path(L"Users\\Xyz"));
    EXPECT_EQ(::mjfs::path(L"/foo/bar").relative_path(), ::mjfs::path(L"foo/bar"));
    EXPECT_EQ(::mjfs::path(L"foo/bar.txt").relative_path(), ::mjfs::path(L"foo/bar.txt"));
}

TEST(path, parent_path) {
    EXPECT_EQ(::mjfs::path(L"/var/tmp/example.txt").parent_path(), ::mjfs::path(L"/var/tmp"));
    EXPECT_EQ(::mjfs::path(L"/").parent_path(), ::mjfs::path(L"/"));
    EXPECT_EQ(::mjfs::path(L"/var/tmp/.").parent_path(), ::mjfs::path(L"/var/tmp"));
}

TEST(path, filename) {
    EXPECT_EQ(::mjfs::path(L"/foo/bar.txt").filename(), ::mjfs::path(L"bar.txt"));
    EXPECT_EQ(::mjfs::path(L"/foo/.bar").filename(), ::mjfs::path(L".bar"));
    EXPECT_EQ(::mjfs::path(L"/foo/.").filename(), ::mjfs::path(L"."));
    EXPECT_EQ(::mjfs::path(L"/foo/..").filename(), ::mjfs::path(L".."));
    EXPECT_EQ(::mjfs::path(L".").filename(), ::mjfs::path(L"."));
    EXPECT_EQ(::mjfs::path(L"..").filename(), ::mjfs::path(L".."));
    EXPECT_EQ(::mjfs::path(L"//host").filename(), ::mjfs::path(L"host"));
    
    EXPECT_FALSE(::mjfs::path(L"/foo/bar/").has_filename());
    EXPECT_FALSE(::mjfs::path(L"/").has_filename());
}

TEST(path, stem) {
    EXPECT_EQ(::mjfs::path(L"/foo/bar.txt").stem(), ::mjfs::path(L"bar"));
    EXPECT_EQ(::mjfs::path(L"/foo/.bar").stem(), ::mjfs::path(L".bar"));
    EXPECT_EQ(::mjfs::path(L"foo.bar.baz.tar").stem(), ::mjfs::path(L"foo.bar.baz"));
}

TEST(path, extension) {
    EXPECT_EQ(::mjfs::path(L"/foo/bar.txt").extension(), ::mjfs::path(L".txt"));
    EXPECT_EQ(::mjfs::path(L"/foo/bar.").extension(), ::mjfs::path(L"."));
    EXPECT_EQ(::mjfs::path(L"/foo/bar.txt/bar.cc").extension(), ::mjfs::path(L".cc"));
    EXPECT_EQ(::mjfs::path(L"/foo/bar.txt/bar.").extension(), ::mjfs::path(L"."));
    EXPECT_EQ(::mjfs::path(L"/foo/..bar").extension(), ::mjfs::path(L".bar"));
    
    EXPECT_FALSE(::mjfs::path(L"/foo/bar").has_extension());
    EXPECT_FALSE(::mjfs::path(L"/foo/bar.txt/bar").has_extension());
    EXPECT_FALSE(::mjfs::path(L"/foo/.").has_extension());
    EXPECT_FALSE(::mjfs::path(L"/foo/..").has_extension());
    EXPECT_FALSE(::mjfs::path(L"/foo/.hidden").has_extension());
}

#endif // _MJFS_UNIT_PATH_HPP_