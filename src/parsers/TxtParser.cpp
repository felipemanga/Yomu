// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/inject.hpp>
#include <common/Parser.hpp>
#include <common/PropertySet.hpp>
#include <common/String.hpp>

using namespace fs;

class TxtParser : public Parser {
public:
    Value parseFile(std::shared_ptr<File> file) override {
        return file->readTextFile();
    }
};

static Parser::Shared<TxtParser> txt{"txt"};
static Parser::Shared<TxtParser> json{"json"};
static Parser::Shared<TxtParser> md{"md"};
