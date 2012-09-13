/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Hash.h"
#include "Logging.h"
#include "Shell.h"
#include "Strings.h"
#include "Subprocess.h"

#include <sstream>

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PERMS755 PR_IRWXU | PR_IRGRP | PR_IXGRP | PR_IROTH | PR_IXOTH


std::string
readTextFile(std::string path)
{
  const char *cpath = path.c_str();

  FILE *fp = fopen(cpath, "r");
  if (!fp)
  {
    fprintf(stderr, "Error on fopen: %s, with mode r.\n", cpath);
    return agentWarn("Cannot open file");
  }

  char buffer[BUFSIZE];
  std::ostringstream output;

  while (fgets(buffer, BUFSIZE, fp))
    output << std::string(buffer);

  fclose(fp);
  std::string str = output.str();
  if (str.size())
    str.erase(str.size() - 1);
  return str;
}


int
getFirstIntPos(char *str, int limit)
{
  for (int i = 0; i < limit; ++i)
  {
    if (str[i] >= '0' && str[i] <= '9')
      return i;
  }

  return -1;
}


std::string
joinPaths(std::string p1, std::string p2)
{
  if (p1[p1.length() - 1] == '/')
    p1 = p1.substr(0, p1.length() - 1);
  if (p2[0] == '/')
    p2 = p2.substr(1);
  return p1 + "/" + p2;
}




std::string
cd(std::vector<std::string>& args)
{
  std::string path(args.size() ? args[0] : "");
  if (path.compare("") == 0)
    path = "/";
  const char *p = path.c_str();

  // check if path exists and it is a dir
  std::string ret = isDir(path);
  if (ret.compare("TRUE") != 0)
  {
    if (ret.compare("FALSE") == 0)
      return agentWarn("path is not a dir");
    return ret;
  }

  // check for read permissions
  PRStatus success = PR_Access(p, PR_ACCESS_READ_OK);
  if (success == PR_SUCCESS)
  {
    // update the cwd
    int s = chdir(p);
    if (s == 0)
      return "";
  }
  return agentWarn("no permissions");
}


std::string
cwd(std::vector<std::string>& args)
{
  char buffer[BUFSIZE];
  getcwd(buffer, BUFSIZE);
  return std::string(buffer);
}


std::string
clok(std::vector<std::string>& args)
{
  PRUint64 now = PR_Now() / PR_USEC_PER_SEC;
  return itoa(now);
}


std::string
dirw(std::vector<std::string>& args)
{
  std::string path(args.size() ? args[0] : "");
  std::string ret = isDir(path);
  if (ret.compare("TRUE") != 0)
  {
    if (ret.compare("FALSE") == 0)
      return agentWarn("path is not a dir");
    return ret;
  }
  if (PR_Access(path.c_str(), PR_ACCESS_WRITE_OK) == PR_SUCCESS)
    return std::string(path + " is writable");
  return std::string(path + " is not writable");
}


std::string
exec(std::vector<std::string>& args)
{
  if (args.size() < 1)
    return agentWarn("command not specified");

  // delete double quotes from args[0], easier to parse
  if (args[0][0] == '"')
  {
    args[0].erase(0, 1); // delete the beginning one
    args[0].erase(args[0].size() - 1); // delete the ending one
  }

  std::vector<std::string>::iterator argi = args.begin();

  // handle first part separately, check if we have env vars
  bool envs = args[0].find('=') != std::string::npos;

  std::vector<std::string> env_names, env_values;
  // if we have envs we have to handle them separately
  if (envs)
  {
    char envVarStr[(*argi).size() + 1];
    (*argi).copy(envVarStr, (*argi).size());
    envVarStr[(*argi).size()] = 0;
    char *r_env;
    char *env = strtok_r(envVarStr, ",", &r_env);
    // now we have something like env1=val1
    while (env)
    {
      int len = strlen(env);
      int pos = -1;
      for (int i = 0; i < len; ++i)
      {
        if (env[i] == '=')
        {
          pos = i;
          break;
        }
      }
      if (pos == -1)
        continue;

      std::string var(env, pos), val(env + pos + 1);
      env_names.push_back(var);
      env_values.push_back(val);

      env = strtok_r(NULL, ",", &r_env);
    }
    // skip past the env part
    argi++;
  }

  // extract the prog
  std::string prog(*argi++);

  // what remains are the args

  // set the env vars and backup the old vals
  std::vector<std::string> backup;
  for (int i = 0; i < env_names.size(); ++i)
  {
    const char *name = env_names[i].c_str();
    char *old = getenv(name);
    if (!old)
      backup.push_back("");
    else
      backup.push_back(std::string(old));
    setenv(name, env_values[i].c_str(), 1);
  }

  std::ostringstream to_exec;
  to_exec << prog << " ";
  for (; argi != args.end(); ++argi)
    to_exec << *argi << " ";

  FILE *p = checkPopen(to_exec.str(), "r");

  // get the output so pclose won't cry even on successful calls
  char buffer[BUFSIZE];
  std::ostringstream output;

  while (fgets(buffer, BUFSIZE, p))
    output << std::string(buffer);

  int status = pclose(p);

  // restore the env
  for (int i = 0; i < env_names.size(); ++i)
  {
    const char *name = env_names[i].c_str();
    if (backup[i].size() == 0)
      unsetenv(name);
    else
      setenv(name, backup[i].c_str(), 1);
  }

  if (status == 0)
    return std::string("success");
  return agentWarn("error");
}


