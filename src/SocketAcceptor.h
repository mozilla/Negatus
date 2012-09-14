/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_socket_acceptor_h
#define negatus_socket_acceptor_h

#include "EventHandler.h"
#include <prio.h>


class SocketAcceptor: public EventHandler {
public:
  SocketAcceptor(EventHandlerFactory* handlerFactory);

  virtual void close();

  PRStatus listen(PRNetAddr addr);
  virtual void getPollDescs(std::vector<PRPollDesc>& desc);
  virtual void handleEvent(PRPollDesc handle);
  virtual std::string name() { return "SocketAcceptor"; }

private:
  PRFileDesc* mSocket;
  EventHandlerFactory* mHandlerFactory;
};

#endif
