/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_registration_h
#define negatus_registration_h

#include <map>
#include <string>

typedef std::map<std::string, std::string> dict;

bool is_alpha(char c);
bool is_allowed_char(char c);
std::string urlencode(std::string plain);
std::string gen_query_url(dict data);
bool read_ini(std::string path, std::map<std::string, dict> & data);

#endif

