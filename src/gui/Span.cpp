// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Config.hpp>
#include <common/Font.hpp>
#include <common/match.hpp>
#include <common/Messages.hpp>
#include <common/Parser.hpp>
#include <common/PubSub.hpp>
#include <common/Surface.hpp>
#include <common/System.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Graphics.hpp>
#include <gui/Node.hpp>
#include <texttransform/TextTransform.hpp>

namespace ui {
    class Span : public Node {
        Property<String> fontPath{this, "font", "", &Span::reload};
        Property<Rect> fontPadding{this, "font-padding"};
        Property<String> text{this, "text", "", &Span::redraw};
        Property<String> textTransform{this, "text-transform", "translate", &Span::changeTransforms};
        Vector<std::shared_ptr<TextTransform>> transforms;
        bool transformsLoaded = false;
        Property<S32> align{this, "align", -1};
        Property<U32> size{this, "size", 12, &Span::redraw};
        Property<Color> color{this, "color", {0xFF, 0xFF, 0xFF}, &Span::redraw};
        Property<Vector<S32>> advance{this, "text-advance"};
        Property<std::shared_ptr<Font>> font{this, "font-ptr", nullptr, &Span::redraw};
        Property<std::shared_ptr<Surface>> surface{this, "surface"};
        PubSub<msg::Flush> pub{this};

        void changeTransforms() {
            transformsLoaded = true;
            transforms.clear();
            for (auto& raw : split(textTransform, ",")) {
                String clean = tolower(trim(raw));
                if (clean.empty())
                    continue;
                inject<TextTransform> transform{clean};
                if (!transform)
                    continue;
                transforms.push_back(transform);
            }
        }

        void redraw() {
            if (!transformsLoaded)
                changeTransforms();
            std::shared_ptr<Surface> surface;
            Vector<S32> advance;
            if (auto font = *this->font) {
                String str = text;
                for (auto& transform : transforms)
                    str = transform->run(str, this);
                surface = font->print(size, *color, str, fontPadding, advance);
            }
            set("surface", surface);
            set("text-advance", advance);
            resize();
        }

        void reload() {
            auto& surface = *this->surface;
            surface.reset();
            if (!fontPath->empty())
                *font = FileSystem::parse(fontPath);
            redraw();
        }

    public:
        void on(msg::Flush& flush) {
            flush.hold(*font);
        }

        void onResize() override {
            if (*surface) {
                localRect.x = 0;
                localRect.y = 0;
                localRect.width = (*surface)->width();
                localRect.height = (*surface)->height();
                if (localRect.width) {
                    F32 factor = globalRect.width / F32(localRect.width);
                    if (factor >= 1) {
                        if (align == -1) { // left, default
                        } else if (align == 0) { // center
                            globalRect.x += globalRect.width/2 - localRect.width/2;
                        } else if (align == 1) { // right
                            globalRect.x += globalRect.width - localRect.width;
                        }
                        globalRect.width = localRect.width;
                        globalRect.height = localRect.height;
                    } else {
                        globalRect.y += globalRect.height/2 - localRect.height/2;
                        globalRect.height = localRect.height * factor;
                    }
                }
            }
        }

        void draw(S32 z, Graphics& g) override {
            if (*surface) {
                g.blit({
                        .surface = *surface,
                        .source = localRect,
                        .destination = globalRect,
                        .nineSlice = {},
                        .zIndex    = z,
                        .multiply  = multiply
                    });
            }
            Node::draw(z, g);
        }
    };
}

static ui::Node::Shared<ui::Span> span{"span"};
