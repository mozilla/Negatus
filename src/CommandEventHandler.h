/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this file,
 You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef __COMMAND_EVENT_HANDLER__
#define __COMMAND_EVENT_HANDLER__

#include <string>
#include <sstream>

#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <prio.h>
#include <prtime.h>

#include "misc.h"
#include "SessionEventHandler.h"

class SessionEventHandler;


class CommandEventHandler {
private:
   SessionEventHandler *session; // need to talk about deallocating this!
public:

  CommandEventHandler();
  CommandEventHandler(SessionEventHandler *session);

  bool cd(std::string path);
  std::string cwd();
  uint64_t clok();
  bool dirw(std::string path);
  std::string hash(std::string path);
  std::string id();
  std::string os();
  std::string systime();
  std::string uptime();
  std::string screen();
  std::string memory();
  std::string power();
  std::string ps();
  int isDir(std::string path);
  std::string ls(std::string path);
  std::string mkdir(std::string path);
  bool rm(std::string path);
  bool rmdr(std::string path);
  std::string testroot();
};

#endif
