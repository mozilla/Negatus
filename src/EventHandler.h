/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_event_handler_h
#define negatus_event_handler_h

#include <prinrval.h>
#include <prio.h>
#include <prtime.h>
#include <string>
#include <vector>

class EventHandler {
public:
  EventHandler() : mClosed(false) {}
  virtual ~EventHandler() {}

  virtual void close() { mClosed = true; }
  bool closed() { return mClosed; }
  virtual void getPollDescs(std::vector<PRPollDesc>& descs) {};
  virtual void handleEvent(PRPollDesc handle) {}
  virtual void handleTimeout() {}
  virtual std::string name() { return "EventHandler"; }

protected:
  void registerWithReactor();
  void setTimeout(PRIntervalTime interval);

private:
  bool mClosed;
};


class EventHandlerFactory {
public:
  virtual EventHandler* createEventHandler(PRFileDesc* socket) = 0;
  virtual ~EventHandlerFactory() {}
};


#endif
