/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_command_event_handler_h
#define negatus_command_event_handler_h

#include <prio.h>
#include <sstream>
#include "EventHandler.h"

#define BUFSIZE 1024
#define ENDL "\r\n"

class BufferedSocket;
class SessionEventHandler;

class CommandEventHandler: public EventHandler
{
public:
  CommandEventHandler(BufferedSocket& bufSocket,
                      SessionEventHandler& session);

  virtual void getPollDescs(std::vector<PRPollDesc>& desc);
  virtual void handleEvent(PRPollDesc handle);
  virtual std::string name() { return "CommandEventHandler"; }

  void handleLine(std::string line);

private:
  struct CommandLine
  {
    CommandLine(std::string line);
    std::string cmd;
    std::string arg;
  };

  BufferedSocket& mBufSocket;
  SessionEventHandler& mSession;
  std::string mPrompt;

  void sendPrompt();
  std::string agentWarn(std::string errStr);
  void do_rmdr(std::string path, std::ostringstream &out);

  FILE *checkPopen(std::string cmd, std::string mode);
  std::string getCmdOutput(std::string cmd);
  std::string readTextFile(std::string path);
  int getFirstIntPos(char *str, int limit);
  std::string joinPaths(std::string p1, std::string p2);

  // Command implementations
  std::string cd(std::string path);
  std::string clok();
  std::string cwd();
  std::string dirw(std::string path);
  std::string exec(std::string cmd);
  std::string hash(std::string path);
  std::string id();
  std::string isDir(std::string path);
  std::string ls(std::string path);
  std::string memory();
  std::string mkdir(std::string path);
  std::string os();
  std::string power();
  std::string ps();
  std::string quit();
  std::string screen();
  std::string systime();
  std::string rm(std::string path);
  std::string rmdr(std::string path);
  std::string testroot();
  std::string uptime();
};

#endif
