/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_session_event_handler_h
#define negatus_session_event_handler_h

#include <prio.h>
#include <vector>
#include "BufferedSocket.h"
#include "EventHandler.h"

class SessionEventHandler: public EventHandler
{
public:
  SessionEventHandler(PRFileDesc* socket);

  virtual void close();

  virtual void getPollDescs(std::vector<PRPollDesc>& desc);
  virtual void handleEvent(PRPollDesc handle);
  virtual std::string name() { return "SessionEventHandler"; }

  std::string cwd;

private:
  std::vector<EventHandler*> mEvtHandlerStack;
  BufferedSocket mBufSocket;
};

#endif
