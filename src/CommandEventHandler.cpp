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
#include "Shell.h"
#include "Strings.h"
#include "Subprocess.h"
#include "SubprocessEventHandler.h"

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
  else if (cl.cmd.compare("rm") == 0)
    result = rm(cl.args);
  else if (cl.cmd.compare("rmdr") == 0)
    result = rmdr(cl.args);
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
CommandEventHandler::cat(std::vector<std::string>& args)
{
  if (args.size() < 1)
    return agentWarnInvalidNumArgs(1);
  std::string path(args[0]);
  mDataEventHandler = new PullFileEventHandler(mBufSocket, path, 0, 0, false);
  return "";
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
CommandEventHandler::quit(std::vector<std::string>& args)
{
  close();
  return "";
}
