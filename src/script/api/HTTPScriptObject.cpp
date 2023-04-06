// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#if !defined(NO_NETWORK)

#include <regex>

#include <httprequest/httplib.hpp>
#include <script/Engine.hpp>
#include <script/ScriptObject.hpp>
#include <task/TaskManager.hpp>

class HTTPScriptObject : public script::ScriptObject {
public:
    TaskHandle handle;

    enum class State {
        Ready,
        Busy,
        Error
    } state = State::Ready;

    enum class Type {
        Get,
        Post
    };

    U32 code = 0;
    Vector<U8> bytes;
    String error;
    String url;
    std::shared_ptr<script::Engine> engine;

    HTTPScriptObject() {
      addMethod("get", this, &HTTPScriptObject::get);
      addMethod("post", this, &HTTPScriptObject::post);
      addProperty("state", [=]{return (int) state;});
      addProperty("code", [=]{return code;});
      addProperty("text", [=]{return String{bytes.begin(), bytes.end()};});
      addProperty("url", [=]{return url;});
      addProperty("error", [=]{return error;});
      addProperty("bytes", [=]{return script::Value(bytes.data(), bytes.size(), false);});
      makeGlobal("http");
    }

    script::Value get(const String& url) {
        return request(Type::Get, url);
    }

    script::Value post(const String& url, const String& body, const String& contentType) {
        return request(Type::Post, url, body, contentType);
    }

    script::Value request(Type type, const String& url, const String& body = "", const String& contentType = ""){
        engine = getEngine().shared_from_this();
        code = 0;
        bytes.clear();

        static std::regex expr("^(https?://[^/]+)(.*$)");
        std::cmatch match;
        std::regex_match(url.c_str(), match, expr);
        if (match.empty())
            return -1;
        httplib::Client req{match[1].str().c_str()};
        auto resp = type == Type::Get ? req.Get(match[2].str().c_str()) :
            req.Post(match[2].str().c_str(),
                     body.c_str(),
                     body.size(),
                     contentType.c_str()
                );
        if (!resp) {
            error = httplib::to_string(resp.error());
            logE((int)type, " ", url, " ", contentType);
            return -1;
        }
        bytes.insert(bytes.end(), resp->body.begin(), resp->body.end());
        return int(resp->status);
    }
};

static script::ScriptObject::Shared<HTTPScriptObject> reg("HTTPScriptObject", {"global"});

#endif
