/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "CommandEventHandler.h"
#include "BufferedSocket.h"
#include "SessionEventHandler.h"
#include "Strings.h"

#include <iostream>
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <prproces.h>
#include <prtypes.h>

CommandEventHandler::CommandLine::CommandLine(std::string line)
  : cmd(""), arg("")
{
  char linec[line.size()];
  strcpy(linec, line.c_str());
  char* cmdc = strtok(linec, " ");
  if (!cmdc)
    return;
  cmd = cmdc;
  char* argc = strtok(NULL, " ");
  while (argc) {
    arg += argc;
    arg += " ";
    argc = strtok(NULL, " ");
  }
  arg = trim(arg);
}


CommandEventHandler::CommandEventHandler(BufferedSocket& bufSocket,
                                         SessionEventHandler& session)
  : mBufSocket(bufSocket), mSession(session), mPrompt("$>")
{
  sendPrompt();
}


void
CommandEventHandler::getPollDescs(std::vector<PRPollDesc>& descs)
{
  if (!mBufSocket.closed())
  {
    PRPollDesc desc;
    desc.fd = mBufSocket.fd();
    desc.in_flags = PR_POLL_READ;
    descs.push_back(desc);
  }
}


void
CommandEventHandler::handleEvent(PRPollDesc desc)
{
  if (desc.fd != mBufSocket.fd())
    return;
  if (!(desc.out_flags & PR_POLL_READ))
    return;
  
  while (true)
  {
    std::stringstream buf;
    PRUint32 numRead = mBufSocket.readLine(buf);
    if (!numRead)
      break;
    std::string line(trim(buf.str()));
    handleLine(line);
    sendPrompt();
  }
  if (mBufSocket.closed())
    close();
}


void
CommandEventHandler::handleLine(std::string line)
{
  CommandLine cl(line);
  if (cl.cmd.empty())
    return;
  std::string result(agentWarn("unknown command"));
  if (cl.cmd.compare("cd") == 0)
    result = cd(cl.arg);
  else if (cl.cmd.compare("cwd") == 0)
    result = cwd();
  else if (cl.cmd.compare("clok") == 0)
    result = clok();
  else if (cl.cmd.compare("dirw") == 0)
    result = dirw(cl.arg);
  else if (cl.cmd.compare("exec") == 0)
    result = exec(cl.arg);
  else if (cl.cmd.compare("hash") == 0)
    result = hash(cl.arg);
  else if (cl.cmd.compare("id") == 0)
    result = id();
  else if (cl.cmd.compare("os") == 0)
    result = os();
  else if (cl.cmd.compare("systime") == 0)
    result = systime();
  else if (cl.cmd.compare("uptime") == 0)
    result = uptime();
  else if (cl.cmd.compare("screen") == 0)
    result = screen();
  else if (cl.cmd.compare("memory") == 0)
    result = memory();
  else if (cl.cmd.compare("power") == 0)
    result = power();
  else if (cl.cmd.compare("ps") == 0)
    result = ps();
  else if (cl.cmd.compare("isDir") == 0)
    result = isDir(cl.arg);
  else if (cl.cmd.compare("ls") == 0)
    result = ls(cl.arg);
  else if (cl.cmd.compare("mkdir") == 0)
    result = mkdir(cl.arg);
  else if (cl.cmd.compare("quit") == 0)
    result = quit();
  else if (cl.cmd.compare("rm") == 0)
    result = rm(cl.arg);
  else if (cl.cmd.compare("rmdr") == 0)
    result = rmdr(cl.arg);
  else if (cl.cmd.compare("testroot") == 0)
    result = testroot();
  if (!result.empty())
    mBufSocket.write(result);
}


void
CommandEventHandler::sendPrompt()
{
  mBufSocket.write(mPrompt.c_str(), mPrompt.size());
}


std::string
CommandEventHandler::agentWarn(std::string errStr)
{
  return "### AGENT-WARNING: " + errStr + ENDL;
}


FILE*
CommandEventHandler::checkPopen(std::string cmd, std::string mode)
{
  FILE *fp = popen(cmd.c_str(), mode.c_str());
  if (!fp)
  {
    fprintf(stderr, "Error on popen: %s, with mode %s.\n", cmd.c_str(),
            mode.c_str());
    exit(1);
  }
  return fp;
}


