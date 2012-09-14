/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "HeartbeatEventHandler.h"
#include "Shell.h"
#include "Strings.h"

#include <stdio.h>
#include <prinrval.h>
#include <prtime.h>

#include <iostream>
#include <string>
#include <sstream>

#define TIMEOUT 60


HeartbeatEventHandler::HeartbeatEventHandler(PRFileDesc* socket)
  : mBufSocket(socket)
{
  registerWithReactor();
  sendTraceOutput();
  setTimeout(PR_SecondsToInterval(TIMEOUT));
}


void
HeartbeatEventHandler::close()
{
  mBufSocket.close();
  EventHandler::close();
}


void
HeartbeatEventHandler::getPollDescs(std::vector<PRPollDesc>& descs)
{
  if (!mBufSocket.closed())
  {
    if (!mBufSocket.recvClosed())
    {
      PRPollDesc desc;
      desc.fd = mBufSocket.fd();
      desc.in_flags = PR_POLL_READ;
      descs.push_back(desc);
    }
  }
}


void
HeartbeatEventHandler::handleEvent(PRPollDesc desc)
{
  // Make sure we drain all the data we can from the socket.
  while (!closed())
  {
    if (desc.fd != mBufSocket.fd())
      break;
    if (!(desc.out_flags & PR_POLL_READ))
      break;

    bool noMoreToRead = false;
    while (!closed())
    {
      std::stringstream buf;
      PRUint32 numRead = mBufSocket.readLine(buf);
      if (!numRead)
      {
        noMoreToRead = true;
        break;
      }
    }

    if (noMoreToRead)
      break;
  }

  if (mBufSocket.recvClosed())
    close();
}


void
HeartbeatEventHandler::handleTimeout()
{
  setTimeout(PR_SecondsToInterval(TIMEOUT));
  sendThump();
}

std::string
HeartbeatEventHandler::timestamp()
{
  PRTime now = PR_Now();
  PRExplodedTime ts;
  PR_ExplodeTime(now, PR_LocalTimeParameters, &ts);

  char buffer[20];
  sprintf(buffer, "%d%02d%02d-%02d:%02d:%02d", ts.tm_year, ts.tm_month,
    ts.tm_mday, ts.tm_hour, ts.tm_min, ts.tm_sec);

  return std::string(buffer);
}


void
HeartbeatEventHandler::sendThump()
{
  std::ostringstream msg;
  msg << timestamp() << " Thump thump - " << id() << ENDL;
  std::string thump = msg.str();
  // devmgrSUT.py expects NULL terminated str
  mBufSocket.write(thump.c_str(), thump.size() + 1);
}


void
HeartbeatEventHandler::sendTraceOutput()
{
  std::ostringstream msg;
  msg << timestamp() << " trace output" << ENDL;
  std::string output = msg.str();
  // devmgrSUT.py expects NULL terminated str
  mBufSocket.write(output.c_str(), output.size() + 1);
}


EventHandler*
HeartbeatEventHandlerFactory::createEventHandler(PRFileDesc* socket)
{
  return new HeartbeatEventHandler(socket);
}
