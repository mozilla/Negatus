/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "CommandEventHandler.h"
#include "Hash.h"
#include "Logger.h"
#include "Logging.h"
#include "PullFileEventHandler.h"
#include "PushFileEventHandler.h"
#include "Reactor.h"
#include "Shell.h"
#include "Strings.h"
#include "Subprocess.h"
#include "SubprocessEventHandler.h"
#include "Version.h"

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

#define PERMS755 PR_IRWXU | PR_IRGRP | PR_IXGRP | PR_IROTH | PR_IXOTH


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
    else if (!mBufSocket.recvClosed())
    {
      PRPollDesc desc;
      desc.fd = mBufSocket.fd();
      desc.in_flags = PR_POLL_READ;
      descs.push_back(desc);
    }
  }
}


bool
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
      return false;
    }
    return true;
  }
  return false;
}


void
CommandEventHandler::handleTimeout()
{
  if (mDataEventHandler && !mDataEventHandler->closed())
    mDataEventHandler->handleTimeout();
  if (mDataEventHandler->closed())
  {
    delete mDataEventHandler;
    mDataEventHandler = NULL;
    sendPrompt();
  }
}


void
CommandEventHandler::handleEvent(PRPollDesc desc)
{
  // Make sure we drain all the data we can from the socket.
  while (!closed())
  {
    if (checkDataEventHandler(desc))
      break;

    if (desc.fd != mBufSocket.fd())
      break;
    if (!(desc.out_flags & PR_POLL_READ))
      break;

    bool noMoreToRead = false;
    while (!closed() && !mDataEventHandler)
    {
      std::stringstream buf;
      PRUint32 numRead = mBufSocket.readLine(buf);
      if (!numRead)
      {
        noMoreToRead = true;
        break;
      }
      std::string line(trim(buf.str()));
      handleLine(line);
    }

    if (noMoreToRead)
      break;
  }

  if (mBufSocket.recvClosed())
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
  Logger::instance()->log("Recvd line: " + line);
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
  else if (cl.cmd.compare("execsu") == 0)
    result = exec(cl.args);
  else if (cl.cmd.compare("hash") == 0)
    result = hash(cl.args);
  else if (cl.cmd.compare("info") == 0)
    result = info(cl.args);
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
  else if (cl.cmd.compare("rebt") == 0)
    result = rebt(cl.args);
  else if (cl.cmd.compare("rm") == 0)
    result = rm(cl.args);
  else if (cl.cmd.compare("rmdr") == 0)
    result = rmdr(cl.args);
  else if (cl.cmd.compare("settime") == 0)
    result = settime(cl.args);
  else if (cl.cmd.compare("setutime") == 0)
    result = setutime(cl.args);
  else if (cl.cmd.compare("testroot") == 0)
    result = testroot(cl.args);
  else if (cl.cmd.compare("ver") == 0)
    result = ver(cl.args);
  if (!mBufSocket.sendClosed() && !mDataEventHandler)
  {
    // FIXME: It appears that DeviceManager will get confused if
    // the prompt isn't sent with the response for some commands.
    // I think this is a bug in DeviceManager, but it needs to be
    // further investigated.
    result += std::string(ENDL);
    result += mPrompt;
    mBufSocket.write(result.c_str(), result.size() + 1);
  }
}


void
CommandEventHandler::sendPrompt()
{
  // devmgrSUT.py expects NULL terminated str
  mBufSocket.write(mPrompt.c_str(), mPrompt.size() + 1);
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
  std::string str = output.str();
  if (str.size())
    str.erase(str.size() - 1);
  return str;
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
  if (args.size() < 1)
    return agentWarnInvalidNumArgs(1);
  std::string path(args[0]);
  mDataEventHandler = new PullFileEventHandler(mBufSocket, path, 0, 0, false);
  return "";
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
      return "";
  }
  return agentWarn("no permissions");
}


std::string
CommandEventHandler::cwd(std::vector<std::string>& args)
{
  char buffer[BUFSIZE];
  getcwd(buffer, BUFSIZE);
  return std::string(buffer);
}


std::string
CommandEventHandler::clok(std::vector<std::string>& args)
{
  PRUint64 now = PR_Now() / PR_MSEC_PER_SEC;
  return itoa(now);
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
    return std::string(path + " is writable");
  return std::string(path + " is not writable");
}


std::string
CommandEventHandler::exec(std::vector<std::string>& args)
{
  if (args.size() < 1)
    return agentWarnInvalidNumArgs(1);

  // delete double quotes from args[0], easier to parse
  if (args[0][0] == '"')
  {
    args[0].erase(0, 1); // delete the beginning one
    args[0].erase(args[0].size() - 1); // delete the ending one
  }

  std::vector<std::string>::iterator argi = args.begin();

  // handle first part separately, check if we have env vars
  bool envs = args[0].find('=') != std::string::npos;

  std::vector<std::string> envNames, envValues;
  // if we have envs we have to handle them separately
  if (envs)
  {
    char envVarStr[(*argi).size() + 1];
    envVarStr[(*argi).size()] = 0;
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
      envNames.push_back(var);
      envValues.push_back(val);

      env = strtok_r(NULL, ",", &r_env);
    }
    // skip past the env part
    argi++;
  }

  // extract the prog
  std::string prog(*argi++);

  // what remains are the args
  std::ostringstream to_exec;
  to_exec << prog;
  for (; argi != args.end(); ++argi)
    to_exec << " " << *argi;

  mDataEventHandler = new SubprocessEventHandler(mBufSocket, *this,
                                                 to_exec.str(), envNames,
                                                 envValues);
  if (mDataEventHandler->closed())
  {
    delete mDataEventHandler;
    mDataEventHandler = NULL;
    return agentWarn("failed to launch process");
  }
  return "";
}


