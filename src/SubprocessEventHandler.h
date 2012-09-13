/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_subprocess_event_handler_h
#define negatus_subprocess_event_handler_h

#include "EventHandler.h"

#define SUBPROCESS_POLL_PERIOD_MS 100

class BufferedSocket;
class CommandEventHandler;

class SubprocessEventHandler: public EventHandler
{
public:
  SubprocessEventHandler(BufferedSocket& bufSocket,
                         CommandEventHandler& commandEventHandler,
                         std::string cmdLine,
                         std::vector<std::string>& envNames,
                         std::vector<std::string>& envValues);
  virtual ~SubprocessEventHandler();

  virtual void close();
  virtual void handleTimeout();
  virtual std::string name() { return "SubprocessEventHandler"; }

private:
  BufferedSocket& mBufSocket;
  CommandEventHandler& mCommandEventHandler;
  FILE* mP;
  char* mBuffer;
};

#endif
