// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef USE_TERM

#include <common/Config.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/System.hpp>
#include <gui/Events.hpp>
#include <gui/Graphics.hpp>
#include <log/Log.hpp>


static const char* getKeyName(S32 code);

class TermSystem : public System {
public:
    Provides sys{this};
    PubSub<> pub{this};

    bool running = false;
    std::unordered_set<String> pressedKeys;
    bool mapJoyhatToMouseWheel = false;
    bool mapJoyaxisToMouseWheel = false;
    bool invertMouseWheelX = false;
    bool invertMouseWheelY = false;

    bool boot() override {
        inject<Config> config;
        mapJoyhatToMouseWheel = config->properties->get<bool>("map-joyhat-to-mousewheel");
        mapJoyaxisToMouseWheel = config->properties->get<bool>("map-joyaxis-to-mousewheel");
        invertMouseWheelX = config->properties->get<bool>("mouse-wheel-invert-x");
        invertMouseWheelY = config->properties->get<bool>("mouse-wheel-invert-y");
        return true;
    }

    bool run() override {
        if (!running) return false;
        return running;
    }

    void setMouseCursorVisible(bool visible) override {}
};

System::Shared<TermSystem> sys{"new"};

#endif