std::string
hash(std::vector<std::string>& args)
{
  if (args.size() < 1)
    return agentWarnInvalidNumArgs(1);
  std::string path = args[0];

  if (PR_Access(path.c_str(), PR_ACCESS_READ_OK) != PR_SUCCESS)
  {
    std::ostringstream err;
    err << "Couldn't calculate hash for file " << path << ": cannot open file for reading";
    return agentWarn(err.str());
  }

  return fileHash(path);
}


std::string
info(std::vector<std::string>& args)
{
  if (args.size() != 1)
    return agentWarnInvalidNumArgs(1);
  if (args[0].compare("id") == 0)
    return id();
  else if (args[0].compare("os") == 0)
    return os();
  else if (args[0].compare("systime") == 0)
    return systime();
  else if (args[0].compare("uptime") == 0)
    return uptime();
  else if (args[0].compare("uptimemillis") == 0)
    return uptimemillis();
  else if (args[0].compare("screen") == 0)
    return screen();
  else if (args[0].compare("memory") == 0)
    return memory();
  else if (args[0].compare("power") == 0)
    return power();
  return agentWarn("Invalid info subcommand.");
}

std::string
id()
{
  std::string interfaces[3] = {"wlan0", "usb0", "lo"};
  FILE *iface;
  char buffer[BUFSIZE];

  for (int i = 0; i < 3; ++i)
  {
    sprintf(buffer, "/sys/class/net/%s/address", interfaces[i].c_str());
    iface = fopen(buffer, "r");
    if (!iface)
      continue;

    fgets(buffer, BUFSIZE, iface);
    buffer[strlen(buffer) - 1] = '\0'; // remove extra newline
    fclose(iface);

    return std::string(buffer);
  }
  return std::string("00:00:00:00:00:00");
}


std::string
os()
{
  // not really supported yet. Best we could do is
  // cat /system/sources.xml | grep gaia and another grep for m-c
  return std::string("B2G");
}


std::string
systime()
{
  PRTime now = PR_Now();
  PRExplodedTime ts;
  PR_ExplodeTime(now, PR_LocalTimeParameters, &ts);

  char buffer[BUFSIZE];
  sprintf(buffer, "%d/%02d/%02d %02d:%02d:%02d:%03d", ts.tm_year, ts.tm_month,
    ts.tm_mday, ts.tm_hour, ts.tm_min, ts.tm_sec, ts.tm_usec / 1000);

  return std::string(buffer);
}


// need to figure a better way
std::string
uptime()
{
  return getCmdOutput("uptime");
}


// this is not cross platform
std::string
uptimemillis()
{
  std::string uptime_file = readTextFile("/proc/uptime");
  double uptime;
  sscanf(&uptime_file[0], "%lf", &uptime);
  uptime *= 1000;
  std::ostringstream out;
  out << (int) uptime;
  return out.str();
}
// TODO rotation

// need to figure a better way
std::string
screen()
{
  return readTextFile("/sys/devices/virtual/graphics/fb0/modes");
}


std::string
memory()
{
  FILE *meminfo = fopen("/proc/meminfo", "r");
  if (!meminfo)
  {
    return agentWarn("Error on fopen: /proc/meminfo, with mode r");
  }

  unsigned int total, available;
  char buffer[BUFSIZE];
  fgets(buffer, BUFSIZE, meminfo);
  sscanf(buffer + getFirstIntPos(buffer, BUFSIZE), "%u", &total);
  fgets(buffer, BUFSIZE, meminfo);
  sscanf(buffer + getFirstIntPos(buffer, BUFSIZE), "%u", &available);

  sprintf(buffer, "Total: %d; Available: %d.", total * 1000, available * 1000);
  return std::string(buffer);
}


