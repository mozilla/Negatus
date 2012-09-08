/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "Subprocess.h"
#include <iostream>
#include <sstream>
#include <stdio.h>

FILE*
checkPopen(std::string cmd, std::string mode)
{
  FILE *fp = popen(cmd.c_str(), mode.c_str());
  if (!fp)
  {
    std::cerr << "Error on popen: " << cmd << " , with mode " << mode
              << std::endl;
    return NULL;
  }
  return fp;
}


std::string
getCmdOutput(std::string cmd)
{
  FILE *fp = checkPopen(cmd, "r");
  if (!fp)
    return "";
  char buffer[BUFSIZE];
  std::ostringstream output;

  while (fgets(buffer, BUFSIZE, fp))
    output << std::string(buffer);
  pclose(fp);

  std::string str = output.str();
  if (str.size())
    str.erase(str.size() - 1); // remove the last extra newline
  return str;
}
