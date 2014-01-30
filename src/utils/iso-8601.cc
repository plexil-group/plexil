/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
*  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Universities Space Research Association nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Error.hh"
#include "timespec-utils.hh"

#include <cmath>
#include <cstring>
#include <ctime>

// Parse the first len characters of in as an integer and store result in dest.
// If successful, returns pointer to the first character of in which is NOT a digit
// If failed (substring does not start with digit), returns NULL and does not modify dest
static const char* substrtoi(const char* in, size_t len, int& dest)
{
  if (in == NULL || !isdigit(*in) || len == 0) // || len > 5 ?
    return NULL;

  // First character is known to be a digit
  int result = (*in++ - '0');
  while (--len && isdigit(*in))
    result = result * 10 + (*in++ - '0');

  dest = result;
  return in;
}

// Internal function

static const char* parseTimeInternal(const char* dateStr, bool extended, struct tm& date, double& fracSecs)
{
  if (dateStr == NULL || *dateStr == '\0') {
    std::cout << "parseTimeInternal: null or empty input" << std::endl;
    return NULL;
  }
  
  const char* next = substrtoi(dateStr, 2, date.tm_hour);
  if (next == NULL || date.tm_hour > 24) { // allow for 24:00:00
    std::cout << "parseTimeInternal: invalid hour" << std::endl;
    return NULL;
  }

  // TODO?: Allow minutes & secs to be elided
  if (extended && *next++ != ':') {
    std::cout << "parseTimeInternal: extended format and no separator between hours and minutes" << std::endl;
    return NULL;
  }

  next = substrtoi(next, 2, date.tm_min);
  if (next == NULL || date.tm_min > 59) {
    std::cout << "parseTimeInternal: invalid minute" << std::endl;
    return NULL;
  }

  // Allow seconds to be elided
  // if extended format, next char could be one of ":Z+-"
  if (extended && *next != '\0') {
    if (*next == ':') 
      ++next;
    else if (!strchr("+-Z", *next)) {
      std::cout << "parseTimeInternal: extended format, no separator after seconds, and invalid timezone spec" << std::endl;
      return NULL;
    }
  }
  if (isdigit(*next)) {
    next = substrtoi(next, 2, date.tm_sec);
    if (next == NULL || date.tm_sec > 60) { // allow for leap second
      std::cout << "parseTimeInternal: invalid seconds" << std::endl;
      return NULL;
    }

    // Fractional seconds
    if (*next == '.' || *next == ',') {
      // TODO: Parse fractional second
    }
  }

  // Timezone
  if (*next == 'Z') {
    ++next;
#if !defined(__VXWORKS__) /* tm lacks tm_gmtoff */
    // Offset by local time
    fracSecs += date.tm_gmtoff;
#endif
  }
  else if (*next == '+' || *next == '-') {
    bool isPlus = (*next == '+');
    ++next;
    int offsetHrs = 0;
    int offsetMins = 0;
    next = substrtoi(next, 2, offsetHrs);
    if (extended) {
      if (*next++ != ':')
        return NULL;
    }
    if (isdigit(*next))
      next = substrtoi(next, 2, offsetMins);
    long totalOffset = 60 * (offsetMins + 60 * offsetHrs);
#if !defined(__VXWORKS__) /* tm lacks tm_gmtoff */
    // Figure out how much to adjust
    // Must add local timezone delta, then add the spec'd offset
    fracSecs = fracSecs + (isPlus ? totalOffset : -totalOffset) + date.tm_gmtoff;
#endif
  }
  // else it's local time

  return next;
}

