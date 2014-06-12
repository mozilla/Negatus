/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_strings_h
#define negatus_strings_h

#include <prio.h>
#include <prtypes.h>
#include <memory>
#include <string>

#define ENDL "\n"

// std::unique_ptr uses delete, this little functor + typedef
// is a shorthand for a unique_ptr that uses free, so we can store
// strdup'ed pointers in them.
struct freer {
  void operator()(void* p) {
    free(p);
  }
};

typedef std::unique_ptr<char[], freer> strdup_ptr;

std::string
trim(std::string s);

std::string
itoa(PRUint64 i);

std::string addrStr(PRNetAddr addr);

#endif
