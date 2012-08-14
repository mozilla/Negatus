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


class SearchableBuffer: public std::stringbuf
{
public:
  std::streamsize find(char c);
};


class BufferedSocket
{
public:
  BufferedSocket(PRFileDesc* socket);
  virtual ~BufferedSocket();

  void close();
  bool closed() { return !mSocket; }

  PRFileDesc* fd() { return mSocket; }

  /* Attempts to read 'size' bytes into 'buf'. If the whole
     amount cannot be read, it is buffered and nothing is
     written to 'buf'. Returns the number of bytes written to
     'buf'. 'buf' should be empty; anything in it may be
     overwritten. */
  PRUint32 read(SearchableBuffer& buf, PRUint32 size);
  PRUint32 readUntil(SearchableBuffer& buf, char c);
  PRUint32 readUntilNewline(SearchableBuffer& buf)
    { return readUntil(buf, '\n'); }

  void write(const char* buf, PRUint32 size);
  void flush();

  bool writeBufferEmpty();

private:
  PRFileDesc* mSocket;
  SearchableBuffer mReadBuffer;
  //char* mReadBuffer;
  PRUint32 mReadBufferSize;
  SearchableBuffer mWriteBuffer;
  //char* mWriteBuffer;
  //PRUint32 mWriteBufferSize;
  char* tmpBuf;

  PRUint32 readIntoBuffer(PRUint32 size);
  PRUint32 copyBuf(SearchableBuffer& dest, SearchableBuffer& src,
                   PRUint32& bufferSize, PRUint32 size);
};

#endif
