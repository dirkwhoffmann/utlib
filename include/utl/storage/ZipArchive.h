// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/storage/Buffer.h"
#include <regex>
#include <memory>

struct zip_t;

namespace utl {

struct ZipDeleter {
    void operator()(zip_t *z) const noexcept;
};

class ZipArchive {

    fs::path path;
    std::unique_ptr<zip_t, ZipDeleter> zip;

public:

    ZipArchive(const fs::path &path, char access = 'r');
    ZipArchive(ZipArchive &&other) noexcept = default;
    ZipArchive &operator=(ZipArchive &&other) noexcept = default;
    ZipArchive(const ZipArchive &) = delete;
    ZipArchive &operator=(const ZipArchive &) = delete;
    ~ZipArchive() noexcept = default;

    void close() noexcept { zip.reset(); }
    void swap(ZipArchive &other) noexcept;

    isize size() const;
    vector<string> listFiles() const;

    std::vector<u8> uncompress(const string &fileName);

    static constexpr u64 DEFAULT_MAX_UNCOMPRESSED_SIZE = 1024 * 1024 * 1024ULL;
    void uncompress(const string &fileName, const fs::path &targetDir, u64 maxFileSize = DEFAULT_MAX_UNCOMPRESSED_SIZE);
    void uncompressAll(const fs::path &targetDir, u64 maxFileSize = DEFAULT_MAX_UNCOMPRESSED_SIZE);

    void write(const fs::path &file, const fs::path &root);
    void write(const std::vector<fs::path> &files, const fs::path &root);

    void replace(const std::vector<fs::path> &files, const fs::path &root);
};

}