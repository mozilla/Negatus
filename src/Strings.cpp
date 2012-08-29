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
  // also trim beginning whitespace
  int first_relevant = -1;
  for (int i = 0; i < s.size(); ++i)
    if (!(s[i] == '\n' || s[i] == '\r' || s[i] == ' ' || s[i] == '\t'))
    {
      first_relevant = i;
      break;
    }
  if (first_relevant != -1)
    s = s.substr(first_relevant);
  return s;
}


std::string
itoa(PRUint64 i)
{
  std::ostringstream out;
  out << i;
  return out.str();
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
