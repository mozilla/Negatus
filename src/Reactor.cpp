/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "Reactor.h"
#include <map>
#include "EventHandler.h"

Reactor::Timeout::Timeout(PRIntervalTime _epoch, PRIntervalTime _interval,
  EventHandler* _evtHandler)
  : epoch(_epoch), interval(_interval), evtHandler(_evtHandler)
{
}


Reactor::Timeout::Timeout(const Timeout& t)
  : epoch(t.epoch), interval(t.interval), evtHandler(t.evtHandler)
{
}


Reactor::Timeout&
Reactor::Timeout::operator=(const Timeout& rhs)
{
  epoch = rhs.epoch;
  interval = rhs.interval;
  evtHandler = rhs.evtHandler;
  return *this;
}


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
       i != mTimeouts.end(); )
  {
    if ((*i).expired())
    {
      expiredHandlers.push_back((*i).evtHandler);
      i = mTimeouts.erase(i);
    }
    else
      ++i;
  }

  for (std::vector<EventHandler*>::iterator i = expiredHandlers.begin();
       i != expiredHandlers.end(); i++)
  {
    (*i)->handleTimeout();
  }

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
    i = mEvtHandlers.erase(i);
  }
}


void
Reactor::setTimeout(PRIntervalTime interval, EventHandler* evtHandler)
{
  Timeout t(PR_IntervalNow(), interval, evtHandler);
  mTimeouts.push_back(t);
}


void
Reactor::deleteClosed()
{
  for (std::vector<EventHandler*>::iterator i = mEvtHandlers.begin();
       i != mEvtHandlers.end(); )
  {
    if ((*i)->closed())
    {
      EventHandler* hdlr = *i;
      i = mEvtHandlers.erase(i);
      for (std::vector<Timeout>::iterator j = mTimeouts.begin();
           j != mTimeouts.end(); )
      {
        if ((*j).evtHandler == hdlr)
          j = mTimeouts.erase(j);
        else
          ++j;
      }
      delete hdlr;
    }
    else
      ++i;
  }
}
