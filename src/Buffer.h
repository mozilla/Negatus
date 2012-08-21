/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_buffer_h
#define negatus_buffer_h

#include <prtypes.h>
#include <sstream>
#include <vector>

#define DEFAULT_BLOCK_SIZE 1024

class BlockBuffer
{
public:
  BlockBuffer(PRUint32 defaultBlockSize=DEFAULT_BLOCK_SIZE);

  int get(char* out, PRUint32 size);
  int put(const char* in, PRUint32 size);
  /* Only writes to 'out' if a whole line is found.
     If 'size' is less than the length of the line
     plus one (for the null char), returns -1. */
  int getline(char* out, PRUint32 size);
  int getline(std::stringstream& out);
  int find(char c);

  char* allocBlock(PRUint32 size);
  void advancePutPtr(PRUint32 amount);

  PRUint32 avail();

private:
  class Block
  {
  public:
    Block(PRUint32 size=DEFAULT_BLOCK_SIZE);
    ~Block();

    int get(char* out, PRUint32 start, PRUint32 size);
    int put(const char* in, PRUint32 start, PRUint32 size);
    int find(char c, PRUint32 start, PRUint32 end);

    PRUint32 size() { return mSize; }

    /* Resizes the buffer. Does not actuallyd deallocate any memory;
       rather, it just makes the block look smaller. This is needed if
       a new block of a defined size needs to be added to the buffer. */
    void shrink(PRUint32 newSize);

    char* data(PRUint32 start=0) { return mData + start; }

  private:
    PRUint32 mSize;
    char* mData;
  };

  struct BlockPtr
  {
    BlockPtr() : block(0), elem(0) {}
    PRUint32 block;
    PRUint32 elem;

    bool operator!=(BlockPtr& other)
    { return block != other.block || elem != other.elem; }
  };

  void newBlock(PRUint32 size);

  PRUint32 mDefaultBlockSize;
  std::vector<Block*> mBlocks;
  BlockPtr mGetPtr;
  BlockPtr mPutPtr;

  void updateBufferStart();
  void updateBufferEnd();
};

#endif
