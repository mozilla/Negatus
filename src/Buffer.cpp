/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "Buffer.h"
#include <assert.h>
#include <string.h>

BlockBuffer::Block::Block(unsigned int size)
  : mSize(size), mData(new char[size])
{
}


BlockBuffer::Block::~Block()
{
  delete[] mData;
}


int
BlockBuffer::Block::get(char* out, unsigned int start, unsigned int size)
{
  assert(start + size <= mSize);
  memcpy(out, &mData[start], size);
  return size;
}


int
BlockBuffer::Block::put(const char* in, unsigned int start, unsigned int size)
{
  assert(start + size <= mSize);
  memcpy(mData + start, in, size);
  return size;
}


int
BlockBuffer::Block::find(char c, unsigned int start, unsigned int end)
{
  for (int i = start; i < end; i++)
  {
    if (mData[i] == c)
      return i;
  }
  return -1;
}


void
BlockBuffer::Block::shrink(unsigned int newSize)
{
  assert(newSize <= mSize);
  mSize = newSize;
}


BlockBuffer::BlockBuffer(unsigned int defaultBlockSize)
  : mDefaultBlockSize(defaultBlockSize)
{
  newBlock(mDefaultBlockSize);
}

 
int
BlockBuffer::get(char* out, unsigned int size)
{
  unsigned int totalGotten = 0;
  Block* blockp;
  while (totalGotten < size && mGetPtr != mPutPtr)
  {
    blockp = mBlocks[mGetPtr.block];
    unsigned int toGet = size - totalGotten;
    if (toGet > blockp->size() - mGetPtr.elem)
      toGet = blockp->size() - mGetPtr.elem;
    unsigned int gotten = blockp->get(&out[totalGotten], mGetPtr.elem, toGet);
    mGetPtr.elem += gotten;
    totalGotten += gotten;
    updateBufferStart();
  }
  return totalGotten;
}


int
BlockBuffer::put(const char* in, unsigned int size)
{
  unsigned int totalWritten = 0;
  Block* blockp;
  while (totalWritten < size)
  {
    blockp = mBlocks[mPutPtr.block];
    unsigned int toWrite = blockp->size() - mPutPtr.elem;
    if (toWrite > size - totalWritten)
      toWrite = size - totalWritten;
    unsigned int written = blockp->put(&in[totalWritten], mPutPtr.elem, toWrite);
    totalWritten += written;
    mPutPtr.elem += written;
    updateBufferEnd();
  }
  return totalWritten;
}


void
BlockBuffer::updateBufferEnd()
{
  if (mPutPtr.elem == mBlocks[mPutPtr.block]->size())
    newBlock(mDefaultBlockSize);
}


void
BlockBuffer::updateBufferStart()
{
  if (mGetPtr.elem == mBlocks[mGetPtr.block]->size())
  {
    mGetPtr.block++;
    mGetPtr.elem = 0;
  }

  while (mGetPtr.block > 0)
  {
    Block* b = mBlocks[0];
    mBlocks.erase(mBlocks.begin());
    delete b;
    mGetPtr.block--;
    mPutPtr.block--;
  }
}


int
BlockBuffer::find(char c)
{
  unsigned int block = mGetPtr.block;
  unsigned int len = 0;
  while (block < mBlocks.size())
  {
    unsigned int start = block == mGetPtr.block ? mGetPtr.elem : 0;
    unsigned int end = block == mPutPtr.block ? mPutPtr.elem : mBlocks[block]->size();
    unsigned int nl = mBlocks[block]->find('\n', start, end);
    if (nl == -1)
    {
      len += end - start;
      block++;
      continue;
    }
    len += nl - start + 1;  // include newline
    break;
  }
  if (block >= mBlocks.size())
    return -1;
  return len;
}


int
BlockBuffer::getline(std::stringstream& out)
{
  // FIXME: This does a double copy. We should be able to make a
  // BlockBuffer::Block::get(std::stringstream& out, unsigned int size)
  // function to get around this.
  int len = find('\n');
  if (len == -1)
    return 0;
  char* outp = new char[len+1];
  outp[len] = 0;
  get(outp, len);
  out.write(outp, len);
  return len;
}


int
BlockBuffer::getline(char* out, unsigned int size)
{
  int len = find('\n');
  if (len == -1)
    return 0;
  if (size < len + 1)
    return -1;
  out[len] = 0;
  return get(out, len);
}


void
BlockBuffer::newBlock(unsigned int size)
{
  Block* b = new Block(size);
  mBlocks.push_back(b);
  mPutPtr.block = mBlocks.size() - 1;
  mPutPtr.elem = 0;
}


char*
BlockBuffer::allocBlock(unsigned int size)
{
  mBlocks[mPutPtr.block]->shrink(mPutPtr.elem);
  newBlock(size);
  return mBlocks[mPutPtr.block]->data();
}


void
BlockBuffer::advancePutPtr(unsigned int amount)
{
  unsigned int totalAdvanced = 0;
  Block* blockp;
  while (totalAdvanced < amount)
  {
    blockp = mBlocks[mPutPtr.block];
    unsigned int toAdvance = blockp->size() - mPutPtr.elem;
    if (toAdvance > amount - totalAdvanced)
      toAdvance = amount - totalAdvanced;
    totalAdvanced += toAdvance;
    mPutPtr.elem += toAdvance;
    updateBufferEnd();
  }
}


unsigned int
BlockBuffer::avail()
{
  unsigned int avail = 0;
  unsigned int block = mGetPtr.block;
  while (block <= mPutPtr.block)
  {
    unsigned int start = block == mGetPtr.block ? mGetPtr.elem : 0;
    unsigned int end = block == mPutPtr.block ? mPutPtr.elem :
      mBlocks[block]->size();
    avail += end - start;
    block++;
  }
  return avail;
}
