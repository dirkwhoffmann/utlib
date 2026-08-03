// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "main.h"

#include <fstream>

TempDir::TempDir(const std::string &name)
{
    path = fs::temp_directory_path() / ("utlib-test-" + name);

    // Start from a known-empty state even if a previous run died mid-test
    std::error_code ec;
    fs::remove_all(path, ec);
    fs::create_directories(path);
}

TempDir::~TempDir()
{
    // Destructors do not throw, so failures are swallowed by the error code
    std::error_code ec;
    fs::remove_all(path, ec);
}

void
writeFile(const fs::path &path, const std::string &contents)
{
    fs::create_directories(path.parent_path());

    std::ofstream out(path, std::ios::binary);
    out << contents;
}

std::string
readFile(const fs::path &path)
{
    std::ifstream in(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)),
                       std::istreambuf_iterator<char>());
}
