/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "Reactor.h"
#include <map>
#include "EventHandler.h"

bool
Reactor::Timeout::expired()
{
  return static_cast<PRIntervalTime>(PR_IntervalNow() - epoch) > interval;
}


Reactor* Reactor::mInstance = NULL;

Reactor::Reactor()
{
}


Reactor*
Reactor::instance()
{
  if (!mInstance)
    mInstance = new Reactor();
  return mInstance;
}


void
Reactor::registerHandler(EventHandler* evtHandler)
{
  mEvtHandlers.push_back(evtHandler);
}


void
Reactor::removeHandler(EventHandler* evtHandler)
{
  for (std::vector<EventHandler*>::iterator i = mEvtHandlers.begin();
       i != mEvtHandlers.end(); i++)
  {
    if (*i == evtHandler)
    {
      mEvtHandlers.erase(i);
      return;
    }
  }
}


void
Reactor::run()
{
  std::map<PRFileDesc*, EventHandler*> handleMap;

  // FIXME: Blech some double copies here that are surely avoidable.
  std::vector<PRPollDesc> descs;
  for (std::vector<EventHandler*>::iterator i = mEvtHandlers.begin();
       i != mEvtHandlers.end(); i++)
  {
    std::vector<PRPollDesc> handlerDescs;
    (*i)->getPollDescs(handlerDescs);
    for (std::vector<PRPollDesc>::iterator j = handlerDescs.begin();
         j != handlerDescs.end(); j++)
    {
      descs.push_back(*j);
      handleMap[(*j).fd] = *i;
    }
  }
  PRIntn npds = descs.size();
  PRPollDesc* pds = new PRPollDesc[npds];
  int count = 0;
  for (std::vector<PRPollDesc>::iterator i = descs.begin();
       i != descs.end(); i++)
  {
    pds[count] = *i;
    count++;
  }

  // FIXME: Use default timeout unless there is a lesser timeout in
  // mTimeouts.
  PRInt32 npdsReady = PR_Poll(pds, npds, PR_MillisecondsToInterval(100));

  // FIXME: log errors
  if (npdsReady > 0)
  {
    for (int i = 0; i < npds; i++)
    {
      if (pds[i].out_flags)
      {
        EventHandler* evtHandler = handleMap[pds[i].fd];
        evtHandler->handleEvent(pds[i]);
      }
    }
  }

  delete[] pds;

  // Copy pointers to expired handlers in case the handlers
  // modify the timeouts list, which causes problems when
  // iterating through a vector.
  // FIXME: Determine if there's a cleaner way to do this.
  std::vector<EventHandler*> expiredHandlers;

  for (std::vector<Timeout>::iterator i = mTimeouts.begin();
       i != mTimeouts.end(); i++)
  {
    if ((*i).expired())
    {
      expiredHandlers.push_back((*i).evtHandler);
      mTimeouts.erase(i);
      i = mTimeouts.begin();
    }
  }

  for (std::vector<EventHandler*>::iterator i = expiredHandlers.begin();
       i != expiredHandlers.end(); i++)
    (*i)->handleTimeout();

  deleteClosed();
}


void
Reactor::stop()
{
  for (std::vector<EventHandler*>::iterator i = mEvtHandlers.begin();
       i != mEvtHandlers.end(); )
  {
    (*i)->close();
    delete (*i);
    mEvtHandlers.erase(i);
  }
}


void
Reactor::setTimeout(PRIntervalTime interval, EventHandler* evtHandler)
{
  Timeout t;
  t.epoch = PR_IntervalNow();
  t.interval = interval;
  t.evtHandler = evtHandler;
  mTimeouts.push_back(t);
}


void
Reactor::deleteClosed()
{
  for (std::vector<EventHandler*>::iterator i = mEvtHandlers.begin();
       i != mEvtHandlers.end(); i++)
  {
    if ((*i)->closed())
    {
      EventHandler* hdlr = *i;
      mEvtHandlers.erase(i);
      i = mEvtHandlers.begin();
      for (std::vector<Timeout>::iterator j = mTimeouts.begin();
           j != mTimeouts.end(); j++)
      {
        if ((*j).evtHandler == hdlr)
        {
          mTimeouts.erase(j);
          j = mTimeouts.begin();
        }
      }
      delete hdlr;
    }
  }
}
