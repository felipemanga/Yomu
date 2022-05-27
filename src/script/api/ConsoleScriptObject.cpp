// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <sstream>

#include <script/Engine.hpp>
#include <script/ScriptObject.hpp>

#include <termios.h>

class ConsoleScriptObject : public script::ScriptObject {
public:
    inject<Log> logger;

    ConsoleScriptObject() {
        addMethod("input", this, &ConsoleScriptObject::input);
        addMethod("log", this, &ConsoleScriptObject::log);
        addMethod("assert", this, &ConsoleScriptObject::_assert);
        addMethod("logRaw", this, &ConsoleScriptObject::logRaw);
        addMethod("getChar", this, &ConsoleScriptObject::getChar);
        makeGlobal("console");
    }

    void _assert(bool condition, const std::string& msg){
        if (!condition)
            log();
    }

    void log() {
        std::stringstream stream;
        bool first = true;
        for (auto& arg : script::Function::varArgs()) {
            if (!first) {
                stream << " ";
            }
            first = false;
            stream << arg.str();
        }
        logger->write(Log::Level::Info, stream.str());
    }

    void logRaw() {
        std::stringstream stream;
        for (auto& arg : script::Function::varArgs()) {
            stream << arg.str();
        }
        std::cout << stream.str();
    }

    std::string input() {
        std::stringstream stream;
        bool first = true;
        for (auto& arg : script::Function::varArgs()) {
            if (!first)
                stream << " ";
            stream << arg.str();
            first = false;
        }
        std::cout << stream.str();
        std::string ret;
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
            ret.push_back(c);
        }
        return ret;
    }

    int getChar(bool raw) {
        if (!raw)
            return getchar();

        int c = 0;

        termios oldOpts, opts;
        int res = 0;

        auto STDIN = fileno(stdin);

        res=tcgetattr(fileno(stdin), &oldOpts);
        if (res!=0) {
            return EOF;
        }

        opts = oldOpts;
        opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
        tcsetattr(STDIN, TCSANOW, &opts);
        c = getchar();

        res = tcsetattr(STDIN, TCSANOW, &oldOpts);
        return c;
    }
};

static script::ScriptObject::Shared<ConsoleScriptObject> reg("ConsoleScriptObject", {"global"});
