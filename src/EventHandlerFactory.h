/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_event_handler_factory_h
#define negatus_event_handler_factory_h

#include "EventHandler.h"


class EventHandlerFactory {
public:
    virtual EventHandler* createEventHandler(PRFileDesc* socket) = 0;
};

#endif
