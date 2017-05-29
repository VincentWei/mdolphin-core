// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Time represents an absolute point in time, internally represented as
// microseconds (s/1,000,000) since a platform-dependent epoch.  Each
// platform's epoch, along with other system-dependent clock interface
// routines, is defined in time_PLATFORM.cc.
//
// TimeDelta represents a duration of time, internally represented in
// microseconds.
//
// TimeTicks represents an abstract time that is always incrementing for use
// in measuring time durations. It is internally represented in microseconds.
// It can not be converted to a human-readable time, but is guaranteed not to
// decrease (if the user changes the computer clock, Time::Now() may actually
// decrease or jump).
//
// These classes are represented as only a 64-bit value, so they can be
// efficiently passed by value.

#ifndef BASE_TIME_H_
#define BASE_TIME_H_

#include <time.h>

namespace base {

class Time;
class TimeTicks;

// This unit test does a lot of manual time manipulation.
class PageLoadTrackerUnitTest;

// TimeDelta ------------------------------------------------------------------
//

class TimeDelta 
{
    public:
        TimeDelta() : delta_(0) {}

        // Converts units of time to TimeDeltas.
        static TimeDelta FromDays(int64 days);
        static TimeDelta FromHours(int64 hours);
        static TimeDelta FromMinutes(int64 minutes);
        static TimeDelta FromSeconds(int64 secs);
        static TimeDelta FromMilliseconds(int64 ms);
        static TimeDelta FromMicroseconds(int64 us);

        // Returns the internal numeric value of the TimeDelta object. Please don't
        // use this and do arithmetic on it, as it is more error prone than using the
        // provided operators.
        int64 ToInternalValue() const 
        {
            return delta_;
        }

        // Returns the time delta in some unit. The F versions return a floating
        // point value, the "regular" versions return a rounded-down value.
        //
        // InMillisecondsRoundedUp() instead returns an integer that is rounded up
        // to the next full millisecond.
        int InDays() const;
        int InHours() const;
        int InMinutes() const;
        double InSecondsF() const;
        int64 InSeconds() const;
        double InMillisecondsF() const;
        int64 InMilliseconds() const;
        int64 InMillisecondsRoundedUp() const;
        int64 InMicroseconds() const;

        TimeDelta& operator=(TimeDelta other) 
        {
            delta_ = other.delta_;
            return *this;
        }

        // Computations with other deltas.
        TimeDelta operator+(TimeDelta other) const 
        {
            return TimeDelta(delta_ + other.delta_);
        }
        TimeDelta operator-(TimeDelta other) const 
        {
            return TimeDelta(delta_ - other.delta_);
        }

        TimeDelta& operator+=(TimeDelta other) 
        {
            delta_ += other.delta_;
            return *this;
        }
        TimeDelta& operator-=(TimeDelta other) 
        {
            delta_ -= other.delta_;
            return *this;
        }
        TimeDelta operator-() const 
        {
            return TimeDelta(-delta_);
        }

        // Computations with ints, note that we only allow multiplicative operations
        // with ints, and additive operations with other deltas.
        TimeDelta operator*(int64 a) const 
        {
            return TimeDelta(delta_ * a);
        }
        TimeDelta operator/(int64 a) const 
        {
            return TimeDelta(delta_ / a);
        }
        TimeDelta& operator*=(int64 a) 
        {
            delta_ *= a;
            return *this;
        }
        TimeDelta& operator/=(int64 a) 
        {
            delta_ /= a;
            return *this;
        }
        int64 operator/(TimeDelta a) const 
        {
            return delta_ / a.delta_;
        }

        // Defined below because it depends on the definition of the other classes.
        Time operator+(Time t) const;
        TimeTicks operator+(TimeTicks t) const;

        // Comparison operators.
        bool operator==(TimeDelta other) const 
        {
            return delta_ == other.delta_;
        }
        bool operator!=(TimeDelta other) const 
        {
            return delta_ != other.delta_;
        }
        bool operator<(TimeDelta other) const 
        {
            return delta_ < other.delta_;
        }
        bool operator<=(TimeDelta other) const 
        {
            return delta_ <= other.delta_;
        }
        bool operator>(TimeDelta other) const 
        {
            return delta_ > other.delta_;
        }
        bool operator>=(TimeDelta other) const 
        {
            return delta_ >= other.delta_;
        }