std::string
CommandEventHandler::hash(std::vector<std::string>& args)
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
CommandEventHandler::info(std::vector<std::string>& args)
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


// this is not cross platform
std::string
CommandEventHandler::uptimemillis()
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
CommandEventHandler::ps(std::vector<std::string>& args)
{
  // might need to change command on linux since the output is different on B2G
  return getCmdOutput("ps | tr -s \" \" | cut -d' ' -f1,2,9 | tail +2");
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
  if (success == PR_SUCCESS && info.type == PR_FILE_DIRECTORY)
    return std::string("TRUE");
  return std::string("FALSE");
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

  std::string output = out.str();
  if (output.size())
    output.erase(output.size() - 1); // erase the extra newline
  return output;
}


std::string
CommandEventHandler::mkdr(std::vector<std::string>& args)
{
  if (args.size() < 1)
    return agentWarnInvalidNumArgs(1);
  std::string path = args[0];
  if (PR_MkDir(path.c_str(), PERMS755) != PR_SUCCESS)
    return agentWarn("Could not create directory " + path);
  return std::string(path + " successfuly created");
}


std::string
CommandEventHandler::quit(std::vector<std::string>& args)
{
  close();
  return "";
}


std::string
CommandEventHandler::rebt(std::vector<std::string>& args)
{
  if (args.size() != 0 && args.size() != 2)
    return agentWarn("Invalid args. Valid options: no args or IP PORT");

  // store callback IP and PORT in REBOOT_FILE, if specified
  if (args.size() == 2)
  {
    FILE *f = fopen(REBOOT_FILE, "w");
    if (!f)
      return agentWarn("Could not write to reboot callback file");
    fprintf(f, "%s %s\n", args[0].c_str(), args[1].c_str());
    fclose(f);
  }

  close();
  Reactor::instance()->stop();
  Logger::instance()->log("Rebooting.");
  popen("reboot", "r");
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
    return std::string("removing file" + path);
  return agentWarn("error: could not delete " + path);
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
CommandEventHandler::settime(std::vector<std::string>& args)
{
  if (args.size() < 2)
    return agentWarnInvalidNumArgs(2);

  // It would be nice to use the NSPR date functions here, but sadly NSPR
  // provides no way to actually *set* the date, so we use standard POSIX C
  // calls.
  std::string dates(args[0]);
  std::string times(args[1]);
  std::vector<PRUint64> ints;
  std::ostringstream out;
  struct tm datetime;
  datetime.tm_wday = 0;
  datetime.tm_yday = 0;
  datetime.tm_isdst = -1;

  char datec[dates.size()+1];
  strcpy(datec, dates.c_str());
  char* tok = strtok(datec, "/");
  while (tok)
  {
    ints.push_back(PR_strtod(tok, NULL));
    tok = strtok(NULL, "/");
  }

  if (ints.size() < 3)
    return agentWarn("Invalid argument(s)");

  datetime.tm_year = ints[0] - 1900;
  datetime.tm_mon = ints[1] - 1;
  datetime.tm_mday = ints[2];

  ints.clear();
  char timec[times.size()+1];
  strcpy(timec, times.c_str());
  tok = strtok(timec, ":");
  while (tok)
  {
    ints.push_back(PR_strtod(tok, NULL));
    tok = strtok(NULL, ":");
  }

  if (ints.size() < 3)
    return agentWarn("Invalid argument(s)");

  datetime.tm_hour = ints[0];
  datetime.tm_min = ints[1];
  datetime.tm_sec = ints[2];

  time_t tsecs = mktime(&datetime);

  struct timeval tv;
  struct timezone tz;
  gettimeofday(NULL, &tz);
  tv.tv_sec = tsecs;
  tv.tv_usec = 0;
  settimeofday(&tv, &tz);

  time_t t = time(NULL);
  struct tm* newtmp = localtime(&t);
  out << newtmp->tm_year + 1900 << "/" << newtmp->tm_mon + 1 << "/"
      << newtmp->tm_mday << " " << newtmp->tm_hour << ":" << newtmp->tm_min
      << ":" << newtmp->tm_sec;
  return out.str();
}


std::string
CommandEventHandler::setutime(std::vector<std::string>& args)
{
  if (args.size() < 1)
    return agentWarnInvalidNumArgs(1);
  std::string times(args[0]);
  PRUint64 t = PR_strtod(times.c_str(), NULL);

  struct timeval tv;
  struct timezone tz;
  tv.tv_sec = t / 1000;
  tv.tv_usec = (t - tv.tv_sec * 1000) * 1000;
  tz.tz_minuteswest = 0;
  tz.tz_dsttime = 0;

  settimeofday(&tv, &tz);

  return clok(args);  // clok ignores args
}


std::string
CommandEventHandler::testroot(std::vector<std::string>& args)
{
  return std::string(TESTROOT);
}


std::string
CommandEventHandler::ver(std::vector<std::string>& args)
{
  return version();
}


EventHandler*
CommandEventHandlerFactory::createEventHandler(PRFileDesc* socket)
{
  return new CommandEventHandler(socket);
}
