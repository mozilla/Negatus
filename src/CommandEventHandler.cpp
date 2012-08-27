/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "CommandEventHandler.h"
#include "Hash.h"
#include "Logging.h"
#include "PullFileEventHandler.h"
#include "PushFileEventHandler.h"
#include "Strings.h"
#include "Subprocess.h"

#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <prdtoa.h>
#include <prproces.h>
#include <prtypes.h>

CommandEventHandler::CommandLine::CommandLine(std::string line)
  : cmd("")
{
  char linec[line.size()+1];
  strcpy(linec, line.c_str());
  char* cmdc = strtok(linec, " \t");
  if (!cmdc)
    return;
  cmd = cmdc;
  char* argc;
  while ((argc = strtok(NULL, " \t")))
    args.push_back(trim(argc));
}


CommandEventHandler::CommandEventHandler(PRFileDesc* socket)
  : mBufSocket(socket), mDataEventHandler(NULL), mPrompt("$>")
{
  registerWithReactor();
  sendPrompt();
}


void
CommandEventHandler::close()
{
  mBufSocket.close();
  EventHandler::close();
}


void
CommandEventHandler::getPollDescs(std::vector<PRPollDesc>& descs)
{
  if (!mBufSocket.closed())
  {
    if (mDataEventHandler)
      mDataEventHandler->getPollDescs(descs);
    else
    {
      PRPollDesc desc;
      desc.fd = mBufSocket.fd();
      desc.in_flags = PR_POLL_READ;
      descs.push_back(desc);
    }
  }
}


void
CommandEventHandler::checkDataEventHandler(PRPollDesc desc)
{
  if (!closed() && mDataEventHandler)
  {
    if (!mDataEventHandler->closed())
      mDataEventHandler->handleEvent(desc);
    if (mDataEventHandler->closed())
    {
      delete mDataEventHandler;
      mDataEventHandler = NULL;
      sendPrompt();
    }
    else
      return;
  }
}


void
CommandEventHandler::handleEvent(PRPollDesc desc)
{
  checkDataEventHandler(desc);

  if (desc.fd != mBufSocket.fd())
    return;
  if (!(desc.out_flags & PR_POLL_READ))
    return;

  while (!closed() && !mDataEventHandler)
  {
    std::stringstream buf;
    PRUint32 numRead = mBufSocket.readLine(buf);
    if (!numRead)
      break;
    std::string line(trim(buf.str()));
    handleLine(line);
    if (!closed() && !mDataEventHandler)
      sendPrompt();
  }

  checkDataEventHandler(desc);
  if (mBufSocket.closed())
  {
    if (mDataEventHandler)
    {
      delete mDataEventHandler;
      mDataEventHandler = NULL;
    }
    close();
  }
}


