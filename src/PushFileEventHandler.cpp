/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "PushFileEventHandler.h"
#include "BufferedSocket.h"
#include "Hash.h"
#include "Logging.h"
#include "Strings.h"

PushFileEventHandler::PushFileEventHandler(BufferedSocket& bufSocket,
                                           std::string path, PRUint64 size)
  : mBufSocket(bufSocket), mPath(path), mSize(size), mBytesWritten(0),
    mTmpBuf(new char[BLOCK_SIZE])
{
  mFile = PR_Open(path.c_str(), PR_WRONLY | PR_CREATE_FILE | PR_TRUNCATE,
    PR_IRWXU | PR_IRWXG);
  if (!mFile)
  {
    mBufSocket.write(agentWarn("could not open file for writing"));
    close();
    return;
  }
}


PushFileEventHandler::~PushFileEventHandler()
{
  delete[] mTmpBuf;
}


void
PushFileEventHandler::close()
{
  EventHandler::close();
  if (mFile)
  {
    PR_Close(mFile);
    mFile = NULL;
  }
}


void
PushFileEventHandler::getPollDescs(std::vector<PRPollDesc>& descs)
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
PushFileEventHandler::handleEvent(PRPollDesc desc)
{
  if (desc.fd != mBufSocket.fd())
    return;
  if (!(desc.out_flags & PR_POLL_READ))
    return;

  while (mBytesWritten < mSize)
  {
    PRUint32 blockSize = (mSize - mBytesWritten) < BLOCK_SIZE
      ? mSize - mBytesWritten : BLOCK_SIZE;
 
    PRUint32 bytes = mBufSocket.read(mTmpBuf, blockSize);
    if (bytes > 0)
    {
      PR_Write(mFile, mTmpBuf, bytes);
      mBytesWritten += bytes;
    }
    else
      break;
  }
  // a closed socket will be detected automatically by CommandEventHandler.
  if (mSize == mBytesWritten)
  {
    close();
    mBufSocket.write(std::string(ENDL) + fileHash(mPath) + std::string(ENDL));
  }
}
