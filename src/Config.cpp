/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Config.h"
#include <iostream>
#include <prerror.h>
#include <prio.h>
#include <string.h>
#include "Shell.h"
#ifdef _WIN32
#include <windows.h>
#endif

#include <algorithm>
#include <vector>

Config* Config::mInstance = NULL;

Config::Config()
{
}


Config*
Config::instance()
{
  if (!mInstance)
    mInstance = new Config();
  return mInstance;
}


void
Config::setTestRoot(std::string testRoot)
{
  if (testRoot.empty())
    setDefaultTestRoot();
  else
    mTestRoot = testRoot;

  PRFileInfo info;
  PRStatus st = PR_GetFileInfo(mTestRoot.c_str(), &info);
  if (st == PR_SUCCESS)
  {
    if (info.type != PR_FILE_DIRECTORY)
      std::cerr << "Test root " << mTestRoot
                << " exists but is not a directory." << std::endl;
    return;
  }

  if (PR_MkDir(mTestRoot.c_str(), 0777) == PR_FAILURE)
  {
    std::cerr << "Failed to create test root " << mTestRoot
              << ": " << PR_GetError() << std::endl;
  }
}


void
Config::setDefaultTestRoot()
{
#if defined(__linux__)
  // FIXME: POSIX specific
  std::string mounts;
  if (readTextFile("/proc/mounts", mounts))
  {
    char mountsc[mounts.size()+1];
    strcpy(mountsc, mounts.c_str());
    char* line = strtok(mountsc, "\n");
    while (line)
    {
      char* beg = strchr(line, ' ');
      if (!beg)
        continue;
      beg++;
      char* end = strchr(beg, ' ');
      if (!end)
        continue;
      *end = 0;
      if (strcmp(beg, SD_CARD_MOUNT) == 0)
      {
        mTestRoot = TESTROOT_SD_CARD;
        break;
      }
      line = strtok(NULL, "\n");
    }
  }
#elif defined(_WIN32)
  DWORD size = GetTempPath(0, nullptr);
  std::vector<char> path(size);
  if (GetTempPath(size, &path[0]))
  {
    mTestRoot = &path[0];
    std::replace(mTestRoot.begin(), mTestRoot.end(), '\\', '/');
    if (mTestRoot[mTestRoot.size() - 1] == '/')
      mTestRoot.erase(mTestRoot.end() - 1);
  }
#endif

  if (mTestRoot.empty())
    mTestRoot = TESTROOT_NO_SD_CARD;
}

