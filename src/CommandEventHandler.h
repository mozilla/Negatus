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

class BufferedSocket;
class SessionEventHandler;

class CommandEventHandler: public EventHandler
{
public:
  CommandEventHandler(BufferedSocket& bufSocket,
                      SessionEventHandler& session);

  virtual void getPollDescs(std::vector<PRPollDesc>& descs);
  virtual void handleEvent(PRPollDesc desc);
  virtual std::string name() { return "CommandEventHandler"; }

  void handleLine(std::string line);

private:
  struct CommandLine
  {
    CommandLine(std::string line);
    std::string cmd;
    std::vector<std::string> args;
  };

  BufferedSocket& mBufSocket;
  SessionEventHandler& mSession;
  std::string mPrompt;

  void sendPrompt();
  void do_rmdr(std::string path, std::ostringstream &out);

  FILE *checkPopen(std::string cmd, std::string mode);
  std::string getCmdOutput(std::string cmd);
  std::string readTextFile(std::string path);
  int getFirstIntPos(char *str, int limit);
  std::string joinPaths(std::string p1, std::string p2);

  // Command implementations
  std::string cat(std::vector<std::string>& args);
  std::string cd(std::vector<std::string>& args);
  std::string clok(std::vector<std::string>& args);
  std::string cwd(std::vector<std::string>& args);
  std::string dirw(std::vector<std::string>& args);
  std::string exec(std::vector<std::string>& args);
  std::string hash(std::vector<std::string>& args);
  std::string id(std::vector<std::string>& args);
  std::string isDir(std::vector<std::string>& args);
  std::string isDir(std::string path);
  std::string ls(std::vector<std::string>& args);
  std::string memory(std::vector<std::string>& args);
  std::string mkdir(std::vector<std::string>& args);
  std::string os(std::vector<std::string>& args);
  std::string power(std::vector<std::string>& args);
  std::string ps(std::vector<std::string>& args);
  std::string quit(std::vector<std::string>& args);
  std::string screen(std::vector<std::string>& args);
  std::string systime(std::vector<std::string>& args);
  std::string rm(std::vector<std::string>& args);
  std::string rm(std::string path);
  std::string rmdr(std::vector<std::string>& args);
  std::string testroot(std::vector<std::string>& args);
  std::string uptime(std::vector<std::string>& args);
};

#endif