std::string
power()
{
  std::ostringstream ret;

  ret << "Power status:" << ENDL;
  ret << "\tCurrent %: ";
  ret << readTextFile("/sys/class/power_supply/battery/capacity");
  ret << "\tStatus: ";
  ret << readTextFile("/sys/class/power_supply/battery/status");

  return ret.str();
}


std::string
ps(std::vector<std::string>& args)
{
  // might need to change command on linux since the output is different on B2G
  return getCmdOutput("ps | tr -s \" \" | cut -d' ' -f1,2,9 | tail +2");
}


std::string
isDir(std::vector<std::string>& args)
{
  if (args.size() < 1)
    return agentWarnInvalidNumArgs(1);
  return isDir(args[0]);
}


std::string
isDir(std::string path)
{
  // check if path exists and it is a dir
  PRFileInfo info;
  const char *p = path.c_str();
  PRStatus success = PR_GetFileInfo(p, &info);
  if (success == PR_SUCCESS)
  {
    if (info.type == PR_FILE_DIRECTORY)
      return std::string("TRUE");
    return std::string("FALSE");
  }
  return agentWarn("invalid path");
}

std::string
ls(std::vector<std::string>& args)
{
  std::string path(args.size() ? args[0] : "");
  if (path.compare("") == 0)
    path = ".";
  std::ostringstream out;
  std::string ret = isDir(path);

  if (ret.compare("TRUE") != 0)
  {
    if (ret.compare("FALSE") == 0)
      return agentWarn("path is not a dir");
    return ret;
  }

  PRDir *dir = PR_OpenDir(path.c_str());
  PRDirEntry *entry = PR_ReadDir(dir, PR_SKIP_BOTH);

  while (entry)
  {
    out << std::string(entry->name) << ENDL;
    entry = PR_ReadDir(dir, PR_SKIP_BOTH);
  }

  if (PR_CloseDir(dir) != PR_SUCCESS)
    return agentWarn("could not close dir object");

  std::string output = out.str();
  if (output.size())
    output.erase(output.size() - 1); // erase the extra newline
  return output;
}


std::string
mkdr(std::vector<std::string>& args)
{
  if (args.size() < 1)
    return agentWarnInvalidNumArgs(1);
  std::string path = args[0];
  if (PR_MkDir(path.c_str(), PERMS755) != PR_SUCCESS)
    return agentWarn("Could not create directory " + path);
  return std::string(path + " successfuly created");
}


std::string
rm(std::vector<std::string>& args)
{
  if (args.size() < 1)
    return agentWarnInvalidNumArgs(1);
  return rm(args[0]);
}


std::string
rm(std::string path)
{
  if (PR_Delete(path.c_str()) == PR_SUCCESS)
    return std::string("removing file" + path);
  return agentWarn("error: could not delete " + path);
}


std::string
rmdr(std::vector<std::string>& args)
{
  if (args.size() < 1)
    return agentWarnInvalidNumArgs(1);
  std::string path = args[0];
  std::ostringstream out;
  do_rmdr(path, out);
  return out.str();
}


void
do_rmdr(std::string path, std::ostringstream &out)
{
  std::string ret = isDir(path);
  const char *p = path.c_str();

  // if it's a file, nothing special to do
  if (ret.compare("TRUE") != 0)
  {
    if (ret.compare("FALSE") == 0)
    {
      rm(path);
      return;
    }
    // if this does not exist, return
    out << ret;
    return;
  }

  // recurse for dir contents
  PRDir *dir = PR_OpenDir(p);
  PRDirEntry *entry = PR_ReadDir(dir, PR_SKIP_BOTH);

  while (entry)
  {
    do_rmdr(joinPaths(path, std::string(entry->name)), out);
    entry = PR_ReadDir(dir, PR_SKIP_BOTH);
  }
  if (PR_CloseDir(dir) != PR_SUCCESS)
  {
    out << "error: could not close dir object" << ENDL;
    // maybe return;
  }
  if (PR_RmDir(p) != PR_SUCCESS)
    out << std::string("error: could not remove " + path) << ENDL;
}


std::string
testroot(std::vector<std::string>& args)
{
  return std::string(TESTROOT);
}


std::string
ver(std::vector<std::string>& args)
{
  return std::string("SUTAgentAndroid Version 1.13");
}
