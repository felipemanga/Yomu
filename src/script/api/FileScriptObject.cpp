// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <fs/File.hpp>
#include <script/Engine.hpp>
#include <script/ScriptObject.hpp>

class FileScriptObject : public script::ScriptObject {
public:
    std::shared_ptr<fs::File> root;

    Value getWrapped() override {
        return root;
    }

    void setWrapped(const Value& vmodel) override {
        root = vmodel;
        script::ScriptObject::setWrapped(vmodel);
    }

    FileScriptObject() {
    }
};

// static script::ScriptObject::Shared<FileScriptObject> xsoType{typeid(std::shared_ptr<FileNode>).name()};
