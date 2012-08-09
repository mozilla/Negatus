/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this file,
 You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "commands.h"


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

std::string ps() {
  FILE *p = checkPopen("ps | tr -s \" \" | cut -d' ' -f1,2,9 | tail +2", "r");
  std::ostringstream ret;
  char buffer[BUFSIZE];

  while (fgets(buffer, BUFSIZE, p)) {
    ret << buffer;
  }

  return ret.str();
}
