/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "BufferedSocket.h"
#include <prerror.h>
#include <string.h>


BufferedSocket::BufferedSocket(PRFileDesc* socket)
  : mSocket(socket), tmpBuf(new char[1024])
{
}


BufferedSocket::~BufferedSocket()
{
  delete[] tmpBuf;
}


void
BufferedSocket::close()
{
  if (mSocket && !closed())
  {
    // FIXME: log errors
    PR_Shutdown(mSocket, PR_SHUTDOWN_BOTH);
    PR_Close(mSocket);
    mSocket = NULL;
  }
}


PRUint32
BufferedSocket::read(char* buf, PRUint32 size)
{
  while (readIntoBuffer(1024))
    continue;

  return mReadBuffer.get(buf, mReadBuffer.avail());
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
  PR_Send(mSocket, buf, size, 0, PR_INTERVAL_NO_WAIT);
}


void
BufferedSocket::write(std::string line)
{
  write(line.c_str(), line.size());
}


PRUint32
BufferedSocket::readIntoBuffer(PRUint32 size)
{
  if (!size)
    return 0;

  PRInt32 numRead = 0;

  PRInt32 status = PR_Recv(mSocket, tmpBuf, size, 0, PR_INTERVAL_NO_WAIT);
  if (status == 0)
    close();
  else if (status == -1)
  {
    PRErrorCode err = PR_GetError();
    if (err != PR_WOULD_BLOCK_ERROR)
      close();
  }
  else if (status > 0)
  {
    tmpBuf[status] = 0;
    mReadBuffer.put(tmpBuf, status);
    numRead = status;
  }

  return numRead;
}
