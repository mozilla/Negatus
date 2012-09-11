/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_reactor_h
#define negatus_reactor_h

#include <prinrval.h>
#include <prio.h>
#include <prtime.h>
#include <vector>

class EventHandler;

class Reactor {
public:
  /** Register an EventHandler with the Reactor.
   * Registering causes the Reactor to call getPollDescs()
   * on the handler and to delete the handler if it is
   * closed.
   * It is not necessary to register a handler in order
   * to set a timeout unless automatic cleanup is desired.
   * FIXME: is this weirdly asymmetric?
   */
  void registerHandler(EventHandler* evtHandler);
  void removeHandler(EventHandler* evtHandler);
  void run();
  void stop();

  void setTimeout(PRIntervalTime interval, EventHandler* evtHandler);

  static Reactor* instance();

private:
  struct Timeout
  {
    PRIntervalTime epoch;
    PRIntervalTime interval;
    EventHandler* evtHandler;

    Timeout(PRIntervalTime _epoch, PRIntervalTime _interval,
            EventHandler* _evtHandler);
    Timeout(const Timeout& t);
    Timeout& operator=(const Timeout& rhs);

    bool expired();
  };

  Reactor();

  std::vector<EventHandler*> mEvtHandlers;
  std::vector<Timeout> mTimeouts;
  static Reactor* mInstance;
  void deleteClosed();
};

#endif
