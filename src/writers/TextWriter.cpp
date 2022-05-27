// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Writer.hpp>
#include <log/Log.hpp>

using namespace fs;

class TextWriter : public Writer {
public:
    bool writeFile(std::shared_ptr<File> file, const Value& data) override {
        auto str = data.toString();
        auto written = file->write(str.data(), str.size());
        return written == str.size();
    }
};

static Writer::Shared<TextWriter> txt{"txt", {"*.txt"}};
static Writer::Shared<TextWriter> json{"json", {"*.json"}};
