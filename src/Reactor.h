/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_reactor_h
#define negatus_reactor_h

#include <prio.h>
#include <prtime.h>
#include <vector>

class EventHandler;

class Reactor {
public:
  Reactor();

  void registerHandler(EventHandler* evtHandler);
  void removeHandler(EventHandler* evtHandler);
  void run();
  void stop();

  static Reactor* instance();

private:
  std::vector<EventHandler*> mEvtHandlers;
  static Reactor* mInstance;
  void deleteClosed();
};

#endif
