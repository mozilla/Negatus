/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "Hash.h"
#include "Subprocess.h"

#include <sstream>

std::string
fileHash(std::string path)
{
  std::ostringstream ss;
  ss << "md5sum " << path;
  std::string hash = getCmdOutput(ss.str());
  // strip everything past actual hash string
  return hash.substr(0, hash.find(" "));
}
