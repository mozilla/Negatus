/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "SessionEventHandler.h"
#include "Reactor.h"
#include <iostream>

SessionEventHandler::SessionEventHandler(PRFileDesc* socket)
  : mBufSocket(socket)
{
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
  if (!mBufSocket.closed())
  {
    PRPollDesc desc;
    desc.fd = mBufSocket.fd();
    desc.in_flags = PR_POLL_READ;
    descs.push_back(desc);
  }
}


void
SessionEventHandler::handleEvent(PRPollDesc desc)
{
  if (desc.fd != mBufSocket.fd())
    return;
  if (!(desc.out_flags & PR_POLL_READ))
    return;
  
  SearchableBuffer buf;
  PRUint32 bytesRead = mBufSocket.readUntilNewline(buf);
  std::cout << "read " << bytesRead << " bytes: " << buf.str() << std::endl;
  if (mBufSocket.closed())
    close();
}
