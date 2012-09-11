/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/. */

#define DEBUG
#include "Logger.h"

Logger* Logger::mInstance = NULL;
PRLogModuleInfo* Logger::logModule = NULL;

Logger::Logger()
{
  logModule = PR_NewLogModule("NegatusLOG");
  PR_SetLogFile("Negatus.log");
  PR_LOG(logModule, PR_LOG_ALWAYS, ("NegatusLOG init.\n"));
}

Logger*
Logger::instance()
{
    if (!mInstance)
        mInstance = new Logger();
    return mInstance;
}

void
Logger::log(std::string msg) {
  PR_LOG(logModule, PR_LOG_ALWAYS, (msg.c_str()));
}
