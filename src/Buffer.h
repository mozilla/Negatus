/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef negatus_buffer_h
#define negatus_buffer_h

#include <sstream>
#include <vector>

#define DEFAULT_BLOCK_SIZE 1024

class BlockBuffer
{
public:
  BlockBuffer(unsigned int defaultBlockSize=DEFAULT_BLOCK_SIZE);

  int get(char* out, unsigned int size);
  int put(const char* in, unsigned int size);
  /* Only writes to 'out' if a whole line is found.
     If 'size' is less than the length of the line
     plus one (for the null char), returns -1. */
  int getline(char* out, unsigned int size);
  int getline(std::stringstream& out);
  int find(char c);

  char* allocBlock(unsigned int size);
  void advancePutPtr(unsigned int amount);

  unsigned int avail();

private:
  class Block
  {
  public:
    Block(unsigned int size=DEFAULT_BLOCK_SIZE);
    ~Block();

    int get(char* out, unsigned int start, unsigned int size);
    int put(const char* in, unsigned int start, unsigned int size);
    int find(char c, unsigned int start, unsigned int end);

    unsigned int size() { return mSize; }

    /* Resizes the buffer. Does not actuallyd deallocate any memory;
       rather, it just makes the block look smaller. This is needed if
       a new block of a defined size needs to be added to the buffer. */
    void shrink(unsigned int newSize);

    char* data(unsigned int start=0) { return mData + start; }

  private:
    unsigned int mSize;
    char* mData;
  };

  struct BlockPtr
  {
    BlockPtr() : block(0), elem(0) {}
    unsigned int block;
    unsigned int elem;

    bool operator!=(BlockPtr& other)
    { return block != other.block || elem != other.elem; }
  };

  void newBlock(unsigned int size);

  unsigned int mDefaultBlockSize;
  std::vector<Block*> mBlocks;
  BlockPtr mGetPtr;
  BlockPtr mPutPtr;

  void updateBufferStart();
  void updateBufferEnd();
};

#endif