extern const char* parseISO8601Date(const char* dateStr, double& result)
{
  if (dateStr == NULL || *dateStr == '\0')
    return NULL;

  // Get current date and time to default local timezone
  time_t now;
  struct tm date;
  time(&now);
  localtime_r(&now, &date);

  double frac = 0; // fractional seconds
  bool extended = false;

  // Parse into date and/or frac
  int year = 0;
  const char* next = substrtoi(dateStr, 4, year);
  if (next == NULL || year < 1970) {
    std::cout << "parseISO8601Date: Invalid year" << std::endl;
    return NULL;
  }
  date.tm_year = year - 1900;

  // Check extended format
  if (*next == '-') {
    extended = true;
    ++next;
  }
  if (!isdigit(*next)) {
    std::cout << "parseISO8601Date: month is not a digit" << std::endl;
    return NULL;
  }
  next = substrtoi(next, 2, date.tm_mon);
  if (next == NULL || date.tm_mon == 0 || date.tm_mon > 12) {
    std::cout << "parseISO8601Date: invalid month" << std::endl;
    return NULL;
  }
  if (extended && *next++ != '-') {
    std::cout << "parseISO8601Date: extended format missing separator between month and day" << std::endl;
    return NULL;
  }
  if (!isdigit(*next)) {
    std::cout << "parseISO8601Date: day is not a digit" << std::endl;
    return NULL;
  }
  next = substrtoi(next, 2, date.tm_mday);
  if (next == NULL || date.tm_mday == 0 || date.tm_mday > 31) { // check for months < 31?
    std::cout << "parseISO8601Date: invalid day" << std::endl;
    return NULL;
  }

  // Date parsed, is there a time?
  if (*next == 'T') {
    next = parseTimeInternal(++next, extended, date, frac);
    if (next == NULL) {
      std::cout << "parseISO8601Date: time parsing failed" << std::endl;
      return NULL;
    }
  }

  // turn date into time_t
  time_t tt = mktime(&date);

  // turn time_t into double and add fractional seconds
  result = frac + (double) tt;

  return next;
}

extern const char* parseISO8601Time(const char* timeStr, double& result)
{
  // Sanity check input
  assertTrue_1(timeStr != NULL);
  assertTrue_1(*timeStr != '\0');

  // Get current date and time to default local timezone
  time_t now;
  struct tm date;
  time(&now);
  localtime_r(&now, &date);

  // Is this extended time format? Look at 3rd char.
  bool extended = timeStr[1] != '\0' && timeStr[2] == ':';

  double frac = 0;
  const char* next = parseTimeInternal(timeStr, extended, date, frac);
  if (next == NULL)
    return NULL;

  // turn date into time_t
  time_t tt = mktime(&date);

  // turn time_t into double and add fractional seconds
  result = frac + (double) tt;

  return next;
}

