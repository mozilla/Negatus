/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "BufferedSocket.h"
#include <iostream>
#include <prerror.h>
#include <string.h>


std::streamsize
SearchableBuffer::find(char c)
{
  std::streamsize count = 0;
  char* p = gptr();
  for (char* p = gptr(); p != egptr(); p++)
  {
    if (*p == c)
      return count;
    count++;
  }
  return -1;
}


BufferedSocket::BufferedSocket(PRFileDesc* socket)
  : mSocket(socket), mReadBufferSize(0), tmpBuf(new char[1024])
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
BufferedSocket::read(SearchableBuffer& buf, PRUint32 size)
{
  PRUint32 numRead = 0;

  while (readIntoBuffer(1024))
    continue;
    
  if (mReadBufferSize)
    numRead = copyBuf(buf, mReadBuffer, mReadBufferSize, size);

  return numRead;
}


PRUint32
BufferedSocket::readUntil(SearchableBuffer& buf, char c)
{
  while (readIntoBuffer(256) != 0)
    continue;

  PRUint32 numRead = 0;
  std::streamsize loc = mReadBuffer.find(c);
  if (c != -1)
    numRead = copyBuf(buf, mReadBuffer, mReadBufferSize, c);
  return numRead;
}


void
BufferedSocket::write(const char* buf, PRUint32 size)
{
  PRInt32 numSent = PR_Send(mSocket, buf, size, 0, PR_INTERVAL_NO_WAIT);
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
    std::streamsize bytesPut = mReadBuffer.sputn(tmpBuf, status);
    mReadBufferSize += status;
    numRead = status;
  }

  return numRead;
}


PRUint32
BufferedSocket::copyBuf(SearchableBuffer& dest, SearchableBuffer& src,
                        PRUint32& bufferSize, PRUint32 size)
{
  PRUint32 sizeWritten = 0;
  while (bufferSize && sizeWritten < size)
  {
    PRUint32 toWrite = (size - sizeWritten) % 1024;
    if (toWrite > bufferSize)
      toWrite = bufferSize;
    src.sgetn(tmpBuf, toWrite);
    dest.sputn(tmpBuf, toWrite);
    sizeWritten += toWrite;
    bufferSize -= toWrite;
  }
  return sizeWritten;
}
