// Copyright 2013, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef CODE_LECTURE_04_TIMER_H_
#define CODE_LECTURE_04_TIMER_H_

#include <time.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <locale>
#include <string>
#include <sstream>
#include <iostream>

using std::string;

class Timer {
  public:

  // Gets a readable timestamp as string.
  static string getTimeStamp() {
    struct timeb timebuffer;
    char timeline[26];
    ftime(&timebuffer);
    ctime_r(&timebuffer.time, timeline);
    timeline[19] = '.';
    timeline[20] = 0;

    std::ostringstream os;
    os << timeline;
    os.fill('0');
    os.width(3);
    os << timebuffer.millitm;
    return os.str();
  }

  // Resets the timer value to zero and starts the measurement.
  inline void start() {
    _usecs = 0;
    cont();
  }

  // Starts the measurement.
  inline void cont() {
    gettimeofday(&_tstart, &_tz);
  }

  // Stops the measurement
  inline void stop() {
    gettimeofday(&_tend, &_tz);
    _usecs = (off_t) (1000000) * (off_t) (_tend.tv_sec - _tstart.tv_sec)
          + (off_t) (_tend.tv_usec - _tstart.tv_usec);
  }

  inline off_t getUSecs() {
    return _usecs;
  }

  private:
  // Timer value.
  off_t _usecs;

  // Used by the gettimeofday command.
  struct timeval _tstart;
  struct timeval _tend;
  struct timezone _tz;
};


#endif  // CODE_LECTURE_04_TIMER_H_
