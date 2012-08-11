/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this file,
 You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "misc.h"


FILE *checkPopen(std::string cmd, std::string mode) {
  FILE *fp = popen(cmd.c_str(), mode.c_str());
  if (!fp) {
    fprintf(stderr, "Error on popen: %s, with mode %s.\n", cmd.c_str(), mode.c_str());
    exit(1);
  }
  return fp;
}

std::string getCmdOutput(std::string cmd) {
  FILE *fp = checkPopen(cmd, "r");
  char buffer[BUFSIZE];
  std::string output;

  while (fgets(buffer, BUFSIZE, fp)) {
    output += std::string(buffer);
  }

  pclose(fp);
  return output;
}

std::string readTextFile(std::string path) {
  const char *cpath = path.c_str();

  FILE *fp = fopen(cpath, "r");
  if (!fp) {
    fprintf(stderr, "Error on fopen: %s, with mode r.\n", cpath);
    exit(1);
  }

  char buffer[BUFSIZE];
  std::string output;

  while (fgets(buffer, BUFSIZE, fp)) {
    output += std::string(buffer);
  }

  fclose(fp);
  return output;
}

int getFirstIntPos(char *str, int limit) {
  for (int i = 0; i < limit; ++i) {
    if (str[i] >= '0' && str[i] <= '9') {
      return i;
    }
  }

  return -1;
}

std::string joinPaths(std::string p1, std::string p2) {
  if (p1[p1.length() - 1] == '/') {
    p1 = p1.substr(0, p1.length() - 1);
  }
  if (p2[0] == '/') {
    p2 = p2.substr(1);
  }
  return p1 + "/" + p2;
}