    private:
        friend class Time;
        friend class TimeTicks;
        friend TimeDelta operator*(int64 a, TimeDelta td);

        // Constructs a delta given the duration in microseconds. This is private
        // to avoid confusion by callers with an integer constructor. Use
        // FromSeconds, FromMilliseconds, etc. instead.
        explicit TimeDelta(int64 delta_us) : delta_(delta_us) {}

        // Delta in microseconds.
        int64 delta_;
};

inline TimeDelta operator*(int64 a, TimeDelta td) 
{
    return TimeDelta(a * td.delta_);
}

// Time -----------------------------------------------------------------------

// Represents a wall clock time.
class Time 
{
    public:
        static const int64 kMillisecondsPerSecond = 1000;
        static const int64 kMicrosecondsPerMillisecond = 1000;
        static const int64 kMicrosecondsPerSecond = kMicrosecondsPerMillisecond *
            kMillisecondsPerSecond;
        static const int64 kMicrosecondsPerMinute = kMicrosecondsPerSecond * 60;
        static const int64 kMicrosecondsPerHour = kMicrosecondsPerMinute * 60;
        static const int64 kMicrosecondsPerDay = kMicrosecondsPerHour * 24;
        static const int64 kMicrosecondsPerWeek = kMicrosecondsPerDay * 7;
        static const int64 kNanosecondsPerMicrosecond = 1000;
        static const int64 kNanosecondsPerSecond = kNanosecondsPerMicrosecond *
            kMicrosecondsPerSecond;

#if !defined(OS_WIN)
    // On Mac & Linux, this value is the delta from the Windows epoch of 1601 to
    // the Posix delta of 1970. This is used for migrating between the old
    // 1970-based epochs to the new 1601-based ones. It should be removed from
    // this global header and put in the platform-specific ones when we remove the
    // migration code.
    static const int64 kWindowsEpochDeltaMicroseconds;
#endif

        // Represents an exploded time that can be formatted nicely. This is kind of
        // like the Win32 SYSTEMTIME structure or the Unix "struct tm" with a few
        // additions and changes to prevent errors.
        struct Exploded {
            int year;          // Four digit year "2007"
            int month;         // 1-based month (values 1 = January, etc.)
            int day_of_week;   // 0-based day of week (0 = Sunday, etc.)
            int day_of_month;  // 1-based day of month (1-31)
            int hour;          // Hour within the current day (0-23)
            int minute;        // Minute within the current hour (0-59)
            int second;        // Second within the current minute (0-59 plus leap
            //   seconds which may take it up to 60).
            int millisecond;   // Milliseconds within the current second (0-999)
        };

        // Contains the NULL time. Use Time::Now() to get the current time.
        explicit Time() : us_(0) {}

        // Returns true if the time object has not been initialized.
        bool is_null() const 
        {
            return us_ == 0;
        }

        // Returns the current time. Watch out, the system might adjust its clock
        // in which case time will actually go backwards. We don't guarantee that
        // times are increasing, or that two calls to Now() won't be the same.
        static Time Now();

        // Returns the current time. Same as Now() except that this function always
        // uses system time so that there are no discrepancies between the returned
        // time and system time even on virtual environments including our test bot.
        // For timing sensitive unittests, this function should be used.
        static Time NowFromSystemTime();

        // Converts to/from time_t in UTC and a Time class.
        // TODO(brettw) this should be removed once everybody starts using the |Time|
        // class.
        static Time FromTimeT(time_t tt);
        //  time_t ToTimeT() const;

        // Converts time to/from a double which is the number of seconds since epoch
        // (Jan 1, 1970).  Webkit uses this format to represent time.
        static Time FromDoubleT(double dt);
        double ToDoubleT() const;


        // Converts an exploded structure representing either the local time or UTC
        // into a Time class.
        static Time FromUTCExploded(const Exploded& exploded) 
        {
            return FromExploded(false, exploded);
        }
        static Time FromLocalExploded(const Exploded& exploded) 
        {
            return FromExploded(true, exploded);
        }

        // Converts an integer value representing Time to a class. This is used
        // when deserializing a |Time| structure, using a value known to be
        // compatible. It is not provided as a constructor because the integer type
        // may be unclear from the perspective of a caller.
        static Time FromInternalValue(int64 us) 
        {
            return Time(us);
        }

