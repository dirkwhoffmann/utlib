// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "utl/storage/ZipArchive.h"
#include "utl/io/IOError.h"
#include "utl/storage/ZipError.h"
#include <fstream>
#include <sstream>
#include <zip.h>

namespace utl {

void
ZipDeleter::operator()(zip_t *z) const noexcept
{
    if (z) zip_close(z);
}

struct ZipEntry {

    zip_t *zip = nullptr;

    ZipEntry(zip_t *zip, const string &fileName) : zip(zip)
    {
        if (auto ec = zip_entry_open(zip, fileName.c_str()); ec)
            throw ZipError(ZipError::KUBA_ZIP_ERROR, ec);
    }

    ~ZipEntry()
    {
        if (zip) zip_entry_close(zip);
    }

    ZipEntry(const ZipEntry&) = delete;
    ZipEntry& operator=(const ZipEntry&) = delete;
    ZipEntry(ZipEntry&&) = delete;
    ZipEntry& operator=(ZipEntry&&) = delete;

};

ZipArchive::ZipArchive(const fs::path &path, char access) : path(path)
{
    if (access == 'r' && !fs::exists(path)) {
        throw IOError(IOError::FILE_NOT_FOUND, path);
    }

    zip_t *rawZip = nullptr;
    switch (access) {

        case 'r': rawZip = zip_open(path.string().c_str(), 0, 'r'); break;
        case 'w': rawZip = zip_open(path.string().c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'w'); break;
        case 'a': rawZip = zip_open(path.string().c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'a'); break;
        default:  throw ZipError(ZipError::INVALID_ZIP_MODE, "'" + string(1, access) + "'");
    }

    if (rawZip == nullptr) {
        throw ZipError(ZipError::INVALID_ARCHIVE, path);
    }

    zip.reset(rawZip);
}

void
ZipArchive::swap(ZipArchive &other) noexcept
{
    std::swap(path, other.path);
    std::swap(zip, other.zip);
}

isize
ZipArchive::size() const
{
    return zip ? isize(zip_entries_total(zip.get())) : 0;
}

vector<string>
ZipArchive::listFiles() const
{
    assert(zip);
    vector<string> result;

    for (int i = 0, n = (int)size(); i < n; ++i) {

        auto ec = zip_entry_openbyindex(zip.get(), i);
        if (ec != 0) throw ZipError(ZipError::KUBA_ZIP_ERROR, ec);

        const char *name = zip_entry_name(zip.get());
        if (name) result.emplace_back(name);
        zip_entry_close(zip.get());
    }

    return result;
}

std::vector<u8>
ZipArchive::uncompress(const string &fileName)
{
    assert(zip);
    ZipEntry open(zip.get(), fileName);

    void *buf = nullptr;
    size_t bufsize = 0;

    if (auto ec = zip_entry_read(zip.get(), &buf, &bufsize); ec < 0) {
        free(buf);
        throw ZipError(ZipError::KUBA_ZIP_ERROR, ec);
    }

    std::unique_ptr<void, void(*)(void*)> guard(buf, ::free);
    return std::vector<u8>(static_cast<u8*>(buf), static_cast<u8*>(buf) + bufsize);
}

static optional<fs::path>
safeResolveTarget(const fs::path &baseDir, const fs::path &target)
{
    auto resolved = fs::weakly_canonical(baseDir / target);
    auto rel      = fs::relative(resolved, baseDir);

    if (rel.empty() || rel.string().starts_with("..") || rel.is_absolute()) {
        return std::nullopt;
    }
    return resolved;
}

void
ZipArchive::uncompress(const string &fileName, const fs::path &targetDir, u64 maxFileSize)
{
    if (!fs::is_directory(targetDir)) {
        throw IOError(IOError::DIR_NOT_FOUND, targetDir);
    }

    auto baseDir    = fs::weakly_canonical(targetDir);
    auto targetFile = safeResolveTarget(baseDir, fs::path(fileName));
    if (!targetFile) {
        throw ZipError(ZipError::INVALID_ARCHIVE, "Path traversal in zip entry: " + fileName);
    }

    if (fs::is_symlink(*targetFile)) {
        throw ZipError(ZipError::INVALID_ARCHIVE, "Target path is a symlink: " + targetFile->string());
    }

    ZipEntry open(zip.get(), fileName);

    if (fileName.ends_with('/') || zip_entry_isdir(zip.get())) {
        fs::create_directories(*targetFile);
        return;
    }

    if (maxFileSize > 0 && zip_entry_size(zip.get()) > maxFileSize) {
        throw ZipError(ZipError::INVALID_ARCHIVE, "File size exceeds limit: " + fileName);
    }

    fs::create_directories(targetFile->parent_path());

    if (auto ec = zip_entry_fread(zip.get(), targetFile->string().c_str()); ec < 0) {
        throw ZipError(ZipError::KUBA_ZIP_ERROR, "Failed to extract " + fileName + " to " + targetFile->string() + " (code " + std::to_string(ec) + ")");
    }
}

void
ZipArchive::uncompressAll(const fs::path &targetDir, u64 maxFileSize)
{
    for (auto &fileName : listFiles()) {
        uncompress(fileName, targetDir, maxFileSize);
    }
}

void
ZipArchive::write(const fs::path &file, const fs::path &root)
{
    const std::vector<fs::path> files = { file };
    write(files, root);
}

void
ZipArchive::write(const std::vector<fs::path> &files, const fs::path &root)
{
    for (auto &item : files) {

        auto rel = fs::relative(item, root);

        if (rel.empty() || rel.string().starts_with("..") || rel.is_absolute())
            throw IOError(IOError::FILE_NOT_FOUND, item);

        ZipEntry open(zip.get(), rel.generic_string().c_str());

        if (auto ec = zip_entry_fwrite(zip.get(), item.string().c_str()); ec < 0)
            throw ZipError(ZipError::KUBA_ZIP_ERROR, ec);
    }
}

void
ZipArchive::replace(const std::vector<fs::path> &files, const fs::path &root)
{
    fs::path tempPath = path.string() + ".tmp";
    ZipArchive temporary(tempPath, 'w');

    temporary.write(files, root);

    temporary.close();
    close();

    fs::rename(tempPath, path);

    zip_t *rawZip = zip_open(path.string().c_str(), 0, 'r');
    if (rawZip == nullptr)
        throw ZipError(ZipError::INVALID_ARCHIVE, path);

    zip.reset(rawZip);
}

}