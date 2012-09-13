/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_shell_h
#define negatus_shell_h

#include <string>
#include <vector>

#define TESTROOT "/data/local"


std::string readTextFile(std::string path);
int getFirstIntPos(char *str, int limit);
std::string joinPaths(std::string p1, std::string p2);
void do_rmdr(std::string path, std::ostringstream &out);

// Command implementations
std::string cd(std::vector<std::string>& args);
std::string clok(std::vector<std::string>& args);
std::string cwd(std::vector<std::string>& args);
std::string dirw(std::vector<std::string>& args);
std::string exec(std::vector<std::string>& args);
std::string hash(std::vector<std::string>& args);
std::string isDir(std::vector<std::string>& args);
std::string isDir(std::string path);
std::string info(std::vector<std::string>& args);
std::string ls(std::vector<std::string>& args);
std::string mkdr(std::vector<std::string>& args);
std::string ps(std::vector<std::string>& args);
std::string rm(std::vector<std::string>& args);
std::string rm(std::string path);
std::string rmdr(std::vector<std::string>& args);
std::string testroot(std::vector<std::string>& args);
std::string ver(std::vector<std::string>& args);

std::string id();
std::string memory();
std::string os();
std::string power();
std::string screen();
std::string uptime();
std::string uptimemillis();
std::string systime();

#endif
