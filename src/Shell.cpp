/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Shell.h"
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "Logging.h"
#include "Subprocess.h"

std::string
id()
{
  std::string interfaces[3] = {"wlan0", "usb0", "lo"};
  FILE *iface;
  char buffer[BUFSIZE];

  for (int i = 0; i < 3; ++i)
  {
    sprintf(buffer, "/sys/class/net/%s/address", interfaces[i].c_str());
    iface = fopen(buffer, "r");
    if (!iface)
      continue;

    fgets(buffer, BUFSIZE, iface);
    
    // Error handling here. What if line is longer than BUFSIZE bytes?
    buffer[strlen(buffer) - 1] = '\0'; // remove extra newline
    fclose(iface);

    return std::string(buffer);
  }
  return std::string("00:00:00:00:00:00");
}


bool
readTextFile(std::string path, std::string& contents)
{
  const char *cpath = path.c_str();

  FILE *fp = fopen(cpath, "r");
  if (!fp)
  {
    fprintf(stderr, "Error on fopen: %s, with mode r.\n", cpath);
    return false;
  }

  char buffer[BUFSIZE];
  std::ostringstream output;

  while (fgets(buffer, BUFSIZE, fp))
    output << std::string(buffer);

  fclose(fp);
  std::string str = output.str();
  if (str.size())
    str.erase(str.size() - 1);
  contents = str;
  return true;
}

