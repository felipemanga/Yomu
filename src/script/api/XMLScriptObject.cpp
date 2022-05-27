// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/XML.hpp>
#include <memory>
#include <script/Engine.hpp>
#include <script/ScriptObject.hpp>

class XMLScriptObject : public script::ScriptObject {
public:
    std::shared_ptr<XMLNode> root;

    Value getWrapped() override {
        return root;
    }

    void setWrapped(const Value& vmodel) override {
        root = vmodel;
        script::ScriptObject::setWrapped(vmodel);
    }

    XMLScriptObject() {
        addFunction("attribute", [=](const String& name) -> std::string {
            if (!root->isElement())
                return "";
            auto& attr = std::dynamic_pointer_cast<XMLElement>(root)->attributes;
            auto it = attr.find(name);
            return it == attr.end() ? "" : it->second;
        });

        addProperty("tag", [=]{return root->isElement() ? std::dynamic_pointer_cast<XMLElement>(root)->tag : "";});

        addProperty("text", [=]{return root->text;});

        addProperty("childCount", [=]() {
            return !root->isElement() ? 0 : std::dynamic_pointer_cast<XMLElement>(root)->children.size();
        });

        addFunction("getChild", [=](int offset) -> script::Value {
            if (!root->isElement())
                return nullptr;
            auto& children = std::dynamic_pointer_cast<XMLElement>(root)->children;
            if (offset >= children.size())
                return nullptr;
            return getEngine().toValue(children[offset]);
        });
    }
};

static script::ScriptObject::Shared<XMLScriptObject> xsoType{typeid(std::shared_ptr<XMLNode>).name()};