std::string
CommandEventHandler::getCmdOutput(std::string cmd)
{
  FILE *fp = checkPopen(cmd, "r");
  char buffer[BUFSIZE];
  std::string output;

  while (fgets(buffer, BUFSIZE, fp))
    output += std::string(buffer);

  pclose(fp);
  return output;
}


std::string
CommandEventHandler::readTextFile(std::string path)
{
  const char *cpath = path.c_str();

  FILE *fp = fopen(cpath, "r");
  if (!fp)
  {
    fprintf(stderr, "Error on fopen: %s, with mode r.\n", cpath);
    exit(1);
  }

  char buffer[BUFSIZE];
  std::string output;

  while (fgets(buffer, BUFSIZE, fp))
    output += std::string(buffer);

  fclose(fp);
  return output;
}


int
CommandEventHandler::getFirstIntPos(char *str, int limit)
{
  for (int i = 0; i < limit; ++i)
  {
    if (str[i] >= '0' && str[i] <= '9')
      return i;
  }

  return -1;
}


std::string
CommandEventHandler::joinPaths(std::string p1, std::string p2)
{
  if (p1[p1.length() - 1] == '/')
    p1 = p1.substr(0, p1.length() - 1);
  if (p2[0] == '/')
    p2 = p2.substr(1);
  return p1 + "/" + p2;
}


std::string
CommandEventHandler::cd(std::string path)
{
  if (path.compare("") == 0)
    path = "/";
  const char *p = path.c_str();

  // check if path exists and it is a dir
  std::string ret = isDir(path);
  if (ret.compare("") != 0)
    return ret;

  // check for read permissions
  PRStatus success = PR_Access(p, PR_ACCESS_READ_OK);
  if (success == PR_SUCCESS)
  {
    // update the cwd
    int s = chdir(p);
    if (s == 0)
      return cwd();
  }
  return agentWarn("no permissions");
}


std::string
CommandEventHandler::cwd()
{
  char buffer[BUFSIZE];
  getcwd(buffer, BUFSIZE);
  return std::string(buffer) + ENDL;
}


std::string
CommandEventHandler::clok()
{
  PRUint64 now = PR_Now() / PR_USEC_PER_SEC;
  return itoa(now) + ENDL;
}


std::string
CommandEventHandler::dirw(std::string path)
{
  std::string ret = isDir(path);
  if (ret.compare("") != 0)
    return ret;
  if (PR_Access(path.c_str(), PR_ACCESS_WRITE_OK) == PR_SUCCESS)
    return std::string(path + " is writable" + ENDL);
  return std::string(path + " is not writable" + ENDL);
}


std::string
CommandEventHandler::exec(std::string cmd)
{
  std::vector<char> vcmd;
  vcmd.assign(cmd.begin(), cmd.end());
  vcmd.push_back('\0');

  // split by whitespace
  char *r_whitespace;
  char *found = strtok_r(&vcmd[0], " \t", &r_whitespace);
  if (!found)
    return agentWarn("invalid cmd");

  // handle first part separately, check if we have env vars
  int len = strlen(found);
  bool envs = false;
  for (int i = 0; i < len; ++i)
  {
    if (found[i] == '=')
    {
      envs = true;
      break;
    }
  }

  std::vector<std::string> env_names, env_values;
  // if we have envs we have to handle them separately
  if (envs)
  {
    char *r_env;
    char *env = strtok_r(found, ",", &r_env);
    // now we have something like env1=val1
    while (env) {
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
    found = strtok_r(NULL, " \t", &r_whitespace);
  }

  // extract the prog
  std::string prog(found);
  found = strtok_r(NULL, " \t", &r_whitespace);

  // what remains are the args
  std::vector<std::string> args;
  while (found)
  {
    args.push_back(std::string(found));
    found = strtok_r(NULL, " \t", &r_whitespace);
  }

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
  for (int i = 0; i < args.size(); ++i)
    to_exec << args[i] << " ";

  FILE *p = checkPopen(to_exec.str(), "r");
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
    return std::string("success") + ENDL;
  return agentWarn("error");
}


std::string
CommandEventHandler::hash(std::string path)
{
  const char *cpath = path.c_str();
  char buffer[BUFSIZE];

  if (PR_Access(cpath, PR_ACCESS_READ_OK) != PR_SUCCESS)
    return std::string("");

  sprintf(buffer, "md5sum %s", cpath);
  return getCmdOutput(std::string(buffer));
}


std::string
CommandEventHandler::id()
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
    fclose(iface);

    return std::string(buffer);
  }
  return std::string("00:00:00:00:00:00");
}