extern const char* parseISO8601Duration(const char* durStr, double& result)
{
  if (durStr == NULL || *durStr != 'P') {
    std::cout << "parseISO8601Duration: null pointer, empty string, or not a duration format" << std::endl;
    return NULL;
  }

  const char* next = durStr;
  int years = 0;
  int months = 0;
  int days = 0;
  int hours = 0;
  int minutes = 0;
  int seconds = 0;
  
  int tmp = 0;
  const char* designator = NULL;

  bool complete = false;
  bool extended = false;
  bool basic = false;

  // Complete, alternative formats overlap somewhat;
  // determining which is which can be very complicated
  if (isdigit(*++next)) {
    // Could be complete, alternative basic or alternative extended
    if ((designator = strpbrk(next, "DMWY"))) {
      // Complete format
      complete = true;
      next = substrtoi(next, designator - next, tmp);
      if (next != designator) {
        // junk in string
        std::cout << "parseISO8601Duration: invalid complete format for \"" << durStr << "\"" << std::endl;
        return NULL;
      }
      if ('W' == *next) {
        // Weeks - we're done
        result = (double) (tmp * 7 * 24 * 60 * 60);
        return ++next;
      }
      if ('Y' == *next) {
        years = tmp;
        if ((designator = strpbrk(++next, "DM"))) {
          next = substrtoi(next, designator - next, tmp);
          if (next != designator) {
            // junk in string
            std::cout << "parseISO8601Duration: invalid complete format for \"" << durStr << "\"" << std::endl;
            return NULL;
          }
        }
      }
      if (designator && 'M' == *next) {
        months = tmp;
        designator = strchr(++next, 'D');
        if (designator != NULL) {
          next = substrtoi(next, designator - next, tmp);
          if (next != designator) {
            // junk in string
            std::cout << "parseISO8601Duration: invalid complete format for \"" << durStr << "\"" << std::endl;
            return NULL;
          }
        }
      }
      if (designator && 'D' == *next) {
        days = tmp;
        ++next;
      }
      // Should now be at T designator or end of string
    }
    else {
      // First 4 digits are years
      next = substrtoi(next, 4, years);
      if (next == NULL) {
        std::cout << "parseISO8601Duration: invalid alternative format for \"" << durStr << "\"" << std::endl;
        return NULL;
      }
      bool yyyyddd = false;
      if ('-' == *next) {
        extended = true;
        ++next;
        if (*next && next[1] && isdigit(next[2]))
          yyyyddd = true;
      }
      else {
        basic = true;
        if (*next && next[1] && next[2] && !isdigit(next[3]))
          yyyyddd = true;
      }
      if (yyyyddd) {
        next = substrtoi(next, 3, days);
        if (next == NULL) {
          std::cout << "parseISO8601Duration: invalid alternative year-day format for \"" << durStr << "\"" << std::endl;
          return NULL;
        }
      }
      else {
        next = substrtoi(next, 2, months);
        if (next == NULL) {
          std::cout << "parseISO8601Duration: invalid alternative year-month-day format for \"" << durStr << "\"" << std::endl;
          return NULL;
        }
        if (extended) {
          if (*next != '-') {
            std::cout << "parseISO8601Duration: invalid extended date format for \"" << durStr << "\"" << std::endl;
            return NULL;
          }
          else 
            ++next;
        }
        next = substrtoi(next, 2, days);
        if (next == NULL) {
          std::cout << "parseISO8601Duration: invalid alternative year-month-day format for \"" << durStr << "\"" << std::endl;
          return NULL;
        }
      }
    }
  } // if (isdigit(...))

  // Have parsed date (if any), should now be at T or end of string 
  if (*next == '\0') {
    if (years == 0 && months == 0 && days == 0) {
      std::cout << "parseISO8601Duration: empty duration \"" << durStr << "\"" << std::endl;
      return NULL;
    }
  }
  else if (*next == 'T') {
    ++next;
    if (next - durStr == 2) {
      // no previous data, figure out format
      if (NULL != strpbrk(next, "HMS"))
        complete = true;
    }
    if (complete) {
      if (next == NULL || *next == '\0') {
        std::cout << "parseISO8601Duration: empty alternative time specification for \"" << durStr << "\"" << std::endl;
        return NULL;
      }

      if (!(designator = strpbrk(next, "HMS"))) {
        std::cout << "parseISO8601Duration: invalid complete format for \"" << durStr << "\"" << std::endl;
        return NULL;
      }
      next = substrtoi(next, designator - next, tmp);
      if (next != designator) {
        // junk in string
        std::cout << "parseISO8601Duration: invalid complete format for \"" << durStr << "\"" << std::endl;
        return NULL;
      }
      if ('H' == *next) {
        hours = tmp;
        if ((designator = strpbrk(++next, "MS."))) {
          next = substrtoi(next, designator - next, tmp);
          if (next != designator) {
            // junk in string
            std::cout << "parseISO8601Duration: invalid complete format for \"" << durStr << "\"" << std::endl;
            return NULL;
          }
        }
      }
      if ('M' == *next) {
        minutes = tmp;
        if ((designator = strchr(++next, 'S'))) {
          next = substrtoi(next, designator - next, tmp);
          if (next != designator) {
            // junk in string
            std::cout << "parseISO8601Duration: invalid complete format for \"" << durStr << "\"" << std::endl;
            return NULL;
          }
        }
      }
      if ('S' == *next) {
        seconds = tmp;
        next++;
      }
      // FIXME: am I missing any error conditions?
    }
    else {
      // Alternative format
      if (next == NULL || *next == '\0') {
        std::cout << "parseISO8601Duration: empty alternative time format for \"" << durStr << "\"" << std::endl;
        return NULL;
      }
      next = substrtoi(next, 2, hours);
      if (next == NULL) {
        std::cout << "parseISO8601Duration: invalid alternative time format for \"" << durStr << "\"" << std::endl;
        return NULL;
      }
      // Valid to end string here
      if (*next != '\0') {
        if (*next == ':') {
          if (basic) {
            std::cout << "parseISO8601Duration: invalid alternative basic time format for \"" << durStr << "\"" << std::endl;
            return NULL;
          }
          extended = true;
          ++next;
        }
        next = substrtoi(next, 2, minutes);
        if (next == NULL) {
          std::cout << "parseISO8601Duration: invalid alternative time format for \"" << durStr << "\"" << std::endl;
          return NULL;
        }
        if (*next != '\0') {
          if (extended && *next++ != ':') {
            std::cout << "parseISO8601Duration: invalid alternative extendede time format for \"" << durStr << "\"" << std::endl;
            return NULL;
          }
          next = substrtoi(next, 2, seconds);
          if (next == NULL) {
            std::cout << "parseISO8601Duration: invalid alternative time format for \"" << durStr << "\"" << std::endl;
            return NULL;
          }
        }
      }
    }
  }
  else if (isdigit(*next)) {
    std::cout << "parseISO8601Duration: invalid format for \"" << durStr << "\"" << std::endl;
    return NULL;
  }

  long resulttmp = seconds + 60 * (minutes + 60 * (hours + 24 * (days + 30 * months + 365 * years)));
  result = (double) resulttmp;
  return next;
}

