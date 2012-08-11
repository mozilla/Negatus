/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this file,
 You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "CommandEventHandler.h"


CommandEventHandler::CommandEventHandler() {
  session = new SessionEventHandler;
}

CommandEventHandler::CommandEventHandler(SessionEventHandler *session) {
  this->session = session;
}

std::string CommandEventHandler::actualPath(std::string path) {
  std::string newPath = session->cwd;
  if (path[0] != '/') {
    newPath += path;
  } else {
    newPath = path;
  }
  return newPath;
}

std::string CommandEventHandler::cd(std::string path) {
  std::string newPath = actualPath(path);
  // check if path exists and it is a dir
  std::string ret = isDir(newPath);
  if (ret.compare("") != 0) {
    return ret;
  }
  // check for read permissions
  PRStatus success = PR_Access(newPath.c_str(), PR_ACCESS_READ_OK);
  if (success == PR_SUCCESS) {
    // update the cwd
    session->cwd = newPath;
    return std::string("");
  }
  return std::string("error: no permissions");
}

std::string CommandEventHandler::cwd() {
  return session->cwd;
}

PRUint64 CommandEventHandler::clok() {
  PRTime now = PR_Now();
  return (PRUint64) now / PR_USEC_PER_MSEC;
}

std::string CommandEventHandler::dirw(std::string path) {
  std::string newPath = actualPath(path);
  std::string ret = isDir(newPath);
  if (ret.compare("") != 0) {
    return ret;
  }
  if (PR_Access(newPath.c_str(), PR_ACCESS_READ_OK) == PR_SUCCESS) {
    return std::string(path + " is writable");
  }
  return std::string(path + " is not writable");
}

// TODO exec

std::string CommandEventHandler::hash(std::string path) {
  const char *cpath = path.c_str();
  char buffer[BUFSIZE];

  if (access(cpath, F_OK | R_OK)) {
    return std::string("");
  }

  sprintf(buffer, "md5sum %s", cpath);
  return getCmdOutput(std::string(buffer));
}

std::string CommandEventHandler::id() {
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

std::string CommandEventHandler::os() {
  // not really supported yet. Best we could do is
  // cat /system/sources.xml | grep gaia and another grep for m-c
  return std::string("B2G");
}

// need to figure out how to build NSPR for ARM first
std::string CommandEventHandler::systime() {
  PRTime now = PR_Now();
  PRExplodedTime ts;
  PR_ExplodeTime(now, PR_LocalTimeParameters, &ts);

  char buffer[BUFSIZE];
  sprintf(buffer, "%d/%02d/%02d %02d:%02d:%02d:%03d", ts.tm_year, ts.tm_month,
    ts.tm_mday, ts.tm_hour, ts.tm_min, ts.tm_sec, ts.tm_usec / 1000);

  return std::string(buffer);
}

// need to figure a better way
std::string CommandEventHandler::uptime() {
  return getCmdOutput("uptime");
}

// TODO uptimimilis
// TODO rotation

// need to figure a better way
std::string CommandEventHandler::screen() {
  return readTextFile("/sys/devices/virtual/graphics/fb0/modes");
}

std::string CommandEventHandler::memory() {
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

std::string CommandEventHandler::power() {
  std::ostringstream ret;

  ret << "Power status:" << std::endl;
  ret << "\tCurrent %: ";
  ret << readTextFile("/sys/class/power_supply/battery/capacity");
  ret << "\tStatus: ";
  ret << readTextFile("/sys/class/power_supply/battery/status");

  return ret.str();
}

std::string CommandEventHandler::ps() {
  FILE *p = checkPopen("ps | tr -s \" \" | cut -d' ' -f1,2,9 | tail +2", "r");
  std::ostringstream ret;
  char buffer[BUFSIZE];

  while (fgets(buffer, BUFSIZE, p)) {
    ret << buffer;
  }

  return ret.str();
}

std::string CommandEventHandler::isDir(std::string path) {
  std::string newPath = actualPath(path);

  // check if path exists and it is a dir
  PRFileInfo info;
  const char *p = newPath.c_str();
  PRStatus success = PR_GetFileInfo(p, &info);
  if (success == PR_SUCCESS) {
    if (info.type == PR_FILE_DIRECTORY) {
      return std::string("");
    }
    return std::string("error: path is not a directory");
  }
  return std::string("error: invalid path");
}

std::string CommandEventHandler::ls(std::string path) {
  struct dirent *dp;
  DIR *dirp = opendir(path.c_str());
  std::ostringstream ret;

  if (!dirp) {
    return std::string("");
  }

  dp = readdir(dirp);
  while (dp) {
    ret << std::string(dp->d_name) << std::endl;
    dp = readdir(dirp);
  }

  closedir(dirp);
  return ret.str();
}

std::string CommandEventHandler::mkdir(std::string path) {
  char buffer[BUFSIZE];
  sprintf(buffer, "mkdir %s 2>&1", path.c_str());
  FILE *s = checkPopen(std::string(buffer), "r");
  std::ostringstream output;

  memset(buffer, 0, BUFSIZE);
  fgets(buffer, BUFSIZE, s);

  if (strlen(buffer) <= 1) {
    return std::string(path + " successfuly created");
  }
  return std::string("Could not create directory " + path);
}

// TODO push
// TODO pull
// TODO quit
// TODO rebt

bool CommandEventHandler::rm(std::string path) {
  char buffer[BUFSIZE];
  sprintf(buffer, "rm %s 2>&1", path.c_str());
  FILE *s = checkPopen(std::string(buffer), "r");
  std::ostringstream output;

  memset(buffer, 0, BUFSIZE);
  fgets(buffer, BUFSIZE, s);

  if (strlen(buffer) <= 1) {
    return true;
  }
  return false;
}

bool CommandEventHandler::rmdr(std::string path) {
  char buffer[BUFSIZE];
  sprintf(buffer, "rm -r %s 2>&1", path.c_str());
  FILE *s = checkPopen(std::string(buffer), "r");
  std::ostringstream output;

  memset(buffer, 0, BUFSIZE);
  fgets(buffer, BUFSIZE, s);

  if (strlen(buffer) <= 1) {
    return true;
  }
  return false;
}

std::string CommandEventHandler::testroot() {
  return std::string("/data/local");
}
