/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

// C++
#include <iostream>
#include <vector>

// C
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// NSPR
#include <prio.h>
#include <prnetdb.h>
#include <prtime.h>

#include "Logging.h"
#include "Reactor.h"
#include "SocketAcceptor.h"

#define BUFSIZE 1024

bool wantToDie = false;


// FIXME: This is not portable!
#include <signal.h>

void signalHandler(int signal)
{
  std::cout << "signal caught!" << std::endl;
  wantToDie = true;
}


FILE *checkPopen(std::string cmd, std::string mode) {
  FILE *fp = popen(cmd.c_str(), mode.c_str());
  if (!fp) {
    fprintf(stderr, "Error on popen: %s, with mode %s.\n", cmd.c_str(), mode.c_str());
    exit(1);
  }
  return fp;
}

std::string getCmdOutput(std::string cmd) {
  FILE *fp = checkPopen(cmd, "r");
  char buffer[BUFSIZE];
  std::string output;

  while (fgets(buffer, BUFSIZE, fp)) {
    output += std::string(buffer);
  }

  pclose(fp);
  return output;
}

std::string readTextFile(std::string path) {
  const char *cpath = path.c_str();

  FILE *fp = fopen(cpath, "r");
  if (!fp) {
    fprintf(stderr, "Error on fopen: %s, with mode r.\n", cpath);
    exit(1);
  }

  char buffer[BUFSIZE];
  std::string output;

  while (fgets(buffer, BUFSIZE, fp)) {
    output += std::string(buffer);
  }

  fclose(fp);
  return output;
}

bool cd(std::string path) {
  int success = chdir(path.c_str());
  return (success == 0);
}

std::string cwd() {
  char output[BUFSIZE];
  getcwd(output, BUFSIZE);

  return std::string(output);
}

uint64_t clok() {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (uint64_t) ts.tv_sec * 1000 + ts.tv_nsec / 1000000.0;
}

bool dirw(std::string path) {
  // maybe check first that dir exists and is a dir
  int success = access(path.c_str(), W_OK);
  return (success == 0);
}

// TODO exec

std::string hash(std::string path) {
  const char *cpath = path.c_str();
  char buffer[BUFSIZE];

  if (access(cpath, F_OK | R_OK)) {
    return std::string("");
  }

  sprintf(buffer, "md5sum %s", cpath);
  return getCmdOutput(std::string(buffer));
}

std::string id() {
  std::string interfaces[3] = {"wlan0", "usb0", "lo"};
  FILE *iface;
  char buffer[BUFSIZE];

  for (int i = 0; i < 3; ++i) {
    sprintf(buffer, "/sys/class/net/%s/address", interfaces[i].c_str());
    iface = fopen(buffer, "r");
    if (!iface) {
      continue;
    }

    fgets(buffer, BUFSIZE, iface);
    fclose(iface);

    return std::string(buffer);
  }
  return std::string("00:00:00:00:00:00");
}

std::string os() {
  // not really supported yet. Best we could do is
  // cat /system/sources.xml | grep gaia and another grep for m-c
  return std::string("B2G");
}

// need to figure out how to build NSPR for ARM first
std::string systime() {
  PRTime now = PR_Now();
  PRExplodedTime ts;
  PR_ExplodeTime(now, NULL, &ts);

  return std::string("");
}

// need to figure a better way
std::string uptime() {
  return getCmdOutput("uptime");
}

// need to figure a better way
std::string screen() {
  return readTextFile("/sys/devices/virtual/graphics/fb0/modes");
}

int main(int argc, char **argv) {
/*
  std::cout << getCmdOutput("uname -s -m -r") << std::endl;

  cd("/data/local");
  std::cout << cwd() << std::endl;

  std::cout << clok() << std::endl;
  std::cout << time(NULL) << std::endl;

  std::cout << dirw("/") << std::endl;
  std::cout << dirw("/data/local") << std::endl;

  std::cout << hash("/init.rc") << std::endl;
  std::cout << hash("/weird/path") << std::endl;

  std::cout << id() << std::endl;

  std::cout << uptime() << std::endl;

  std::cout << systime() << std::endl;

  std::cout << screen() << std::endl;
*/
  signal(SIGTERM, &signalHandler);
  signal(SIGINT, &signalHandler);
  signal(SIGHUP, &signalHandler);
  SocketAcceptor* acceptor = new SocketAcceptor();
  PRNetAddr acceptorAddr;
  PR_InitializeNetAddr(PR_IpAddrAny, 20801, &acceptorAddr);
  std::cout << "listening on " << addrStr(acceptorAddr) << std::endl;
  acceptor->listen(acceptorAddr);
  Reactor* reactor = Reactor::instance();
  while (!wantToDie)
    reactor->run();
  reactor->stop();
  return 0;
}
