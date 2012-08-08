/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this file,
 You can obtain one at http://mozilla.org/MPL/2.0/.
*/

// C++
#include <iostream>
#include <vector>
#include <sstream>

// C
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// NSPR
#include <prtime.h>

#define BUFSIZE 1024


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

bool cd(std::string path) {
  int success = chdir(path.c_str());
  return (success == 0);
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

bool dirw(std::string path) {
  // maybe check first that dir exists and is a dir
  int success = access(path.c_str(), W_OK);
  return (success == 0);
}

// TODO exec

std::string hash(std::string path) {
  const char *cpath = path.c_str();
  char buffer[BUFSIZE];

  if (access(cpath, F_OK | R_OK)) {
    return std::string("");
  }

  sprintf(buffer, "md5sum %s", cpath);
  return getCmdOutput(std::string(buffer));
}

std::string id() {
  std::string interfaces[3] = {"wlan0", "usb0", "lo"};
  FILE *iface;
  char buffer[BUFSIZE];

  for (int i = 0; i < 3; ++i) {
    sprintf(buffer, "/sys/class/net/%s/address", interfaces[i].c_str());
    iface = fopen(buffer, "r");
    if (!iface) {
      continue;
    }

    fgets(buffer, BUFSIZE, iface);
    fclose(iface);

    return std::string(buffer);
  }
  return std::string("00:00:00:00:00:00");
}

std::string os() {
  // not really supported yet. Best we could do is
  // cat /system/sources.xml | grep gaia and another grep for m-c
  return std::string("B2G");
}

// need to figure out how to build NSPR for ARM first
std::string systime() {
  PRTime now = PR_Now();
  PRExplodedTime ts;
  PR_ExplodeTime(now, PR_LocalTimeParameters, &ts);

  char buffer[BUFSIZE];
  sprintf(buffer, "%d/%02d/%02d %02d:%02d:%02d:%03d", ts.tm_year, ts.tm_month,
    ts.tm_mday, ts.tm_hour, ts.tm_min, ts.tm_sec, ts.tm_usec / 1000);

  return std::string(buffer);
}

// need to figure a better way
std::string uptime() {
  return getCmdOutput("uptime");
}

// TODO uptimimilis
// TODO rotation

// need to figure a better way
std::string screen() {
  return readTextFile("/sys/devices/virtual/graphics/fb0/modes");
}

std::string memory() {
  FILE *meminfo = fopen("/proc/meminfo", "r");
  if (!meminfo) {
    fprintf(stderr, "Error on fopen: /proc/meminfo, with mode r.\n");
    exit(1);
  }

  unsigned int total, available;
  char buffer[BUFSIZE];
  fgets(buffer, BUFSIZE, meminfo);
  sscanf(buffer + getFirstIntPos(buffer, BUFSIZE), "%u", &total);
  fgets(buffer, BUFSIZE, meminfo);
  sscanf(buffer + getFirstIntPos(buffer, BUFSIZE), "%u", &available);

  sprintf(buffer, "Total: %d; Available: %d.\n", total * 1000, available * 1000);
  return std::string(buffer);
}

std::string power() {
  std::ostringstream ret;

  ret << "Power status:" << std::endl;
  ret << "\tCurrent %: ";
  ret << readTextFile("/sys/class/power_supply/battery/capacity");
  ret << "\tStatus: ";
  ret << readTextFile("/sys/class/power_supply/battery/status");

  return ret.str();
}


int main(int argc, char **argv) {

  std::cout << getCmdOutput("uname -s -m -r") << std::endl;

  cd("/data/local");
  std::cout << cwd() << std::endl;

  std::cout << clok() << std::endl;
  std::cout << time(NULL) << std::endl;

  std::cout << dirw("/") << std::endl;
  std::cout << dirw("/data/local") << std::endl;

  std::cout << hash("/init.rc") << std::endl;
  std::cout << hash("/weird/path") << std::endl;

  std::cout << id() << std::endl;

  std::cout << uptime() << std::endl;

  std::cout << systime() << std::endl;

  std::cout << screen() << std::endl;

  std::cout << power() << std::endl;

  std::cout << memory() << std::endl;

  return 0;
}
