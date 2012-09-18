/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include <plgetopt.h>
#include <prdtoa.h>
#include <prerror.h>
#include <prinrval.h>
#include <prio.h>
#include <prnetdb.h>
#include <prtime.h>
#include <prtypes.h>

#include "CommandEventHandler.h"
#include "HeartbeatEventHandler.h"
#include "Logger.h"
#include "Strings.h"
#include "Reactor.h"
#include "SocketAcceptor.h"
#include "Registration.h"


// FIXME: This is not portable!
#include <signal.h>

bool wantToDie = false;

void signalHandler(int signal)
{
  wantToDie = true;
}

#define CHECK_CALL(func) if (func != PR_SUCCESS) return PR_GetError();

PRErrorCode send_query(std::string query, std::string ip, PRUint32 port)
{
  PRFileDesc* sock = PR_NewTCPSocket();
  if (!sock)
    return PR_GetError();
  PRNetAddr addr;
  // set the ip
  CHECK_CALL(PR_StringToNetAddr(ip.c_str(), &addr));
  // set the port
  CHECK_CALL(PR_InitializeNetAddr(PR_IpAddrNull, port, &addr));
  // connect
  CHECK_CALL(PR_Connect(sock, &addr, PR_INTERVAL_MIN));
  // send
  int sent = PR_Send(sock, query.c_str(), query.size(), 0, PR_INTERVAL_MIN);
  if (sent != query.size())
    return PR_IO_ERROR;
  // shutdown the connection
  CHECK_CALL(PR_Shutdown(sock, PR_SHUTDOWN_BOTH));
  // close the socket
  CHECK_CALL(PR_Close(sock));

  std::cout << "Sent query." << std::endl;
  return 0;
}


void handle_reboot(std::string query)
{
  FILE *rebt = fopen(REBOOT_FILE, "r");
  char r_ipaddr[40];
  char line[100];
  PRUint32 r_port;
  if (!rebt)
    std::cout << "No reboot callback data." << std::endl;
  else
  {
    fgets(line, 100, rebt);
    sscanf(line, "%s %d", r_ipaddr, &r_port);
    send_query(query, r_ipaddr, r_port);
    PR_Delete(REBOOT_FILE);
  }
}


dict get_reg_data()
{
  dict data;
  data["NAME"] = "SUTAgent";
  return data;
}


bool setUpAcceptor(EventHandlerFactory* fact, std::string kind, PRInt16 port,
    PRNetAddr& acceptorAddr)
{
  SocketAcceptor* acceptor = new SocketAcceptor(fact);
  PR_InitializeNetAddr(PR_IpAddrAny, port, &acceptorAddr);
  std::cout << kind << " handler ";
  std::cout << "listening on " << addrStr(acceptorAddr) << std::endl;
  PRStatus status = acceptor->listen(acceptorAddr);
  if (status == PR_FAILURE)
  {
    std::cerr << "Failure to open socket: " << PR_GetError() << std::endl;
    return false;
  }
  return true;
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

  PRNetAddr cmdAddr, heartbeatAddr;
  if(!setUpAcceptor(new CommandEventHandlerFactory(), "Command", port,
        cmdAddr))
    return 1;
  if(!setUpAcceptor(new HeartbeatEventHandlerFactory(), "Heartbeat", 20700,
      heartbeatAddr))
    return 1;

  dict reg_data = get_reg_data();
  reg_data["IPADDR"] = addrStr(cmdAddr);
  std::string query = gen_query_url(reg_data);
  std::cout << "Query url: " << query << std::endl;

  // --- check for registration data and send it
  std::map<std::string, dict> data;
  if (!read_ini("/data/local/SUTAgent.ini", data))
    std::cout << "No SUTAgent.ini data." << std::endl;
  else
  {
    std::string ip =  data["Registration Server"]["IPAddr"];
    PRUint32 p;
    sscanf(data["Registration Server"]["PORT"].c_str(), "%d", &p);
    send_query(query, ip, p);
  }

  handle_reboot(query);

  Reactor* reactor = Reactor::instance();
  Logger *logger = Logger::instance();

  while (!wantToDie)
    reactor->run();
  reactor->stop();
  logger->log("Negatus shutting down.");
  return 0;
}
