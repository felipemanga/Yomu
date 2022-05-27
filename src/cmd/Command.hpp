// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/PropertySet.hpp>

class Command : public Injectable<Command>,
                public Model,
                public std::enable_shared_from_this<Command> {
    bool wasCommitted = false;

protected:
    void commit() {}

public:
    bool committed() {return wasCommitted;}
    virtual U32 commitSize() {return 1;}
    virtual void run() = 0;
    virtual void undo() {};
    virtual void redo() {run();}
};
