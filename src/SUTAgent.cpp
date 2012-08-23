/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iostream>

#include <plgetopt.h>
#include <prdtoa.h>
#include <prerror.h>
#include <prio.h>
#include <prnetdb.h>
#include <prtime.h>

#include "Strings.h"
#include "Reactor.h"
#include "SocketAcceptor.h"


// FIXME: This is not portable!
#include <signal.h>

bool wantToDie = false;

void signalHandler(int signal)
{
  wantToDie = true;
}


int main(int argc, char **argv)
{
  signal(SIGTERM, &signalHandler);
  signal(SIGINT, &signalHandler);
  signal(SIGHUP, &signalHandler);
  PRInt16 port = 20701;
  bool optionError = false;
  PLOptState* optState = PL_CreateOptState(argc, argv, "p:");
  while (true)
  {
    PLOptStatus status = PL_GetNextOpt(optState);
    if (status == PL_OPT_BAD)
    {
      std::cerr << "Incorrect option(s). Usage: " << argv[0] << " [-p <port>]"
                << std::endl;
      optionError = true;
      break;
    }
    else if (status == PL_OPT_OK)
    {
      if (optState->option == 'p')
      {
        port = PR_strtod(optState->value, NULL);
        if (port <= 0)
        {
          std::cerr << "Invalid port number." << std::endl;
          optionError = true;
          break;
        }
      }
    }
    else if (status == PL_OPT_EOL)
      break;
  }

  PL_DestroyOptState(optState);

  if (optionError)
    return 1;
  
  SocketAcceptor* acceptor = new SocketAcceptor();
  PRNetAddr acceptorAddr;
  PR_InitializeNetAddr(PR_IpAddrAny, port, &acceptorAddr);
  std::cout << "listening on " << addrStr(acceptorAddr) << std::endl;
  PRStatus status = acceptor->listen(acceptorAddr);
  if (status == PR_FAILURE)
  {
    std::cerr << "Failure to open socket: " << PR_GetError() << std::endl;
    return 1;
  }
  Reactor* reactor = Reactor::instance();
  while (!wantToDie)
    reactor->run();
  reactor->stop();
  return 0;
}
