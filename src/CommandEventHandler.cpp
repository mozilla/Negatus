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

std::string CommandEventHandler::exec(std::string cmd) {
  std::vector<char> vcmd;
  vcmd.assign(cmd.begin(), cmd.end());
  vcmd.push_back('\0');

  // split by whitespace
  char *r_whitespace;
  char *found = strtok_r(&vcmd[0], " \t", &r_whitespace);
  if (!found) {
    return std::string("error: invalid cmd");
  }

  // handle first part separately, check if we have env vars
  int len = strlen(found);
  bool envs = false;
  for (int i = 0; i < len; ++i) {
    if (found[i] == '=') {
      envs = true;
      break;
    }
  }

  std::vector<std::string> env_names, env_values;
  // if we have envs we have to handle them separately
  if (envs) {
    char *r_env;
    char *env = strtok_r(found, ",", &r_env);
    // now we have something like env1=val1
    while (env) {
      int len = strlen(env);
      int pos = -1;
      for (int i = 0; i < len; ++i) {
        if (env[i] == '=') {
          pos = i;
          break;
        }
      }
      if (pos == -1) {
        continue;
      }

      std::string var(env, pos), val(env + pos + 1);
      env_names.push_back(var);
      env_values.push_back(val);

      env = strtok_r(NULL, ",", &r_env);
    }
    // skip past the env part
    found = strtok_r(NULL, " \t", &r_whitespace);
  }

  // extract the prog
  std::string prog(found);
  found = strtok_r(NULL, " \t", &r_whitespace);

  // what remains are the args
  std::vector<std::string> args;
  while (found) {
    args.push_back(std::string(found));
    found = strtok_r(NULL, " \t", &r_whitespace);
  }

  // std::cout << "Env vars: " << std::endl;
  // for (int i = 0; i < env_values.size(); ++i) {
  //   std::cout << env_names[i] << ": " << env_values[i] << std::endl;
  // }
  // std::cout << "Prog: " << prog << std::endl;
  // std::cout << "Args: " << std::endl;
  // for (int i = 0; i < args.size(); ++i) {
  //   std::cout << args[i] << std::endl;
  // }

  // set the env vars and backup the old vals
  std::vector<std::string> backup;
  for (int i = 0; i < env_names.size(); ++i) {
    const char *name = env_names[i].c_str();
    char *old = getenv(name);
    if (!old) {
      backup.push_back("");
    } else {
      backup.push_back(std::string(old));
    }
    setenv(name, env_values[i].c_str(), 1);
  }

  std::ostringstream to_exec;
  to_exec << prog << " ";
  for (int i = 0; i < args.size(); ++i) {
    to_exec << args[i] << " ";
  }

  FILE *p = checkPopen(to_exec.str(), "r");
  int status = pclose(p);

  // restore the env
  for (int i = 0; i < env_names.size(); ++i) {
    const char *name = env_names[i].c_str();
    if (backup[i].size() == 0) {
      unsetenv(name);
    } else {
      setenv(name, backup[i].c_str(), 1);
    }
  }

  if (status == 0) {
    return std::string("success");
  }
  return std::string("error");
}

std::string CommandEventHandler::hash(std::string path) {
  std::string newPath = actualPath(path);
  const char *cpath = newPath.c_str();
  char buffer[BUFSIZE];

  if (PR_Access(cpath, PR_ACCESS_READ_OK) != PR_SUCCESS) {
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
  std::ostringstream out;
  std::string newPath = actualPath(path);
  std::string ret = isDir(newPath);

  if (ret.compare("") != 0) {
    return ret;
  }

  PRDir *dir = PR_OpenDir(newPath.c_str());
  PRDirEntry *entry = PR_ReadDir(dir, PR_SKIP_BOTH);

  while (entry) {
    out << std::string(entry->name) << std::endl;
    entry = PR_ReadDir(dir, PR_SKIP_BOTH);
  }

  if (PR_CloseDir(dir) != PR_SUCCESS) {
    return std::string("error: could not close dir object");
  }
  return out.str();
}

std::string CommandEventHandler::mkdir(std::string path) {
  std::string newPath = actualPath(path);
  if (PR_MkDir(newPath.c_str(), 755) != PR_SUCCESS) {
    return std::string("Could not create directory " + path);
  }
  return std::string(path + " successfuly created");
}

// TODO push
// TODO pull
// TODO quit
// TODO rebt

std::string CommandEventHandler::rm(std::string path) {
  std::string newPath = actualPath(path);
  if (PR_Delete(newPath.c_str()) == PR_SUCCESS) {
    return std::string("");
  }
  return std::string("error: could not delete " + newPath);
}

std::string CommandEventHandler::rmdr(std::string path) {
  std::ostringstream out;
  do_rmdr(path, out);
  return out.str();
}

void CommandEventHandler::do_rmdr(std::string path, std::ostringstream &out) {
  std::string newPath = actualPath(path);
  std::string ret;
  const char *p = newPath.c_str();

  // if it's a file, nothing special to do
  if (isDir(newPath).compare("") != 0) {
    rm(newPath);
    return;
  }

  // recurse for dir contents
  PRDir *dir = PR_OpenDir(p);
  PRDirEntry *entry = PR_ReadDir(dir, PR_SKIP_BOTH);

  while (entry) {
    ret = rmdr(joinPaths(newPath, std::string(entry->name)));
    if (ret.compare("") != 0) {
      out << ret << "\r\n";
    }
    entry = PR_ReadDir(dir, PR_SKIP_BOTH);
  }
  if (PR_CloseDir(dir) != PR_SUCCESS) {
     out << "error: could not close dir object\r\n";
     // maybe return;
  }
  if (PR_RmDir(p) != PR_SUCCESS) {
    out << std::string("error: could not remove " + newPath) << "\r\n";
  }
}

std::string CommandEventHandler::testroot() {
  return std::string("/data/local");
}
