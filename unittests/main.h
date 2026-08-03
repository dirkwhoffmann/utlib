// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "doctest.h"

#include <utl/common.h>
#include <utl/abilities.h>
#include <utl/chrono.h>
#include <utl/io.h>
#include <utl/storage.h>
#include <utl/support.h>
#include <utl/wrappers.h>
#include <utl/types/UUID.h>

/* Included directly because the storage.h umbrella does not list them. That
 * looks like an oversight rather than a decision -- every other header in
 * storage/ is there -- but it is left alone here so that this copy of the
 * library stays byte-identical to the one in Silicium.
 */
#include <utl/storage/ZipArchive.h>
#include <utl/storage/ZipError.h>

#include <string>

using namespace utl;

/* A scratch directory that cleans up after itself.
 *
 * Several modules (Files, ZipArchive, Buffer) can only be tested against a real
 * file system. Each test gets its own directory under the system temp folder so
 * that a failing test cannot leave state behind for the next one to trip over.
 */
struct TempDir {

    fs::path path;

    explicit TempDir(const std::string &name);
    ~TempDir();

    TempDir(const TempDir &) = delete;
    TempDir &operator=(const TempDir &) = delete;

    fs::path operator/(const std::string &rel) const { return path / rel; }
    operator const fs::path &() const { return path; }
};

// Writes a file, creating parent directories as needed
void writeFile(const fs::path &path, const std::string &contents);

// Reads a whole file into a string
std::string readFile(const fs::path &path);
