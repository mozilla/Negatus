/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "SessionEventHandler.h"
#include "CommandEventHandler.h"
#include "Reactor.h"

SessionEventHandler::SessionEventHandler(PRFileDesc* socket)
  : cwd("/"), mBufSocket(socket)
{
  EventHandler* cmdEventHandler = new CommandEventHandler(mBufSocket, *this);
  mEvtHandlerStack.push_back(cmdEventHandler);
  Reactor::instance()->registerHandler(this);
}


void
SessionEventHandler::close()
{
  mBufSocket.close();
  EventHandler::close();
}


void
SessionEventHandler::getPollDescs(std::vector<PRPollDesc>& descs)
{
  (*mEvtHandlerStack.back()).getPollDescs(descs);
}


void
SessionEventHandler::handleEvent(PRPollDesc desc)
{
  EventHandler* evtHandler = mEvtHandlerStack.back();
  evtHandler->handleEvent(desc);
  if (evtHandler->closed())
  {
    mEvtHandlerStack.pop_back();
    delete evtHandler;
  }
  if (mEvtHandlerStack.empty())
    close();
}
