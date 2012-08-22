/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_event_handler_h
#define negatus_event_handler_h

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
  virtual void getPollDescs(std::vector<PRPollDesc>& descs) = 0;
  PRTime getTimeout() { return mTimeout; }
  virtual void handleEvent(PRPollDesc handle) {}
  virtual void handleTimeout() {}
  virtual std::string name() { return "EventHandler"; }

protected:
  void setTimeout(PRTime timeout) { mTimeout = timeout; }
  void clearTimeout() { mTimeout = 0; }

  PRTime mTimeout;

private:
  bool mClosed;
};

#endif
