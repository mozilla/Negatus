/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdio.h>
#include <string.h>

#include <sstream>

#include "Registration.h"
#include "Strings.h"


bool is_alpha(char c)
{
  if (('0' <= c && c <= '9') ||
      ('a' <= c && c <= 'z') ||
      ('A' <= c && c <= 'Z'))
    return true;
  return false;
}


bool is_allowed_char(char c)
{
  if (c == '.' || c == '-' || c == '_' || c == '~')
    return true;
  return false;
}


std::string urlencode(std::string plain)
{
  std::ostringstream buf;

  for (int i = 0; i < plain.size(); ++i)
  {
    char c = plain[i];
    if (is_alpha(c) || is_allowed_char(c))
      buf << c;
    else
    {
      char hex[3];
      sprintf(hex, "%02X", c);
      buf << "%" << hex;
    }
  }
  return buf.str();
}


std::string gen_query_url(dict data)
{
  std::ostringstream buf;
  std::map<std::string, std::string>::iterator it;

  for (it = data.begin(); it != data.end(); ++it)
    buf << urlencode(it->first) << "=" << urlencode(it->second) << "&";

  std::string ret = buf.str();
  if (ret.size())
    ret.erase(ret.size() - 1);
  return ret;
}


bool read_ini(std::string path, std::map<std::string, dict> & data)
{
  FILE *f = fopen(path.c_str(), "r");
  if (!f)
    return false;

  char buf[1024];
  std::string c_section;
  dict c_map;

  while (fgets(buf, 1024, f))
  {
    std::string line = trim(std::string(buf));

    if (line.size() == 0) // blank lines
      continue;
    if (line[0] == '#') // comments
      continue;

    if (line[0] == '[') // section titles
    {
      int idx = line.find_last_of(']');
      if (idx == std::string::npos)
      {
        fclose(f);
        return false;
      }
      c_section = trim(line.substr(1, idx - 1));
      // create new dict for this section
      data[c_section] = dict();
      continue;
    }

    // line is something like "name = value"
    int idx = line.find('=');
    if (idx == std::string::npos)
      continue; // bogus line, skip it

    std::string name = trim(line.substr(0, idx));
    std::string value = trim(line.substr(idx + 1));
    data[c_section][name] = value;
  }
  fclose(f);
  return true;
}

