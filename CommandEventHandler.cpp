/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "CommandEventHandler.h"
#include "BufferedSocket.h"
#include <iostream>

CommandEventHandler::CommandEventHandler(BufferedSocket& mBufSocket,
                                         SessionEventHandler& session)
  : mBufSocket(mBufSocket), mSession(session), mPrompt("$>")
{
  sendPrompt();
}


void
CommandEventHandler::getPollDescs(std::vector<PRPollDesc>& descs)
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
CommandEventHandler::handleEvent(PRPollDesc desc)
{
  if (desc.fd != mBufSocket.fd())
    return;
  if (!(desc.out_flags & PR_POLL_READ))
    return;
  
  SearchableBuffer buf;
  while (mBufSocket.readUntilNewline(buf))
  {
    std::string line(buf.str());
    for (std::string::reverse_iterator i = line.rbegin(); i != line.rend(); )
    {
      char c = *i++;
      if (c == '\n' || c == '\r' || c == ' ' || c == '\t')
        line.erase(line.size()-1);
      else
        break;
    }
    handleLine(line);
    sendPrompt();
  }
  if (mBufSocket.closed())
    close();
}



void
CommandEventHandler::handleLine(std::string line)
{
  std::cout << "got line: " << line << std::endl;
}


void
CommandEventHandler::sendPrompt()
{
  mBufSocket.write(mPrompt.c_str(), mPrompt.size());
}
