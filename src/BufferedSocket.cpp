/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "BufferedSocket.h"
#include <prerror.h>
#include <string.h>


BufferedSocket::BufferedSocket(PRFileDesc* socket)
  : mSocket(socket), mTmpBuf(new char[1024]), mRecvClosed(false),
    mSendClosed(false)
{
}


BufferedSocket::~BufferedSocket()
{
  delete[] mTmpBuf;
}


void
BufferedSocket::close()
{
  if (!closed())
  {
    // FIXME: log errors
    PR_Shutdown(mSocket, PR_SHUTDOWN_BOTH);
    mRecvClosed = true;
    mSendClosed = true;
    PR_Close(mSocket);
    mSocket = NULL;
  }
}


PRUint32
BufferedSocket::read(char* buf, PRUint32 size)
{
  while (readIntoBuffer(1024))
    continue;

  PRUint32 avail = mReadBuffer.avail() > size ? size : mReadBuffer.avail();
  return mReadBuffer.get(buf, avail);
}


PRUint32
BufferedSocket::readLine(std::stringstream& buf)
{
  while (readIntoBuffer(256) != 0)
    continue;

  return mReadBuffer.getline(buf);
}


void
BufferedSocket::write(const char* buf, PRUint32 size)
{
  if (!size || mSendClosed)
    return;

  PRInt32 status = PR_Send(mSocket, buf, size, 0, PR_INTERVAL_NO_WAIT);
  if (status == -1)
  {
    PRErrorCode err = PR_GetError();
    if (err != PR_WOULD_BLOCK_ERROR)
      closeSend();
  }
}


void
BufferedSocket::write(std::string line)
{
  write(line.c_str(), line.size());
}


PRUint32
BufferedSocket::readIntoBuffer(PRUint32 size)
{
  if (!size || mRecvClosed)
    return 0;

  PRInt32 numRead = 0;

  PRInt32 status = PR_Recv(mSocket, mTmpBuf, size, 0, PR_INTERVAL_NO_WAIT);
  if (status == 0)
    closeRecv();
  else if (status == -1)
  {
    PRErrorCode err = PR_GetError();
    if (err != PR_WOULD_BLOCK_ERROR)
      closeRecv();
  }
  else if (status > 0)
  {
    mTmpBuf[status] = 0;
    mReadBuffer.put(mTmpBuf, status);
    numRead = status;
  }

  return numRead;
}


void
BufferedSocket::closeRecv()
{
  mRecvClosed = true;
  if (mSendClosed)
    close();
}


void
BufferedSocket::closeSend()
{
  mSendClosed = true;
  if (mRecvClosed)
    close();
}
