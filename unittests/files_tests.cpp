// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "main.h"

#include <algorithm>

TEST_CASE("Files: paths") {

    SUBCASE("ensureExtension adds a missing extension") {

        CHECK(ensureExtension("foo", ".txt") == "foo.txt");
        CHECK(ensureExtension("foo.txt", ".txt") == "foo.txt");
    }

    SUBCASE("ensureExtension replaces a differing one") {

        CHECK(ensureExtension("foo.bin", ".txt") == "foo.txt");
    }

    SUBCASE("makeUniquePath leaves a free name alone") {

        TempDir dir("unique");
        auto path = dir / "free.txt";

        CHECK(makeUniquePath(path) == path);
    }

    SUBCASE("makeUniquePath avoids an existing name") {

        TempDir dir("unique2");
        auto path = dir / "taken.txt";
        writeFile(path, "x");

        auto unique = makeUniquePath(path);

        CHECK(unique != path);
        CHECK_FALSE(fs::exists(unique));
    }
}

TEST_CASE("Files: queries") {

    TempDir dir("queries");

    writeFile(dir / "a.txt", "hello");
    writeFile(dir / "b.dat", "worldworld");
    writeFile(dir / "sub" / "c.txt", "nested");

    SUBCASE("fileExists distinguishes files from nothing") {

        CHECK(fileExists(dir / "a.txt"));
        CHECK_FALSE(fileExists(dir / "missing.txt"));
    }

    SUBCASE("isDirectory") {

        CHECK(isDirectory(dir.path));
        CHECK(isDirectory(dir / "sub"));
        CHECK_FALSE(isDirectory(dir / "a.txt"));
    }

    SUBCASE("getSizeOfFile reports the byte count") {

        CHECK(getSizeOfFile(dir / "a.txt") == 5);
        CHECK(getSizeOfFile(dir / "b.dat") == 10);
    }

    /* NOTE: files() lists directories alongside regular files -- it does not
     * filter on is_regular_file(). The counts below account for the 'sub'
     * entry, which is a directory. Whether that is intended is a question for
     * the library, not the test; this pins down what it currently does.
     */
    SUBCASE("files() recurses by default") {

        // a.txt, b.dat, sub, sub/c.txt
        auto all = files(dir.path);
        CHECK(all.size() == 4);

        // a.txt, b.dat, sub
        auto flat = files(dir.path, false);
        CHECK(flat.size() == 3);
    }

    SUBCASE("files() filters by suffix, which must carry the leading dot") {

        auto txt = files(dir.path, true, std::vector<string> { ".txt" });

        CHECK(txt.size() == 2);
        for (const auto &f : txt) CHECK(f.extension() == ".txt");

        // Without the dot nothing matches: the comparison is against
        // path::extension(), which includes it.
        CHECK(files(dir.path, true, std::vector<string> { "txt" }).empty());
    }

    SUBCASE("filesRel returns paths relative to the root") {

        auto rel = filesRel(dir.path);

        REQUIRE(rel.size() == 4);
        for (const auto &f : rel) CHECK_FALSE(f.is_absolute());
    }
}

TEST_CASE("Files: creating and removing") {

    TempDir dir("create");

    SUBCASE("createDirectory") {

        auto sub = dir / "made";

        CHECK(createDirectory(sub));
        CHECK(isDirectory(sub));
    }

    SUBCASE("remove deletes a file") {

        auto path = dir / "gone.txt";
        writeFile(path, "x");
        REQUIRE(fileExists(path));

        utl::remove(path);
        CHECK_FALSE(fileExists(path));
    }

    SUBCASE("remove deletes a directory tree") {

        writeFile(dir / "tree" / "deep" / "f.txt", "x");

        utl::remove(dir / "tree");
        CHECK_FALSE(fs::exists(dir / "tree"));
    }

    SUBCASE("numDirectoryItems") {

        writeFile(dir / "one.txt", "1");
        writeFile(dir / "two.txt", "2");

        CHECK(numDirectoryItems(dir.path) == 2);
    }
}

TEST_CASE("Files: header matching") {

    TempDir dir("headers");

    auto path = dir / "magic.bin";
    writeFile(path, "PK\x03\x04rest of file");

    SUBCASE("matchingFileHeader finds a signature at the start") {

        CHECK(matchingFileHeader(path, string("PK")));
        CHECK_FALSE(matchingFileHeader(path, string("XY")));
    }

    SUBCASE("matchingFileHeader honours the offset") {

        CHECK(matchingFileHeader(path, string("rest"), 4));
        CHECK_FALSE(matchingFileHeader(path, string("rest"), 0));
    }

    SUBCASE("matchingBufferHeader works on raw memory") {

        const u8 buf[] = { 'A', 'B', 'C', 'D' };

        CHECK(matchingBufferHeader(buf, string("AB")));
        CHECK(matchingBufferHeader(buf, string("CD"), 2));
        CHECK_FALSE(matchingBufferHeader(buf, string("ZZ")));
    }

    SUBCASE("matchingBufferHeader respects the buffer length") {

        const u8 buf[] = { 'A', 'B' };

        CHECK(matchingBufferHeader(buf, 2, string("AB")));
        CHECK_FALSE(matchingBufferHeader(buf, 2, string("ABC")));
    }
}

TEST_CASE("Files: syncing directories") {

    TempDir dir("sync");
    auto src = dir / "src";
    auto dst = dir / "dst";

    writeFile(src / "a.txt", "aaa");
    writeFile(src / "sub" / "b.txt", "bbb");

    SUBCASE("syncDirectory copies everything into an empty destination") {

        auto copied = syncDirectory(src, dst);

        CHECK(copied == 2);
        CHECK(readFile(dst / "a.txt") == "aaa");
        CHECK(readFile(dst / "sub" / "b.txt") == "bbb");
    }

    SUBCASE("syncDirectory skips files that are already current") {

        syncDirectory(src, dst);
        CHECK(syncDirectory(src, dst) == 0);
    }

    SUBCASE("syncDirectory does not delete extra files") {

        syncDirectory(src, dst);
        writeFile(dst / "extra.txt", "e");

        syncDirectory(src, dst);
        CHECK(fileExists(dst / "extra.txt"));
    }

    SUBCASE("getPruneList finds files missing from the source") {

        syncDirectory(src, dst);
        writeFile(dst / "extra.txt", "e");

        auto prune = getPruneList(src, dst);

        REQUIRE(prune.size() == 1);
        CHECK(prune[0].filename() == "extra.txt");
    }

    SUBCASE("pruneDirectory removes them") {

        syncDirectory(src, dst);
        writeFile(dst / "extra.txt", "e");

        CHECK(pruneDirectory(src, dst) == 1);
        CHECK_FALSE(fileExists(dst / "extra.txt"));
        CHECK(fileExists(dst / "a.txt"));
    }

    SUBCASE("mirrorDirectory makes destination match source exactly") {

        writeFile(dst / "stale.txt", "s");

        mirrorDirectory(src, dst);

        CHECK(fileExists(dst / "a.txt"));
        CHECK(fileExists(dst / "sub" / "b.txt"));
        CHECK_FALSE(fileExists(dst / "stale.txt"));
    }

    SUBCASE("syncDirectory rejects a missing source") {

        CHECK_THROWS(syncDirectory(dir / "nope", dst));
    }
}
