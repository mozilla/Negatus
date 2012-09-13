/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_command_event_handler_h
#define negatus_command_event_handler_h

#include <prio.h>
#include <sstream>
#include "BufferedSocket.h"
#include "EventHandler.h"


class CommandEventHandler: public EventHandler
{
public:
  CommandEventHandler(PRFileDesc* socket);

  virtual void close();
  virtual void getPollDescs(std::vector<PRPollDesc>& descs);
  virtual void handleEvent(PRPollDesc desc);
  virtual void handleTimeout();
  virtual std::string name() { return "CommandEventHandler"; }

  void handleLine(std::string line);

private:
  struct CommandLine
  {
    CommandLine(std::string line);
    std::string cmd;
    std::vector<std::string> args;
  };

  BufferedSocket mBufSocket;
  EventHandler* mDataEventHandler;
  std::string mPrompt;

  void sendPrompt();
  bool checkDataEventHandler(PRPollDesc desc);

  std::string cat(std::vector<std::string>& args);
  std::string exec(std::vector<std::string>& args);
  std::string pull(std::vector<std::string>& args);
  std::string push(std::vector<std::string>& args);
  std::string quit(std::vector<std::string>& args);
};

#endif
