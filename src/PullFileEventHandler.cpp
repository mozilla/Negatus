/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "PullFileEventHandler.h"
#include <sstream>
#include <prerror.h>
#include "BufferedSocket.h"
#include "Logging.h"
#include "Strings.h"

PullFileEventHandler::PullFileEventHandler(BufferedSocket& bufSocket,
                                           std::string path,
                                           PRUint64 start,
                                           PRUint64 size,
                                           bool includeHeader)
  : mBufSocket(bufSocket), mPath(path), mFile(NULL), mBytesRead(0), mSize(size),
    mTmpBuf(new char[BLOCK_SIZE])
{
  if (PR_Access(path.c_str(), PR_ACCESS_EXISTS) != PR_SUCCESS)
  {
    pullError("file does not exist");
    close();
    return;
  }

  if (PR_Access(path.c_str(), PR_ACCESS_READ_OK) != PR_SUCCESS)
  {
    pullError("insufficient permissions to read file");
    close();
    return;
  }

  mFile = PR_Open(path.c_str(), PR_RDONLY, 0);
  if (!mFile)
  {
    pullError("could not open file");
    close();
    return;
  }

  PRFileInfo fileInfo;
  if (PR_GetOpenFileInfo(mFile, &fileInfo) != PR_SUCCESS)
  {
    pullError("failed to read file info");
    close();
    return;
  }

  if (fileInfo.size <= start)
  {
    // we don't error out; we just don't return any data
    start = 0;
    mSize = 0;
  }
  else if (mSize == 0)
    mSize = fileInfo.size - start;

  if (start > 0)
  {
    PRInt64 seekBytes = PR_Seek64(mFile, start, PR_SEEK_SET);
    if (seekBytes == -1)
    {
      pullError("failed to seek");
      close();
      return;
    }
  }
  

  if (!includeHeader)
    return;
  std::stringstream out;
  out << path << "," << mSize << ENDL;  
  mBufSocket.write(out.str());
  if (mSize == 0)
    close();
}


PullFileEventHandler::~PullFileEventHandler()
{
  delete[] mTmpBuf;
}


void
PullFileEventHandler::pullError(std::string err)
{
  std::ostringstream out, errout;
  errout << mPath << ",-1";
  out << agentWarn(errout.str()) << ENDL << err << ENDL;
  mBufSocket.write(out.str());
}


void
PullFileEventHandler::close()
{
  EventHandler::close();
  if (mFile)
  {
    PR_Close(mFile);
    mFile = NULL;
  }
}


void
PullFileEventHandler::getPollDescs(std::vector<PRPollDesc>& descs)
{
  if (!mBufSocket.closed())
  {
    PRPollDesc desc;
    desc.fd = mBufSocket.fd();
    desc.in_flags = PR_POLL_WRITE;
    descs.push_back(desc);
  }
}


void
PullFileEventHandler::handleEvent(PRPollDesc desc)
{
  if (desc.fd != mBufSocket.fd())
    return;
  if (!(desc.out_flags & PR_POLL_WRITE))
    return;

  PRUint32 blockSize = (mSize - mBytesRead) < BLOCK_SIZE ? mSize - mBytesRead
    : BLOCK_SIZE;

  PRInt32 bytes = PR_Read(mFile, mTmpBuf, blockSize);
  if (bytes > 0)
  {
    mBytesRead += bytes;
    mBufSocket.write(mTmpBuf, bytes);
  }
  else if (bytes == 0)
    close();
  else if (bytes < 0)
  {
    // FIXME: log error
    close();
  }
  if (mSize == mBytesRead)
    close();
}
