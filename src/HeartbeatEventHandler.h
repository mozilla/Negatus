/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_heartbeat_event_handler_h
#define negatus_heartbeat_event_handler_h

#include <prio.h>
#include <sstream>
#include "BufferedSocket.h"
#include "EventHandler.h"


class HeartbeatEventHandler: public EventHandler
{
public:
  HeartbeatEventHandler(PRFileDesc* socket);

  virtual void close();
  virtual void handleEvent(PRPollDesc desc);
  virtual void handleTimeout();
  virtual void getPollDescs(std::vector<PRPollDesc>& descs);
  virtual std::string name() { return "HeartbeatEventHandler"; }


private:
  BufferedSocket mBufSocket;

  void sendThump();
};

#endif
