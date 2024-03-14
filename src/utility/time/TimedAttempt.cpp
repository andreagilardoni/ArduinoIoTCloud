/*
  This file is part of the Arduino_SecureElement library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <Arduino.h>
#include "TimedAttempt.h"

/**************************************************************************************
 * CTOR/DTOR
 **************************************************************************************/
TimedAttempt::TimedAttempt(unsigned long delay, unsigned long max_delay)
: _delay(delay)
, _max_delay(max_delay)
{

}

/**************************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 **************************************************************************************/

void TimedAttempt::begin(unsigned long delay)
{
  _retry_cnt = 0;
  _delay = delay;
  _max_delay = delay;
  reload();
}

void TimedAttempt::begin(unsigned long delay, unsigned long max_delay)
{
   _retry_cnt = 0;
   _delay = delay;
   _max_delay = max_delay;
   reload();
}

unsigned long TimedAttempt::reconfigure(unsigned long delay, unsigned long max_delay)
{
  _delay = delay;
  _max_delay = max_delay;
  return reload();
}

unsigned long TimedAttempt::retry()
{
  _retry_cnt++;
  return reload();
}

unsigned long TimedAttempt::reload()
{
  unsigned long retry_delay = (1 << _retry_cnt) * _delay;
  _retry_delay = min(retry_delay, _max_delay);
  _next_retry_tick = millis() + retry_delay;
  return _retry_delay;
}

void TimedAttempt::reset()
{
  _retry_cnt = 0;
}

bool TimedAttempt::isRetry()
{
  return _retry_cnt > 0;
}

bool TimedAttempt::isExpired()
{
  return millis() > _next_retry_tick;
}

unsigned int TimedAttempt::getRetryCount()
{
  return _retry_cnt;
}

unsigned int TimedAttempt::getWaitTime()
{
  return _retry_delay;
}
