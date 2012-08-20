/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "Logging.h"
#include <prnetdb.h>
#include <sstream>

#include <iostream>

std::string
addrStr(PRNetAddr addr)
{
  std::stringstream ss;
  char s[32];
  PR_NetAddrToString(&addr, s, 32);
  ss << s;
  if (addr.inet.port)
    ss << ":" << PR_ntohs(addr.inet.port);
  return ss.str();
}
