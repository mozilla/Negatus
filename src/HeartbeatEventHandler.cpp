/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "HeartbeatEventHandler.h"
#include "Shell.h"
#include "Strings.h"

#include <prinrval.h>

#include <iostream>
#include <string>
#include <sstream>

#define TIMEOUT 60


HeartbeatEventHandler::HeartbeatEventHandler(PRFileDesc* socket)
  : mBufSocket(socket)
{
  registerWithReactor();
  sendThump();
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
  {
    close();
    return;
  }
}


void
HeartbeatEventHandler::handleTimeout()
{
  sendThump();
  setTimeout(PR_SecondsToInterval(TIMEOUT));
}


void
HeartbeatEventHandler::sendThump()
{
  // devmgrSUT.py expects NULL terminated str
  std::ostringstream msg;
  msg << systime() << " Thump thump - " << id() << ENDL;
  std::string thump = msg.str();
  mBufSocket.write(thump.c_str(), thump.size() + 1);
}


