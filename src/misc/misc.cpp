/*
   Copyright (C) 2018-2020 Adrian Michel

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "stdafx.h"
#include <misc.h>
#include <versionno.h>
#pragma comment(lib, "rpcrt4")
#include "rpc.h"
#include "rpcdce.h"
#include <math.h>
#include "stringformat.h"

using namespace tradery;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}

MISC_API std::mutex tradery::m;
MISC_API std::mutex tradery::m_debug;
MISC_API int idcount = 0;
MISC_API int ObjCount::_objCount;
MISC_API int ObjCount::_totalObjects;
MISC_API std::wostream& ObjCount::_os = std::wcout;

static boost::posix_time::ptime EPOCH(boost::gregorian::date(1970, 1, 1));

class DateImpl;
class DateTimeImpl;

class DateDurationImpl : public DateDurationAbstr {
  friend class DateImpl;
  friend class DateTimeImpl;

 private:
  boost::gregorian::date_duration _duration;

 public:
  DateDurationImpl(boost::gregorian::date_duration duration)
      : _duration(duration) {}

  DateDurationImpl(long days) : _duration(days) {}

  DateDurationImpl() : _duration(boost::gregorian::date_duration::unit()) {}

  ~DateDurationImpl() override {}

  long days() const override { return _duration.days(); }
  bool is_negative() const override { return _duration.is_negative(); }

  bool operator==(const DateDurationAbstr& duration) const override {
    try {
      return _duration == dynamic_cast<const DateDurationImpl&>(duration)._duration;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  bool operator!=(const DateDurationAbstr& duration) const override {
    try {
      return _duration != dynamic_cast<const DateDurationImpl&>(duration)._duration;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  bool operator>(const DateDurationAbstr& duration) const override {
    try {
      return _duration > dynamic_cast<const DateDurationImpl&>(duration)._duration;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  bool operator<(const DateDurationAbstr& duration) const override {
    try {
      return _duration < dynamic_cast<const DateDurationImpl&>(duration)._duration;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  bool operator>=(const DateDurationAbstr& duration) const override {
    try {
      return _duration >= dynamic_cast<const DateDurationImpl&>(duration)._duration;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  bool operator<=(const DateDurationAbstr& duration) const override {
    try {
      return _duration <= dynamic_cast<const DateDurationImpl&>(duration)._duration;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  DateDurationAbstrPtr operator+(const DateDurationAbstr& duration) const override {
    try {
      return std::make_shared< DateDurationImpl >(_duration + dynamic_cast<const DateDurationImpl&>(duration)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  DateDurationAbstrPtr operator-(const DateDurationAbstr& duration) const override {
    try {
      return std::make_shared < DateDurationImpl >(_duration - dynamic_cast<const DateDurationImpl&>(duration)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  DateDurationAbstrPtr operator/(int divisor) {
    try {
      return std::make_shared < DateDurationImpl >(_duration / divisor);
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  DateDurationAbstrPtr operator-=(const DateDurationAbstr& duration) override {
    try {
      return std::make_shared < DateDurationImpl >(_duration -= dynamic_cast<const DateDurationImpl&>(duration)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  DateDurationAbstrPtr operator+=(const DateDurationAbstr& duration) override {
    try {
      return std::make_shared < DateDurationImpl >(_duration += dynamic_cast<const DateDurationImpl&>(duration)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  DateDurationAbstrPtr operator/=(int factor) override {
    try {
      return std::make_shared < DateDurationImpl >(_duration /= factor);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

};

class DateImpl : public DateAbstr {
  friend class DateTimeImpl;

private:
  boost::gregorian::date _date;

public:
  DateImpl(unsigned int year, unsigned int month, unsigned int day)
    : _date(year, month, day) {}

  DateImpl(const boost::gregorian::date date) : _date(date) {}

  DateImpl() {}

  std::string to_simple_string() const override {
    return boost::gregorian::to_simple_string_type<char>(_date);
  }

  std::string to_iso_string() const override {
    return boost::gregorian::to_iso_string_type<char>(_date);
  }

  std::string to_iso_extended_string() const override {
    return boost::gregorian::to_iso_extended_string_type<char>(_date);
  }

  unsigned short year() const override { return _date.year(); }
  unsigned short month() const override { return _date.month(); }
  unsigned short day() const override { return _date.day(); }
  bool is_infinity() const override { return _date.is_infinity(); }
  bool is_neg_infinity() const override { return _date.is_neg_infinity(); }
  bool is_pos_infinity() const override { return _date.is_pos_infinity(); }
  bool is_not_a_date() const override { return _date.is_not_a_date(); }
  bool is_special() const override { return _date.is_special(); }

  int week_number() const { return _date.week_number(); }
  bool operator==(const DateAbstr& date) const override {
    try {
      return _date == dynamic_cast<const DateImpl&>(date)._date;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  bool operator!=(const DateAbstr& date) const override {
    try {
      return _date != dynamic_cast<const DateImpl&>(date)._date;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  bool operator>(const DateAbstr& date) const override {
    try {
      return _date > dynamic_cast<const DateImpl&>(date)._date;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  bool operator<(const DateAbstr& date) const override {
    try {
      return _date < dynamic_cast<const DateImpl&>(date)._date;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  bool operator>=(const DateAbstr& date) const override {
    try {
      return _date >= dynamic_cast<const DateImpl&>(date)._date;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  bool operator<=(const DateAbstr& date) const override {
    try {
      return _date <= dynamic_cast<const DateImpl&>(date)._date;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  DateAbstrPtr operator-(const DateDurationAbstr& duration) const override {
    try {
      return std::make_shared < DateImpl >(_date - dynamic_cast<const DateDurationImpl&>(duration)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  DateAbstrPtr operator+(const DateDurationAbstr& duration) const override {
    try {
      return std::make_shared < DateImpl >(_date + dynamic_cast<const DateDurationImpl&>(duration)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  DateDurationAbstrPtr operator-(const DateAbstr& date) const override {
    try {
      return std::make_shared < DateDurationImpl >(_date - dynamic_cast<const DateImpl&>(date)._date);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  DateAbstrPtr operator-=(const DateDurationAbstr& duration) override {
    try {
      return std::make_shared < DateImpl >(_date -= dynamic_cast<const DateDurationImpl&>(duration)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  DateAbstrPtr operator+=(const DateDurationAbstr& duration) override {
    try {
      return std::make_shared < DateImpl >(_date += dynamic_cast<const DateDurationImpl&>(duration)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
};

MISC_API Date::Date(const std::string& xdate, DateFormat format, const std::string& sep) {
  parse(xdate, format, sep);
}

void Date::parse(const std::string& xdate, DateFormat format, const std::string& sep) {
  std::string date = tradery::trim(xdate);

  unsigned int year;
  unsigned int month;
  unsigned int day;

  if (format != dd_mmm_yyyy) {
    unsigned int first;
    unsigned int second;
    unsigned int third;

    if (!sep.empty()) {
      Tokenizer tokens(date, sep);

      if (tokens.size() != 3)
        throw DateException(date, tradery::format("Invalid date: \"%1%\"", date));

      first = atoi(tokens[0].c_str());
      second = atoi(tokens[1].c_str());
      third = atoi(tokens[2].c_str());

    }
    else {
      if (date.length() == 6) {
        first = atoi(date.substr(0, 2).c_str());
        second = atoi(date.substr(2, 2).c_str());
        third = atoi(date.substr(4, 2).c_str());
      }
      else if (date.length() == 8) {
        switch (format) {
          case us:
          case european:
            first = atoi(date.substr(0, 2).c_str());
            second = atoi(date.substr(2, 2).c_str());
            third = atoi(date.substr(4, 4).c_str());
            break;
          case iso:
            first = atoi(date.substr(0, 4).c_str());
            second = atoi(date.substr(4, 2).c_str());
            third = atoi(date.substr(6, 2).c_str());
            break;
          default:
            throw DateException(date, "Unknown format type");
            break;
        }
      }
      else {
        throw DateException(date, tradery::format("Invalid date: \"%1%\"", date));
      }
    }

    switch (format) {
      case us:
        year = third;
        month = first;
        day = second;
        break;
      case european:
        year = third;
        month = second;
        day = first;
        break;
      case iso:
        year = first;
        month = second;
        day = third;
        break;
      default:
        throw DateException(date, "Unknown format type");
        break;
    }

    year = year < 50 ? year + 2000 : (year < 100 ? year + 1900 : year);

    if (year < 1800 || year > 2100) {
      throw DateException(date, "Year must be an integer value between 1800 and 2100");
    }

    if (month < 1 || month > 12) {
      throw DateException(date, "Month must be an integer value between 1 and 12");
    }

    // make sure day is valid for month and year
    if (day < 1 || day > 31) {
      throw DateException(date, "Day must be an integer value between 1 and 31");
    }

  }
  else {
    static std::string months[] = {"", "Jan", "Feb", "Mar", "Apr",
                                   "May", "Jun", "Jul", "Aug", "Sep",
                                   "Oct", "Nov", "Dec"};

    Tokenizer tokens(xdate, sep);

    if (tokens.size() != 3) {
      throw DateException(date, "Wrong date format");
    }

    unsigned int i = 0;
    for (; i <= 12; i++) {
      if (tokens[1] == months[i]) {
        month = i;
        break;
      }
    }

    if (i > 12) {
      throw DateException(date, "Wrong date format");
    }

    year = atoi(tokens[2].c_str());

    if (year < 30) {
      year += 2000;
    }

    if (year < 100 && year >= 30) {
      year += 1900;
    }

    day = atoi(tokens[0].c_str());
  }
  _date = DateAbstr::make(year, month, day);
}

class TimeDurationImpl : public TimeDurationAbstr {
  friend class DateTimeImpl;

 private:
  boost::posix_time::time_duration _duration;

 public:
  TimeDurationImpl(__int64 hours, __int64 mins, __int64 secs, __int64 frac_secs)
      : _duration(hours, mins, secs, frac_secs) {}

  TimeDurationImpl(boost::posix_time::time_duration duration)
      : _duration(duration) {}

  long hours() const override { return _duration.hours(); }
  long minutes() const override { return _duration.minutes(); }
  long seconds() const override { return _duration.seconds(); }
  long total_seconds() const override { return _duration.total_seconds(); }
  long fractional_seconds() const override {
    return _duration.fractional_seconds();
  }
  bool is_negative() const override { return _duration.is_negative(); }

  bool operator==(const TimeDurationAbstr& duration) const override {
    try {
      return _duration == dynamic_cast<const TimeDurationImpl&>(duration)._duration;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  bool operator!=(const TimeDurationAbstr& duration) const override {
    try {
      return _duration != dynamic_cast<const TimeDurationImpl&>(duration)._duration;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  bool operator>(const TimeDurationAbstr& duration) const override {
    try {
      return _duration > dynamic_cast<const TimeDurationImpl&>(duration)._duration;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  bool operator<(const TimeDurationAbstr& duration) const override {
    try {
      return _duration < dynamic_cast<const TimeDurationImpl&>(duration)._duration;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  bool operator>=(const TimeDurationAbstr& duration) const override {
    try {
      return _duration >= dynamic_cast<const TimeDurationImpl&>(duration)._duration;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  bool operator<=(const TimeDurationAbstr& duration) const override {
    try {
      return _duration <= dynamic_cast<const TimeDurationImpl&>(duration)._duration;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  TimeDurationAbstrPtr operator+(const TimeDurationAbstr& duration) const override {
    try {
      return std::make_shared< TimeDurationImpl >(_duration + dynamic_cast<const TimeDurationImpl&>(duration)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  TimeDurationAbstrPtr operator-(const TimeDurationAbstr& duration) const override {
    try {
      return std::make_shared< TimeDurationImpl >(_duration - dynamic_cast<const TimeDurationImpl&>(duration)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  TimeDurationAbstrPtr operator/(int divisor) override {
    try {
      return std::make_shared< TimeDurationImpl >(_duration / divisor);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  TimeDurationAbstrPtr operator*(int factor) override {
    try {
      return std::make_shared< TimeDurationImpl >(_duration * factor);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  TimeDurationAbstrPtr operator-=(const TimeDurationAbstr& duration) override {
    try {
      return std::make_shared< TimeDurationImpl >( _duration -= dynamic_cast<const TimeDurationImpl&>(duration)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  TimeDurationAbstrPtr operator+=(const TimeDurationAbstr& duration) override {
    try {
      return std::make_shared< TimeDurationImpl >( _duration += dynamic_cast<const TimeDurationImpl&>(duration)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  TimeDurationAbstrPtr operator/=(int factor) override {
    try {
      return std::make_shared< TimeDurationImpl >(_duration /= factor);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  TimeDurationAbstrPtr operator*=(int factor) override {
    try {
      return std::make_shared< TimeDurationImpl >(_duration *= factor);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
};

class DateTimeImpl : public DateTimeAbstr {
 private:
  boost::posix_time::ptime _time;

 public:
  DateTimeImpl() : _time() {}

  DateTimeImpl(boost::posix_time::ptime time) : _time(time) {}

  DateTimeImpl(const DateImpl& date) : _time(date._date) {}

  DateTimeImpl(const DateImpl& date, const TimeDurationImpl& duration)
      : _time(date._date, duration._duration) {}

  DateTimeImpl(const DateTimeImpl& time) : _time(time._time) {}

  bool operator<(const DateTimeAbstr& other) const override {
    try {
      return _time < dynamic_cast<const DateTimeImpl&>(other)._time;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  bool operator>(const DateTimeAbstr& other) const override {
    try {
      return _time > dynamic_cast<const DateTimeImpl&>(other)._time;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  bool operator>=(const DateTimeAbstr& other) const override {
    try {
      return _time >= dynamic_cast<const DateTimeImpl&>(other)._time;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  bool operator<=(const DateTimeAbstr& other) const override {
    try {
      return _time <= dynamic_cast<const DateTimeImpl&>(other)._time;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  bool operator==(const DateTimeAbstr& other) const override {
    try {
      return _time == dynamic_cast<const DateTimeImpl&>(other)._time;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  bool operator!=(const DateTimeAbstr& other) const override {
    try {
      return _time != dynamic_cast<const DateTimeImpl&>(other)._time;
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  std::string to_simple_string() const override {
    return boost::posix_time::to_simple_string_type<char>(_time);
  }

  std::string to_iso_string() const override {
    return boost::posix_time::to_iso_string_type<char>(_time);
  }

  __int64 to_epoch_time() const override {
    boost::posix_time::time_duration diff = _time - EPOCH;
    return diff.total_seconds();
  }

  DateAbstrPtr date() const override {
    return std::make_shared< DateImpl >(_time.date());
  }

  TimeDurationAbstrPtr time_of_day() const override {
    return std::make_shared< TimeDurationImpl >(_time.time_of_day());
  }

  bool is_not_a_date_time() const override { return _time.is_not_a_date_time(); }

  bool is_infinity() const override { return _time.is_infinity(); }

  bool is_pos_infinity() const override { return _time.is_pos_infinity(); }

  bool is_neg_infinity() const override { return _time.is_neg_infinity(); }

  bool is_special() const override { return _time.is_special(); }

  TimeDurationAbstrPtr operator-(const DateTimeAbstr& time) const override {
    try {
      return std::make_shared< TimeDurationImpl >(_time - dynamic_cast<const DateTimeImpl&>(time)._time);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  DateTimeAbstrPtr operator+(const DateDurationAbstr& dd) const override {
    try {
      return std::make_shared< DateTimeImpl >(_time + dynamic_cast<const DateDurationImpl&>(dd)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  DateTimeAbstrPtr operator+=(const DateDurationAbstr& dd) override {
    try {
      return std::make_shared< DateTimeImpl >(_time += dynamic_cast<const DateDurationImpl&>(dd)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  DateTimeAbstrPtr operator-(const DateDurationAbstr& dd) const override {
    try {
      return std::make_shared< DateTimeImpl >(_time - dynamic_cast<const DateDurationImpl&>(dd)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  DateTimeAbstrPtr operator-=(const DateDurationAbstr& dd) override {
    try {
      return std::make_shared< DateTimeImpl >(_time -= dynamic_cast<const DateDurationImpl&>(dd)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  DateTimeAbstrPtr operator+(const TimeDurationAbstr& td) const override {
    try {
      return std::make_shared< DateTimeImpl >(_time + dynamic_cast<const TimeDurationImpl&>(td)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  DateTimeAbstrPtr operator+=(const TimeDurationAbstr& td) override {
    try {
      return std::make_shared< DateTimeImpl >(_time += dynamic_cast<const TimeDurationImpl&>(td)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  DateTimeAbstrPtr operator-(const TimeDurationAbstr& td) const override {
    try {
      return std::make_shared< DateTimeImpl >(_time - dynamic_cast<const TimeDurationImpl&>(td)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  DateTimeAbstrPtr operator-=(const TimeDurationAbstr& td) override {
    try {
      return std::make_shared< DateTimeImpl >(_time -= dynamic_cast<const TimeDurationImpl&>(td)._duration);
    }
    catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
};

MISC_API DateTimeAbstrPtr DateTimeAbstr::localTimeSubSec() {
  return std::make_shared< DateTimeImpl >(boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time());
}

MISC_API DateTimeAbstrPtr DateTimeAbstr::localTimeSec() {
  return std::make_shared< DateTimeImpl >(boost::date_time::second_clock<boost::posix_time::ptime>::local_time());
}

MISC_API DateTimeAbstrPtr DateTimeAbstr::universalTime() {
  return std::make_shared< DateTimeImpl >(boost::date_time::second_clock<boost::posix_time::ptime>::universal_time());
}

DateDurationAbstrPtr DateDurationAbstr::make(long days) {
  return std::make_shared< DateDurationImpl >(days);
}

DateDurationAbstrPtr DateDurationAbstr::make() {
  return std::make_shared< DateDurationImpl >();
}

DateTimeAbstrPtr DateTimeAbstr::makeFromIsoString(const std::string& iso_string) {
  return std::make_shared< DateTimeImpl >(boost::posix_time::from_iso_string(iso_string));
}

DateTimeAbstrPtr DateTimeAbstr::makeFromDelimitedString(const std::string& delimitedString) {
  return std::make_shared< DateTimeImpl >(boost::posix_time::time_from_string(delimitedString));
}

// YYYYMMDDHHMMSS
DateTimeAbstrPtr DateTimeAbstr::makeFromNonDelimitedString(const std::string& nonDelimitedString) {
  if (nonDelimitedString.length() != std::string("20091007233000").length()) {
    return std::make_shared< DateTimeImpl >();
  }
  else {
    std::string str(nonDelimitedString);
    str.insert(8, "T");
    return makeFromIsoString(str);
  }
}

TimeDurationAbstrPtr TimeDurationAbstr::make(__int64 hours, __int64 mins, __int64 secs, __int64 frac_secs) {
  return std::make_shared< TimeDurationImpl >(hours, mins, secs, frac_secs);
}

DateAbstrPtr DateAbstr::make(unsigned int year, unsigned int month, unsigned int day) {
  return std::make_shared< DateImpl >(year, month, day);
}

DateAbstrPtr DateAbstr::make() { return std::make_shared< DateImpl >(); }

DateTimeAbstrPtr DateTimeAbstr::make(const DateAbstr& date, const TimeDurationAbstr& duration) {
  try {
    return std::make_shared< DateTimeImpl >(dynamic_cast<const DateImpl&>(date), dynamic_cast<const TimeDurationImpl&>(duration));
  }
  catch (const std::bad_cast&) {
    assert(false);
    return 0;
  }
}

DateTimeAbstrPtr DateTimeAbstr::make() {
  return std::make_shared< DateTimeImpl >();
}
DateTimeAbstrPtr DateTimeAbstr::make(const DateAbstr& date) {
  try {
    return std::make_shared< DateTimeImpl >(dynamic_cast<const DateImpl&>(date));
  }
  catch (const std::bad_cast&) {
    assert(false);
    return 0;
  }
}

DateTimeAbstrPtr DateTimeAbstr::make(const DateTimeAbstr& time) {
  try {
    return std::make_shared< DateTimeImpl >(dynamic_cast<const DateTimeImpl&>(time));
  }
  catch (const std::bad_cast&) {
    assert(false);
    return 0;
  }
}

DateTimeAbstrPtr DateTimeAbstr::make(__int64 time) {
  boost::posix_time::ptime t(EPOCH + boost::posix_time::time_duration(time / 3600, (time % 3600) / 60, time % 60, 0));
  return std::make_shared< DateTimeImpl >(t);
}

// make special values
MISC_API DateAbstrPtr DateAbstr::makePosInfinity() {
  return std::make_shared< DateImpl >(boost::gregorian::date(boost::date_time::pos_infin));
}
MISC_API DateAbstrPtr DateAbstr::makeNegInfinity() {
  return std::make_shared< DateImpl >(boost::gregorian::date(boost::date_time::neg_infin));
}
MISC_API DateAbstrPtr DateAbstr::makeMaxDate() {
  return std::make_shared< DateImpl >(boost::gregorian::date(boost::date_time::max_date_time));
}
MISC_API DateAbstrPtr DateAbstr::makeMinDate() {
  return std::make_shared< DateImpl >(boost::gregorian::date(boost::date_time::min_date_time));
}
MISC_API DateAbstrPtr DateAbstr::makeNotADate() {
  return std::make_shared< DateImpl >(boost::gregorian::date(boost::date_time::not_a_date_time));
}

MISC_API DateTimeAbstrPtr DateTimeAbstr::makePosInfinity() {
  return std::make_shared< DateTimeImpl >(boost::posix_time::ptime(boost::date_time::pos_infin));
}

MISC_API DateTimeAbstrPtr DateTimeAbstr::makeNegInfinity() {
  return std::make_shared< DateTimeImpl >(boost::posix_time::ptime(boost::date_time::neg_infin));
}
MISC_API DateTimeAbstrPtr DateTimeAbstr::makeMaxDateTime() {
  return std::make_shared< DateTimeImpl >(boost::posix_time::ptime(boost::date_time::max_date_time));
}
MISC_API DateTimeAbstrPtr DateTimeAbstr::makeMinDateTime() {
  return std::make_shared< DateTimeImpl >( boost::posix_time::ptime(boost::date_time::min_date_time));
}
MISC_API DateTimeAbstrPtr DateTimeAbstr::makeNotADateTime() {
  return std::make_shared< DateTimeImpl >(boost::posix_time::ptime(boost::date_time::not_a_date_time));
}

DateAbstrPtr DateAbstr::make(const DateAbstr& date) {
  try {
    return std::make_shared< DateImpl >(dynamic_cast<const DateImpl&>(date));
  }
  catch (const std::bad_cast&) {
    assert(false);
    return 0;
  }
}

TimeDurationAbstrPtr TimeDurationAbstr::make(const TimeDurationAbstr& duration) {
  try {
    return std::make_shared< TimeDurationImpl >(dynamic_cast<const TimeDurationImpl&>(duration));
  }
  catch (const std::bad_cast&) {
    assert(false);
    return 0;
  }
}


class TimerImpl : public TimerAbstr {
 private:
  boost::timer _timer;
  double _lastValue;
  bool _running;

 public:
  TimerImpl() : _running(true), _lastValue(0.0) {}

  void restart() {
    _timer.restart();
    _running = true;
  }

  double elapsed() const { return _running ? _timer.elapsed() : _lastValue; }

  void stop() {
    if (_running) {
      _lastValue = _timer.elapsed();
      _running = false;
    }
  }

  bool isStopped() const { return !_running; }
};

TimerAbstr* TimerAbstr::make() { return new TimerImpl(); }

MISC_API Version::Version() : Version(0, 0, 0, 0) {}

MISC_API Version::Version(const char* version)
    : m_major(0), m_minor(0), m_build(0), m_revision(0) {
  parse(version);
}

Version::Version(unsigned int major, unsigned int minor, unsigned int revision, unsigned int build)
    : m_major(major), m_minor(minor), m_revision(revision), m_build(build) {}

MISC_API void Version::parse(const char* version) {
  if (version != nullptr && strlen(version) > 0) {
    Tokenizer tokens(version, VERSION_SEPARATORS);

    try {
      for (size_t n = 0; n < tokens.size(); ++n) {
        unsigned int value = boost::lexical_cast<unsigned int>(tokens[n]);
        switch (n) {
          case 0:
            m_major = value;
            break;
          case 1:
            m_minor = value;
            break;
          case 2:
            m_revision = value;
            break;
          case 3:
            m_build = value;
            break;
          default:
            throw VersionException(std::string(version) + " - too many version elements");
        }
      }
    } 
    catch (const boost::bad_lexical_cast& e) {
      throw VersionException(std::string(version) + ", " + e.what());
    }
  }
}

MISC_API Seconds::Seconds(double seconds)
    : TimeDuration(0, 0, (long)seconds, fmod(seconds, 1.0) * 1000000) {}

MISC_API std::vector<std::string> tradery::cmdLineSplitter(const std::string& line) {
  std::vector<std::string> v;

  bool inQuotedString = false;
  bool inUnquotedString = false;
  bool escape = false;

  std::string sep(" \t");
  std::string str;
  unsigned int state = 0;

  for (char c : line ) {
    switch (state) {
      case 0:
        assert(str.empty());
        switch (c) {
          case '"':
            state = 1;
            break;
          case ' ':
          case '\t':
          case '\r':
          case '\n':
            break;
          default:
            str += c;
            state = 2;
            break;
        }
        break;
      case 1:
        switch (c) {
          case '"':
            state = 2;
            break;
          case '\\':
            str += c;
            state = 3;
            break;
          default:
            str += c;
            break;
        }
        break;
      case 2:
        switch (c) {
          case ' ':
          case '\t':
          case '\r':
          case '\n':
            // assert( !str.empty() );
            if (!str.empty()) {
              v.push_back(str);
              str.clear();
            }
            state = 0;
            break;
          default:
            str += c;
            break;
        }
        break;
      case 3:
        switch (c) {
          case '"':
            state = 4;
            break;
          default:
            state = 1;
            break;
        }
        str += c;
        break;
      case 4:
        switch (c) {
          case ' ':
          case '\t':
          case '\r':
          case '\n':
            // assert( !str.empty() );
            if (!str.empty()) {
              v.push_back(str);
              str.clear();
            }
            state = 0;
            break;
          default:
            str += c;
            state = 1;
            break;
        }
        break;
      default:
        assert(false);
        break;
    }
  }

  if (!str.empty()) v.push_back(str);
  return v;
}

MISC_API std::string tradery::Date::toString(DateFormat format, const std::string& separator) const {
  if (is_special())
    return "";
  else {
    unsigned int first;
    unsigned int second;
    unsigned int third;

    switch (format) {
      case us:
        first = month();
        second = day();
        third = year();
        break;
      case european:
        first = day();
        second = month();
        third = year();
        break;
      case iso:
        first = year();
        second = month();
        third = day();
        break;
      case dd_mmm_yyyy:
        throw DateException("", "Format xyz not supported at this time");
      default:
        throw DateException("", "Unknown format type");
    }

    return tradery::format("%1%%4%%2%%4%%3%", first, second, third, separator);
  }
}
