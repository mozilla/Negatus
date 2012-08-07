/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this file,
 You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <iostream>
#include <vector>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


#define BUFSIZE 1024


FILE *checkPopen(std::string cmd, std::string mode) {
  FILE *fp = popen(cmd.c_str(), mode.c_str());
  if (!fp) {
    fprintf(stderr, "Error on popen: %s, with mode %s.\n", cmd.c_str(), mode.c_str());
    exit(1);
  }
  return fp;
}

bool cd(std::string path) {
  int success = chdir(path.c_str());
  return (success == 1);
}

std::string cwd() {
  char output[BUFSIZE];
  getcwd(output, BUFSIZE);

  return std::string(output);
}

uint64_t clok() {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (uint64_t) ts.tv_sec * 1000 + ts.tv_nsec / 1000000.0;
}

int main(int argc, char **argv) {

  char output[BUFSIZE];

  FILE *uname = checkPopen("uname -s -m -r", "r");
  while (fgets(output, BUFSIZE, uname)) {
    puts(output);
  }

  cd("/data/local");
  std::cout << cwd() << std::endl;

  std::cout << clok() << std::endl;
  std::cout << time(NULL) << std::endl;

  return 0;
}