std::string
CommandEventHandler::os()
{
  // not really supported yet. Best we could do is
  // cat /system/sources.xml | grep gaia and another grep for m-c
  return std::string("B2G");
}


std::string
CommandEventHandler::systime()
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
CommandEventHandler::uptime()
{
  return getCmdOutput("uptime");
}


// TODO uptimimilis
// TODO rotation

// need to figure a better way
std::string
CommandEventHandler::screen()
{
  return readTextFile("/sys/devices/virtual/graphics/fb0/modes");
}


std::string
CommandEventHandler::memory()
{
  FILE *meminfo = fopen("/proc/meminfo", "r");
  if (!meminfo)
  {
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


std::string
CommandEventHandler::power()
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
CommandEventHandler::ps()
{
  FILE *p = checkPopen("ps | tr -s \" \" | cut -d' ' -f1,2,9 | tail +2", "r");
  std::ostringstream ret;
  char buffer[BUFSIZE];

  while (fgets(buffer, BUFSIZE, p))
    ret << buffer;

  return ret.str();
}


std::string
CommandEventHandler::isDir(std::string path)
{
  // check if path exists and it is a dir
  PRFileInfo info;
  const char *p = path.c_str();
  PRStatus success = PR_GetFileInfo(p, &info);
  if (success == PR_SUCCESS)
  {
    if (info.type == PR_FILE_DIRECTORY)
      return std::string("");
    return agentWarn("path is not a directory");
  }
  return agentWarn("invalid path");
}


std::string
CommandEventHandler::ls(std::string path)
{
  if (path.compare("") == 0)
    path = ".";
  std::ostringstream out;
  std::string ret = isDir(path);

  if (ret.compare("") != 0)
    return ret;

  PRDir *dir = PR_OpenDir(path.c_str());
  PRDirEntry *entry = PR_ReadDir(dir, PR_SKIP_BOTH);

  while (entry)
  {
    out << std::string(entry->name) << ENDL;
    entry = PR_ReadDir(dir, PR_SKIP_BOTH);
  }

  if (PR_CloseDir(dir) != PR_SUCCESS)
    return agentWarn("could not close dir object");

  return out.str();
}


std::string
CommandEventHandler::mkdir(std::string path)
{
  if (PR_MkDir(path.c_str(), 755) != PR_SUCCESS)
    return std::string("Could not create directory " + path);
  return std::string(path + " successfuly created");
}


// TODO push
// TODO pull
// TODO quit
// TODO rebt

std::string
CommandEventHandler::quit()
{
  mBufSocket.close();
  return "";
}


std::string
CommandEventHandler::rm(std::string path)
{
  if (PR_Delete(path.c_str()) == PR_SUCCESS)
    return std::string("");
  return std::string("error: could not delete " + path);
}


std::string
CommandEventHandler::rmdr(std::string path)
{
  std::ostringstream out;
  do_rmdr(path, out);
  return out.str();
}


void
CommandEventHandler::do_rmdr(std::string path, std::ostringstream &out)
{
  std::string ret;
  const char *p = path.c_str();

  // if it's a file, nothing special to do
  if (isDir(path).compare("") != 0)
  {
    rm(path);
    return;
  }

  // recurse for dir contents
  PRDir *dir = PR_OpenDir(p);
  PRDirEntry *entry = PR_ReadDir(dir, PR_SKIP_BOTH);

  while (entry)
  {
    ret = rmdr(joinPaths(path, std::string(entry->name)));
    if (ret.compare("") != 0)
      out << ret << ENDL;
    entry = PR_ReadDir(dir, PR_SKIP_BOTH);
  }
  if (PR_CloseDir(dir) != PR_SUCCESS)
  {
     out << "error: could not close dir object\r\n";
     // maybe return;
  }
  if (PR_RmDir(p) != PR_SUCCESS)
    out << std::string("error: could not remove " + path) << ENDL;
}


std::string
CommandEventHandler::testroot()
{
  return std::string("/data/local");
}
