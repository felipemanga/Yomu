// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <fs/FileSystem.hpp>
#include <common/Writer.hpp>

using namespace fs;

bool Writer::writeFile(const String& path, const Value& data) {
    auto fsentity = inject<FileSystem>{}->find(path);
    if (!fsentity)
        return false;
    auto file = fsentity->get<File>();
    if (!file)
        return false;
    if (!file->open({
                .write=true,
                .create=true,
                .truncate=true
            }))
        return false;
    return writeFile(file, data);
}

