/*
  This file is part of the Arduino_SecureElement library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef TIMED_ATTEMPT_H
#define TIMED_ATTEMPT_H

/**************************************************************************************
 * CLASS DECLARATION
 **************************************************************************************/

class TimedAttempt
{

public:
    TimedAttempt(unsigned long delay, unsigned long max_delay);

    void begin(unsigned long delay);
    void begin(unsigned long delay, unsigned long max_delay);
    unsigned long reconfigure(unsigned long delay, unsigned long max_delay);
    unsigned long retry();
    unsigned long reload();
    void reset();
    bool isRetry();
    bool isExpired();
    unsigned int getRetryCount();

private:
    unsigned long _delay;
    unsigned long _max_delay;
    unsigned long _next_retry_tick;
    unsigned int  _retry_cnt;

};

#endif /* TIMED_ATTEMPT_H */
