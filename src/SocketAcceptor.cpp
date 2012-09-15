/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "SocketAcceptor.h"
#include "CommandEventHandler.h"
#include "Logging.h"
#include "Reactor.h"


SocketAcceptor::SocketAcceptor(EventHandlerFactory* handlerFactory)
  : mSocket(NULL), mHandlerFactory(handlerFactory)
{
}


void
SocketAcceptor::close()
{
  if (mSocket)
  {
    // FIXME: log errors
    PR_Shutdown(mSocket, PR_SHUTDOWN_BOTH);
    PR_Close(mSocket);
    mSocket = NULL;
  }
  EventHandler::close();
}


PRStatus
SocketAcceptor::listen(PRNetAddr addr)
{
  // FIXME: log errors
  if (mSocket)
    return PR_FAILURE;

  mSocket = PR_OpenTCPSocket(PR_AF_INET);
  if (!mSocket)
    return PR_FAILURE;

  PRSocketOptionData sockOpt;
  sockOpt.option = PR_SockOpt_Nonblocking;
  sockOpt.value.non_blocking = PR_TRUE;
  PR_SetSocketOption(mSocket, &sockOpt);
  sockOpt.option = PR_SockOpt_Reuseaddr;
  sockOpt.value.reuse_addr = PR_TRUE;
  PR_SetSocketOption(mSocket, &sockOpt);

  PRStatus status = PR_Bind(mSocket, &addr);
  if (status == PR_FAILURE)
    return status;
  status = PR_Listen(mSocket, 128);
  if (status == PR_SUCCESS)
    registerWithReactor();
  return status;
}


void
SocketAcceptor::getPollDescs(std::vector<PRPollDesc>& descs)
{
  if (mSocket)
  {
    PRPollDesc desc;
    desc.fd = mSocket;
    desc.in_flags = PR_POLL_READ;
    descs.push_back(desc);
  }
}


void
SocketAcceptor::handleEvent(PRPollDesc desc)
{
  if (desc.fd != mSocket)
    return;

  if (!(desc.out_flags & PR_POLL_READ))
    return;

  PRNetAddr remoteAddr;
  PRFileDesc* newSocket = PR_Accept(mSocket, &remoteAddr, PR_INTERVAL_NO_WAIT);
  PRSocketOptionData sockOpt;
  sockOpt.option = PR_SockOpt_Nonblocking;
  sockOpt.value.non_blocking = PR_TRUE;
  PR_SetSocketOption(newSocket, &sockOpt);
  EventHandler* handler = mHandlerFactory->createEventHandler(newSocket);
}
