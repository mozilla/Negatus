/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_config_h
#define negatus_config_h

#include <string>

#define SD_CARD_MOUNT "/mnt/sdcard"
#define TESTROOT_SD_CARD "/mnt/sdcard/test"
#ifdef NEGATUS_LINUX_DESKTOP_BUILD
#define TESTROOT_NO_SD_CARD "/tmp"
#else
#define TESTROOT_NO_SD_CARD "/data/local/tmp"
#endif

class Config
{
public:
  static Config* instance();
  std::string mTestRoot;

  /* If testRoot is empty, use default, preferring attached SD card. */
  void setTestRoot(std::string testRoot);

private:
  static Config* mInstance;

  Config();
  void setDefaultTestRoot();
};

#endif