void
CommandEventHandler::handleLine(std::string line)
{
  CommandLine cl(line);
  if (cl.cmd.empty())
    return;
  std::string result(agentWarn("unknown command"));
  if (cl.cmd.compare("cat") == 0)
    result = cat(cl.args);
  else if (cl.cmd.compare("cd") == 0)
    result = cd(cl.args);
  else if (cl.cmd.compare("cwd") == 0)
    result = cwd(cl.args);
  else if (cl.cmd.compare("clok") == 0)
    result = clok(cl.args);
  else if (cl.cmd.compare("dirw") == 0)
    result = dirw(cl.args);
  else if (cl.cmd.compare("exec") == 0)
    result = exec(cl.args);
  else if (cl.cmd.compare("hash") == 0)
    result = hash(cl.args);
  else if (cl.cmd.compare("id") == 0)
    result = id(cl.args);
  else if (cl.cmd.compare("os") == 0)
    result = os(cl.args);
  else if (cl.cmd.compare("systime") == 0)
    result = systime(cl.args);
  else if (cl.cmd.compare("uptime") == 0)
    result = uptime(cl.args);
  else if (cl.cmd.compare("screen") == 0)
    result = screen(cl.args);
  else if (cl.cmd.compare("memory") == 0)
    result = memory(cl.args);
  else if (cl.cmd.compare("power") == 0)
    result = power(cl.args);
  else if (cl.cmd.compare("ps") == 0)
    result = ps(cl.args);
  else if (cl.cmd.compare("pull") == 0)
    result = pull(cl.args);
  else if (cl.cmd.compare("push") == 0)
    result = push(cl.args);
  else if (cl.cmd.compare("isdir") == 0)
    result = isDir(cl.args);
  else if (cl.cmd.compare("ls") == 0)
    result = ls(cl.args);
  else if (cl.cmd.compare("mkdr") == 0)
    result = mkdr(cl.args);
  else if (cl.cmd.compare("quit") == 0)
    result = quit(cl.args);
  else if (cl.cmd.compare("rm") == 0)
    result = rm(cl.args);
  else if (cl.cmd.compare("rmdr") == 0)
    result = rmdr(cl.args);
  else if (cl.cmd.compare("testroot") == 0)
    result = testroot(cl.args);
  if (!result.empty())
  {
    mBufSocket.write(result);
    mBufSocket.write("\0", 1); // devmgrSUT.py expets NULL terminated str
  }
}


void
CommandEventHandler::sendPrompt()
{
  mBufSocket.write(mPrompt.c_str(), mPrompt.size());
  mBufSocket.write("\0", 1); // devmgrSUT.py expects NULL terminated str
}


std::string
CommandEventHandler::readTextFile(std::string path)
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
  return output.str();
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
CommandEventHandler::cat(std::vector<std::string>& args)
{
  std::string path(args.size() ? args[0] : "");
  if (path.compare("") == 0)
    return agentWarn("cat needs an argument");
  if (isDir(path).compare("TRUE") == 0)
    return agentWarn("cannot cat a dir");

  return readTextFile(path);
}


std::string
CommandEventHandler::cd(std::vector<std::string>& args)
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
      return cwd(args);
  }
  return agentWarn("no permissions");
}


std::string
CommandEventHandler::cwd(std::vector<std::string>& args)
{
  char buffer[BUFSIZE];
  getcwd(buffer, BUFSIZE);
  return std::string(buffer) + ENDL;
}


std::string
CommandEventHandler::clok(std::vector<std::string>& args)
{
  PRUint64 now = PR_Now() / PR_USEC_PER_SEC;
  return itoa(now) + ENDL;
}


std::string
CommandEventHandler::dirw(std::vector<std::string>& args)
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
    return std::string(path + " is writable" + ENDL);
  return std::string(path + " is not writable" + ENDL);
}


std::string
CommandEventHandler::exec(std::vector<std::string>& args)
{
  if (args.size() < 1)
    return agentWarn("command not specified");

  std::vector<std::string>::iterator argi = args.begin();

  // handle first part separately, check if we have env vars
  bool envs = args[0].find('=') != std::string::npos;

  std::vector<std::string> env_names, env_values;
  // if we have envs we have to handle them separately
  if (envs)
  {
    char envVarStr[(*argi).size() + 1];
    (*argi).copy(envVarStr, (*argi).size());
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
    return std::string("success") + ENDL;
  return agentWarn("error");
}


std::string
CommandEventHandler::hash(std::vector<std::string>& args)
{
  if (args.size() < 1)
    return agentWarnInvalidNumArgs(1);
  std::string path = args[0];

  if (PR_Access(path.c_str(), PR_ACCESS_READ_OK) != PR_SUCCESS)
    return std::string(agentWarn("cannot open file for reading"));

  return fileHash(path) + std::string(ENDL);
}


std::string
CommandEventHandler::id(std::vector<std::string>& args)
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
CommandEventHandler::os(std::vector<std::string>& args)
{
  // not really supported yet. Best we could do is
  // cat /system/sources.xml | grep gaia and another grep for m-c
  return std::string("B2G");
}


std::string
CommandEventHandler::systime(std::vector<std::string>& args)
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
CommandEventHandler::uptime(std::vector<std::string>& args)
{
  return getCmdOutput("uptime");
}


// TODO uptimimilis
// TODO rotation

// need to figure a better way
std::string
CommandEventHandler::screen(std::vector<std::string>& args)
{
  return readTextFile("/sys/devices/virtual/graphics/fb0/modes");
}


std::string
CommandEventHandler::memory(std::vector<std::string>& args)
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

  sprintf(buffer, "Total: %d; Available: %d.\n", total * 1000, available * 1000);
  return std::string(buffer);
}


