/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "HeartbeatEventHandler.h"

#include <string>


HeartbeatEventHandler::HeartbeatEventHandler(PRFileDesc* socket)
  : mBufSocket(socket)
{
  registerWithReactor();
  // fixme: send thump
}


void
HeartbeatEventHandler::close()
{
  mBufSocket.close();
  EventHandler::close();
}


void
HeartbeatEventHandler::handleEvent(PRPollDesc desc)
{
  if (mBufSocket.recvClosed())
    close();
}


void
HeartbeatEventHandler::handleTimeout()
{
  sendThump();
}

void
HeartbeatEventHandler::sendThump()
{
  // devmgrSUT.py expects NULL terminated str
  std::string thump("Thump thump!");
  mBufSocket.write(thump.c_str(), thump.size() + 1);
}


