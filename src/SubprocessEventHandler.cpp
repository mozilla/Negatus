/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "SubprocessEventHandler.h"
#include "BufferedSocket.h"
#include "CommandEventHandler.h"
#include "Logging.h"
#include "Reactor.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string>
#include <sstream>

#define SUBPROCESS_BUFFER_SIZE 1024

SubprocessEventHandler::SubprocessEventHandler(
  BufferedSocket& bufSocket, CommandEventHandler& commandEventHandler,
  std::string cmdLine,
  std::vector<std::string>& envNames, std::vector<std::string>& envValues)
  : mBufSocket(bufSocket), mCommandEventHandler(commandEventHandler),
    mP(NULL), mBuffer(new char[SUBPROCESS_BUFFER_SIZE])
{
  // set the env vars and backup the old vals
  std::vector<std::string> backup;
  for (int i = 0; i < envNames.size(); ++i)
  {
    const char *name = envNames[i].c_str();
    char *old = getenv(name);
    if (!old)
      backup.push_back("");
    else
      backup.push_back(std::string(old));
    setenv(name, envValues[i].c_str(), 1);
  }

  mP = popen(cmdLine.c_str(), "r");

  // restore the env
  for (int i = 0; i < envNames.size(); ++i)
  {
    const char *name = envNames[i].c_str();
    if (backup[i].size() == 0)
      unsetenv(name);
    else
      setenv(name, backup[i].c_str(), 1);
  }

  if (mP == NULL)
  {
    close();
    return;
  }

  fcntl(fileno(mP), F_SETFL, O_NONBLOCK);

  Reactor::instance()->setTimeout(PR_MillisecondsToInterval(SUBPROCESS_POLL_PERIOD_MS), &mCommandEventHandler);
}


SubprocessEventHandler::~SubprocessEventHandler()
{
  delete[] mBuffer;
}


void
SubprocessEventHandler::close()
{
  if (mP)
  {
   int ret = pclose(mP);
   // send back the return code
   std::ostringstream retcode;
   retcode << "return code [" << ret << "]" << std::endl;
   std::string msg = retcode.str();
   mBufSocket.write(msg.c_str(), msg.length());
  }
  EventHandler::close();
}


void
SubprocessEventHandler::handleTimeout()
{
  ssize_t r = read(fileno(mP), mBuffer, SUBPROCESS_BUFFER_SIZE);
  if (r == -1 && errno != EAGAIN)
  {
    mBufSocket.write(agentWarn("error reading from pipe"));
    close();
  }
  else if (r > 0)
    mBufSocket.write(mBuffer, r);
  else if (r == 0)
    close();

  if (!closed())
    Reactor::instance()->setTimeout(PR_MillisecondsToInterval(SUBPROCESS_POLL_PERIOD_MS), &mCommandEventHandler);
}
