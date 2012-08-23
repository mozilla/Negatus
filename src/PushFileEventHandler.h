/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_push_file_event_handler_h
#define negatus_push_file_event_handler_h

#include <string>
#include <prio.h>
#include "EventHandler.h"

#define BLOCK_SIZE 1024

class BufferedSocket;

class PushFileEventHandler: public EventHandler
{
public:
  PushFileEventHandler(BufferedSocket& bufSocket, std::string path,
                       PRUint64 size);
  virtual ~PushFileEventHandler();

  virtual void close();
  virtual void getPollDescs(std::vector<PRPollDesc>& descs);
  virtual void handleEvent(PRPollDesc desc);
  virtual std::string name() { return "PushFileEventHandler"; }

private:
  BufferedSocket& mBufSocket;
  std::string mPath;
  PRFileDesc* mFile;
  PRUint64 mBytesWritten;
  PRUint64 mSize;
  char* mTmpBuf;
};

#endif
