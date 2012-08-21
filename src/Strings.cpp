/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "Strings.h"
#include <prnetdb.h>
#include <sstream>

std::string
trim(std::string s)
{
  for (std::string::reverse_iterator i = s.rbegin(); i != s.rend(); )
  {
    char c = *i++;
    if (c == '\n' || c == '\r' || c == ' ' || c == '\t')
      s.erase(s.size()-1);
    else
      break;
  }
  return s;
}


std::string
itoa(PRUint64 i)
{
  PRUintn len;
  PRUint64 tmp = i;
  for (len = 0; tmp > 0; len++)
    tmp /= 10;
  char s[len+1];
  s[len-1] = 0;
  tmp = i;
  for (PRUintn j = 0; j < len; j++)
  {
    s[len-1-j] = 0x30 + (tmp % 10);
    tmp /= 10;
  }
  return std::string(s);
}


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
