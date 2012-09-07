/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "Logging.h"
#include "Strings.h"

std::string
agentWarn(std::string errStr)
{
  return "##AGENT-WARNING## " + errStr;
}


std::string
agentWarnInvalidNumArgs(PRIntn numReqArgs)
{
  std::string errStr("Invalid number of args: ");
  errStr += itoa(numReqArgs);
  errStr += " args required";
  return agentWarn(errStr);
}