static void formatDate(const struct tm& timestruct, 
                       long nsecs,
                       bool extendedFmt,
                       std::ostream& stream)
{
  stream.width(4);
  stream.fill('0');
  stream << (timestruct.tm_year + 1900);
  if (extendedFmt)
    stream.put('-');
  stream.width(2);
  stream.fill('0');
  stream << timestruct.tm_mon;
  if (extendedFmt)
    stream.put('-');
  stream.width(2);
  stream.fill('0');
  stream << timestruct.tm_mday;
  stream.put('T');
  stream.width(2);
  stream.fill('0');
  stream << timestruct.tm_hour;
  if (extendedFmt)
    stream.put(':');
  stream.width(2);
  stream.fill('0');
  stream << timestruct.tm_min;
  // TODO?: be smart about zero seconds
  if (extendedFmt)
    stream.put(':');
  stream.width(2);
  stream.fill('0');
  stream << timestruct.tm_sec;

  // Format nanoseconds to stream
  if (nsecs) {
    stream.put('.');
    stream.fill('0');
    // Be smart (?) about precision
    if (0 == nsecs % 1000000) {
      stream.width(3);
      stream << (nsecs / 1000000);
    }
    else if (0 == nsecs % 1000) {
      stream.width(6);
      stream << (nsecs / 1000);
    }
    else {
      stream.width(9);
      stream << nsecs;
    }
  }
}

/**
 * @brief Output the double as an ISO 8601 extended date on the given stream.
 * @param unixDate Seconds since the Unix epoch.
 * @param stream The output stream.
 */
extern void printISO8601Date(double unixDate, std::ostream& stream)
{
  bool extendedFmt = true;

  // convert to timespec
  timespec ts;
  doubleToTimespec(unixDate, ts);
  // FIXME: check for invalid unixDate

  // convert seconds to tm
  struct tm timestruct;
  if (NULL == localtime_r(&ts.tv_sec, &timestruct)) {
    stream << "[INVALID DATE " << unixDate << "]";
    return;
  }

  // Format tm values to stream
  formatDate(timestruct, ts.tv_nsec, extendedFmt, stream);
}

extern void printISO8601DateUTC(double unixDate, std::ostream& stream)
{
  bool extendedFmt = true;

  // convert to timespec
  timespec ts;
  doubleToTimespec(unixDate, ts);
  // FIXME: check for invalid unixDate

  // convert seconds to tm
  struct tm timestruct;
  if (NULL == gmtime_r(&ts.tv_sec, &timestruct)) {
    stream << "[INVALID DATE " << unixDate << "]";
    return;
  }

  // Format tm values to stream
  formatDate(timestruct, ts.tv_nsec, extendedFmt, stream);
  stream.put('Z');
}

extern void printISO8601Duration(double dur, std::ostream& stream)
{
  // break down into components
  int min = 0;
  int hrs = 0;
  int days = 0;
  int mos = 0;
  int yrs = 0;

  double sec = fmod(dur, 60.0);
  long tmp = (long) floor(dur/60.0);
  if (tmp) {
    min = tmp % 60;
    tmp = tmp / 60;
    if (tmp) {
      hrs = tmp % 24;
      tmp = tmp / 24;
      if (tmp) {
        // be smart about years/months
        if (tmp > 365) {
          yrs = tmp / 365;
          tmp = tmp % 365;
        }
        if (tmp > 30) {
          mos = tmp / 30;
          tmp = tmp % 30;
        }
        days = tmp;
      }
    }
  }

  // print 'em
  bool printSecs = sec != 0.0 || (!yrs && !mos && !days && !hrs && !min);

  stream.put('P');
  if (yrs)
    stream << yrs << 'Y';
  if (mos)
    stream << mos << 'M';
  if (days)
    stream << days << 'D';

  if (hrs || min || printSecs) {
    stream.put('T');
    if (hrs)
      stream << hrs << 'H';
    if (min)
      stream << min << 'M';
    if (printSecs)
      stream << sec << 'S';
  }
}

