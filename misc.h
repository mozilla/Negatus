/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this file,
 You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef __MISC_H__
#define __MISC_H__

#include <string>

#include <stdio.H>
#include <stdlib.h>

#define BUFSIZE 1024


FILE *checkPopen(std::string cmd, std::string mode);
std::string getCmdOutput(std::string cmd);
std::string readTextFile(std::string path);
int getFirstIntPos(char *str, int limit);

#endif
