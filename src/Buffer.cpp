/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "Buffer.h"
#include <assert.h>
#include <string.h>

BlockBuffer::Block::Block(PRUint32 size)
  : mSize(size), mData(new char[size])
{
}


BlockBuffer::Block::~Block()
{
  delete[] mData;
}


int
BlockBuffer::Block::get(char* out, PRUint32 start, PRUint32 size)
{
  assert(start + size <= mSize);
  memcpy(out, &mData[start], size);
  return size;
}


int
BlockBuffer::Block::put(const char* in, PRUint32 start, PRUint32 size)
{
  assert(start + size <= mSize);
  memcpy(mData + start, in, size);
  return size;
}


int
BlockBuffer::Block::find(char c, PRUint32 start, PRUint32 end)
{
  for (int i = start; i < end; i++)
  {
    if (mData[i] == c)
      return i;
  }
  return -1;
}


void
BlockBuffer::Block::shrink(PRUint32 newSize)
{
  assert(newSize <= mSize);
  mSize = newSize;
}


BlockBuffer::BlockBuffer(PRUint32 defaultBlockSize)
  : mDefaultBlockSize(defaultBlockSize)
{
  newBlock(mDefaultBlockSize);
}

 
int
BlockBuffer::get(char* out, PRUint32 size)
{
  PRUint32 totalGotten = 0;
  Block* blockp;
  while (totalGotten < size && mGetPtr != mPutPtr)
  {
    blockp = mBlocks[mGetPtr.block];
    PRUint32 toGet = size - totalGotten;
    if (toGet > blockp->size() - mGetPtr.elem)
      toGet = blockp->size() - mGetPtr.elem;
    PRUint32 gotten = blockp->get(&out[totalGotten], mGetPtr.elem, toGet);
    mGetPtr.elem += gotten;
    totalGotten += gotten;
    updateBufferStart();
  }
  return totalGotten;
}


int
BlockBuffer::put(const char* in, PRUint32 size)
{
  PRUint32 totalWritten = 0;
  Block* blockp;
  while (totalWritten < size)
  {
    blockp = mBlocks[mPutPtr.block];
    PRUint32 toWrite = blockp->size() - mPutPtr.elem;
    if (toWrite > size - totalWritten)
      toWrite = size - totalWritten;
    PRUint32 written = blockp->put(&in[totalWritten], mPutPtr.elem, toWrite);
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
  PRUint32 block = mGetPtr.block;
  PRUint32 len = 0;
  while (block < mBlocks.size())
  {
    PRUint32 start = block == mGetPtr.block ? mGetPtr.elem : 0;
    PRUint32 end = block == mPutPtr.block ? mPutPtr.elem : mBlocks[block]->size();
    PRUint32 nl = mBlocks[block]->find('\n', start, end);
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
  // BlockBuffer::Block::get(std::stringstream& out, PRUint32 size)
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
BlockBuffer::getline(char* out, PRUint32 size)
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
BlockBuffer::newBlock(PRUint32 size)
{
  Block* b = new Block(size);
  mBlocks.push_back(b);
  mPutPtr.block = mBlocks.size() - 1;
  mPutPtr.elem = 0;
}


char*
BlockBuffer::allocBlock(PRUint32 size)
{
  mBlocks[mPutPtr.block]->shrink(mPutPtr.elem);
  newBlock(size);
  return mBlocks[mPutPtr.block]->data();
}


void
BlockBuffer::advancePutPtr(PRUint32 amount)
{
  PRUint32 totalAdvanced = 0;
  Block* blockp;
  while (totalAdvanced < amount)
  {
    blockp = mBlocks[mPutPtr.block];
    PRUint32 toAdvance = blockp->size() - mPutPtr.elem;
    if (toAdvance > amount - totalAdvanced)
      toAdvance = amount - totalAdvanced;
    totalAdvanced += toAdvance;
    mPutPtr.elem += toAdvance;
    updateBufferEnd();
  }
}


PRUint32
BlockBuffer::avail()
{
  PRUint32 avail = 0;
  PRUint32 block = mGetPtr.block;
  while (block <= mPutPtr.block)
  {
    PRUint32 start = block == mGetPtr.block ? mGetPtr.elem : 0;
    PRUint32 end = block == mPutPtr.block ? mPutPtr.elem :
      mBlocks[block]->size();
    avail += end - start;
    block++;
  }
  return avail;
}
