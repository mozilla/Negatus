/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_buffered_socket_h
#define negatus_buffered_socket_h

#include <prio.h>
#include <prtypes.h>
#include <sstream>
#include <vector>
#include "Buffer.h"


class BufferedSocket
{
public:
  BufferedSocket(PRFileDesc* socket);
  virtual ~BufferedSocket();

  void close();
  bool closed() { return !mSocket; }
  bool recvClosed() { return mRecvClosed; }
  bool sendClosed() { return mSendClosed; }

  PRFileDesc* fd() { return mSocket; }

  /* Attempts to read 'size' bytes into 'buf'. Returns the
     number of bytes written to 'buf'. 'buf' should be empty;
     anything in it may be overwritten. */
  PRUint32 read(char* buf, PRUint32 size);
  PRUint32 readLine(std::stringstream& buf);

  void write(const char* buf, PRUint32 size);
  void write(std::string line);
  void flush();

  bool writeBufferEmpty();

private:
  PRFileDesc* mSocket;
  BlockBuffer mReadBuffer;
  BlockBuffer mWriteBuffer;
  char* mTmpBuf;
  bool mRecvClosed;
  bool mSendClosed;

  PRUint32 readIntoBuffer(PRUint32 size);
  void closeRecv();
  void closeSend();
};

#endif