std::string
CommandEventHandler::power(std::vector<std::string>& args)
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
CommandEventHandler::ps(std::vector<std::string>& args)
{
  FILE *p = checkPopen("ps | tr -s \" \" | cut -d' ' -f1,2,9 | tail +2", "r");
  std::ostringstream ret;
  char buffer[BUFSIZE];

  while (fgets(buffer, BUFSIZE, p))
    ret << buffer;

  return ret.str();
}


std::string
CommandEventHandler::pull(std::vector<std::string>& args)
{
  if (args.size() < 1)
    return agentWarnInvalidNumArgs(1);
  std::string path(args[0]);
  PRUint64 start = args.size() < 2 ? 0 : PR_strtod(args[1].c_str(), NULL);
  PRUint64 size = args.size() < 3 ? 0 : PR_strtod(args[2].c_str(), NULL);
  mDataEventHandler = new PullFileEventHandler(mBufSocket, path, start, size);
  return "";
}


std::string
CommandEventHandler::push(std::vector<std::string>& args)
{
  if (args.size() < 2)
    return agentWarnInvalidNumArgs(2);
  std::string path(args[0]);
  PRUint64 size = PR_strtod(args[1].c_str(), NULL);
  mDataEventHandler = new PushFileEventHandler(mBufSocket, path, size);
  return "";
}


std::string
CommandEventHandler::isDir(std::vector<std::string>& args)
{
  if (args.size() < 1)
    return agentWarnInvalidNumArgs(1);
  return isDir(args[0]);
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
      return std::string("TRUE");
    return std::string("FALSE");
  }
  return agentWarn("invalid path");
}

std::string
CommandEventHandler::ls(std::vector<std::string>& args)
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

  return out.str();
}


std::string
CommandEventHandler::mkdir(std::vector<std::string>& args)
{
  if (args.size() < 1)
    return agentWarnInvalidNumArgs(1);
  std::string path = args[0];
  if (PR_MkDir(path.c_str(), 755) != PR_SUCCESS)
    return std::string("Could not create directory " + path) + ENDL;
  return std::string(path + " successfuly created") + ENDL;
}


// TODO push
// TODO pull
// TODO rebt

std::string
CommandEventHandler::quit(std::vector<std::string>& args)
{
  close();
  return "";
}


std::string
CommandEventHandler::rm(std::vector<std::string>& args)
{
  if (args.size() < 1)
    return agentWarnInvalidNumArgs(1);
  return rm(args[0]);
}


std::string
CommandEventHandler::rm(std::string path)
{
  if (PR_Delete(path.c_str()) == PR_SUCCESS)
    return std::string("");
  return std::string("error: could not delete " + path);
}


std::string
CommandEventHandler::rmdr(std::vector<std::string>& args)
{
  if (args.size() < 1)
    return agentWarnInvalidNumArgs(1);
  std::string path = args[0];
  std::ostringstream out;
  do_rmdr(path, out);
  return out.str();
}


void
CommandEventHandler::do_rmdr(std::string path, std::ostringstream &out)
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
CommandEventHandler::testroot(std::vector<std::string>& args)
{
  return std::string("/data/local") + ENDL;
}