        // Converts a string representation of time to a Time object.
        // An example of a time string which is converted is as below:-
        // "Tue, 15 Nov 1994 12:45:26 GMT". If the timezone is not specified
        // in the input string, we assume local time.
        // TODO(iyengar) Move the FromString/FromTimeT/ToTimeT/FromFileTime to
        // a new time converter class.
        static bool FromString(const wchar_t* time_string, Time* parsed_time);

        // For serializing, use FromInternalValue to reconstitute. Please don't use
        // this and do arithmetic on it, as it is more error prone than using the
        // provided operators.
        int64 ToInternalValue() const 
        {
            return us_;
        }

        // Fills the given exploded structure with either the local time or UTC from
        // this time structure (containing UTC).
        void UTCExplode(Exploded* exploded) const 
        {
            return Explode(false, exploded);
        }
        void LocalExplode(Exploded* exploded) const 
        {
            return Explode(true, exploded);
        }

        // Rounds this time down to the nearest day in local time. It will represent
        // midnight on that day.
        Time LocalMidnight() const;

        Time& operator=(Time other) 
        {
            us_ = other.us_;
            return *this;
        }

        // Compute the difference between two times.
        TimeDelta operator-(Time other) const 
        {
            return TimeDelta(us_ - other.us_);
        }

        // Modify by some time delta.
        Time& operator+=(TimeDelta delta) 
        {
            us_ += delta.delta_;
            return *this;
        }
        Time& operator-=(TimeDelta delta) 
        {
            us_ -= delta.delta_;
            return *this;
        }

        // Return a new time modified by some delta.
        Time operator+(TimeDelta delta) const 
        {
            return Time(us_ + delta.delta_);
        }
        Time operator-(TimeDelta delta) const 
        {
            return Time(us_ - delta.delta_);
        }

        // Comparison operators
        bool operator==(Time other) const 
        {
            return us_ == other.us_;
        }
        bool operator!=(Time other) const 
        {
            return us_ != other.us_;
        }
        bool operator<(Time other) const 
        {
            return us_ < other.us_;
        }
        bool operator<=(Time other) const 
        {
            return us_ <= other.us_;
        }
        bool operator>(Time other) const 
        {
            return us_ > other.us_;
        }
        bool operator>=(Time other) const 
        {
            return us_ >= other.us_;
        }

    private:
        friend class TimeDelta;

        // Explodes the given time to either local time |is_local = true| or UTC
        // |is_local = false|.
        void Explode(bool is_local, Exploded* exploded) const;

        // Unexplodes a given time assuming the source is either local time
        // |is_local = true| or UTC |is_local = false|.
        static Time FromExploded(bool is_local, const Exploded& exploded);

        explicit Time(int64 us) : us_(us) 
    {
    }

        // The representation of Jan 1, 1970 UTC in microseconds since the
        // platform-dependent epoch.
        static const int64 kTimeTToMicrosecondsOffset;

        // Time in microseconds in UTC.
        int64 us_;
};

inline Time TimeDelta::operator+(Time t) const 
{
    return Time(t.us_ + delta_);
}

// Inline the TimeDelta factory methods, for fast TimeDelta construction.

// static
inline TimeDelta TimeDelta::FromDays(int64 days) 
{
    return TimeDelta(days * Time::kMicrosecondsPerDay);
}

// static
inline TimeDelta TimeDelta::FromHours(int64 hours) 
{
    return TimeDelta(hours * Time::kMicrosecondsPerHour);
}

// static
inline TimeDelta TimeDelta::FromMinutes(int64 minutes) 
{
    return TimeDelta(minutes * Time::kMicrosecondsPerMinute);
}

// static
inline TimeDelta TimeDelta::FromSeconds(int64 secs) 
{
    return TimeDelta(secs * Time::kMicrosecondsPerSecond);
}

// static
inline TimeDelta TimeDelta::FromMilliseconds(int64 ms) 
{
    return TimeDelta(ms * Time::kMicrosecondsPerMillisecond);
}

// static
inline TimeDelta TimeDelta::FromMicroseconds(int64 us) 
{
    return TimeDelta(us);
}

}  // namespace base

#endif  // BASE_TIME_H_