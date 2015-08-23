/*******************************************************************
*   smart_tm.h
*   Smart Time System
*	Kareem Omar
*	Student Researcher, CSPAR - UAH Class of 2017
*
*	8/15/2015
*   This program is entirely my own work.
*******************************************************************/

// Smart Time System is an improvement upon the ctime 'tm' struct,
// for certain purposes. It accounts for leap days and seconds, supports
// new constructors, functions, and operator overloads,
// handles fractional seconds and pretty (and overridable) printing,
// allows user selection of epoch year, and handles large (including
// 64-bit) values intelligently. A primary application is
// conversion between UTC and MET (mission elapsed time)
// for spacecraft, as demonstrated in the example files 'UTC_MET.h'
// and 'UTC_MET.cpp' via the TimeConverter class.
//
// NOTE: because of its aerospace origins, smart_tm's toEpoch()
// call, returning seconds (and optionally fractional seconds)
// elapsed since epoch, DOES account for leap seconds, unlike Unix
// or NTP timestamps, thus avoiding the ambiguity of those systems
// and facilitating easy, unambiguous time representation and
// conversion, especially for METs and other monotonic counting schemes.
//
// Tested for 1900-2100 for every single possible conversion, i.e.
// one for every second in that window.
//
// Actively in use in scientific analysis of data from the Fermi
// space telescope mission.
//
// Initialize with an epoch year, such as 1900. All times thereafter
// must be in or after the epoch year. Epochs closer to the times
// in use do not increase performance, but they permit smaller
// epoch values and thus increase compatibility in 32-bit systems
// or for times far into the future.
//
// Also initialize with an up-to-date official IETF/IERS leap second
// file, typically named 'leap-seconds.list'. At the time of writing,
// this file was available here:
// http://www.ietf.org/timezones/data/leap-seconds.list
//
// Call adjust() to bring a smart_tm back into valid range
// after changing a field such as minutes or seconds.
//
// Example initialization call followed by creation of a smart_tm
// and printing of that time as well as the time advanced by 200 seconds:
/*

smart_tm::init(2008, "leap-seconds.list");

// March 5, 2012, 14:30:00.000
smart_tm myTime(2012, 3, 5, 14, 30, 0, 0.0);

std::cout << myTime << std::endl;
myTime.sec += 200;
// myTime.isValid() would now return false...

myTime.adjust();
std::cout << myTime << std::endl;

*/

#ifndef SMART_TM_H
#define SMART_TM_H

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstring>
#include <ctime>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "FileIO.h"

#define BASE_10_PLEASE				(10)

// for importing from ctime tm struct
#define MONTH_OFFSET				(1)
#define YEAR_OFFSET					(1900)
/////////////////////////////////////

#define DEFAULT_EPOCH_YEAR			(1900)

#define MONTHS_PER_YEAR				(12)
// variable number of days per month
// handled at runtime by func call
#define HOURS_PER_DAY				(24)
#define MINUTES_PER_HOUR			(60)
// variable number of seconds per minute
// thanks to leap years
#define TYPICAL_SECONDS_PER_MINUTE	(60)

#define SECONDS_PER_HOUR			(3600)
#define SECONDS_PER_DAY				(86400)
#define SECONDS_PER_YEAR			(31536000ULL)

// variable number of days per year
// thanks to leap days
#define TYPICAL_DAYS_PER_YEAR		(365)

#define START_MON					(1)
#define START_DAY					(1)
#define START_HR					(0)
#define START_MIN					(0)
#define START_SEC					(0)
#define START_FRAC_SEC				(0.0)

#define END_YR						(9999)
#define END_MON						(12)
// variable end day; handled at runtime by func call
#define END_HR						(23)
#define END_MIN						(59)
// variable end second; handled at runtime by func call
#define END_FRAC_SEC				(1.0)

class smart_tm {

// Variables:

public:
	long long	yr;
	long long	mon;
	long long	day;
	long long	hr;
	long long	min;
	long long	sec;
	double		fracSec;

	enum Month {
		Jan = 0, Feb = 1, Mar = 2, Apr = 3, May = 4, Jun = 5,
		Jul = 6, Aug = 7, Sep = 8, Oct = 9, Nov = 10, Dec = 11
	};

private:
	
	static bool initialized;

	static long long epochYr;
	static smart_tm epoch;

	static const short days[MONTHS_PER_YEAR];
	static const size_t digits = std::numeric_limits<double>::digits10 + 2;
	static std::vector<smart_tm> leapSecondTMs;
	static std::vector<size_t> leapSecondDeltas;

// Methods:

public:

	// Create smart_tm set to epoch
	smart_tm() { *this = epoch; }

	// Create smart_tm from a tm struct
	smart_tm(const tm& c_tm);

	// Create smart_tm from a tm struct and fractional seconds
	smart_tm(const tm& c_tm, const double fracSec);

	// Create smart_tm as seconds since current epoch
	smart_tm(const size_t sinceEpoch);

	// Create smart_tm as seconds and fractional seconds since current epoch
	smart_tm(const size_t sinceEpoch, const double fracSec);

	// Create smart_tm from raw entry of absolute year, month, day, etc.
	smart_tm(const long long yr, const long long mon, const long long day, const long long hr, const long long min, const long long sec, const double fracSec)
		: yr(yr), mon(mon), day(day), hr(hr), min(min), sec(sec), fracSec(fracSec) {}

	~smart_tm() {}

	// Initialize epoch to first second of epochYear
	// and import leap seconds from official IERS leap seconds file,
	// typically named 'leap-seconds.list'
	static void init(const long long epochYr, const std::string& leapFile);

	// Warn user if leap seconds and epoch have not be initialised
	static void checkInit();

	friend std::ostream& operator<<(std::ostream& os, const smart_tm& time);
	friend long long leapDaysWalkedThroughFrom(const smart_tm& start, const smart_tm& end);
	friend long long leapSecondsWalkedThroughFrom(const size_t startEpoch, const size_t endEpoch);

	// Is this a possible date and time?
	bool isValid() const;

	bool isLeapYear() const;
	bool isLeapMinute() const;

	// INCLUDING fractional seconds
	bool equalsWithFrac(const smart_tm& other) const;

	// IGNORING fractional seconds
	bool equalsNoFrac(const smart_tm& other) const;

	// If not a valid time, make valid by
	// propagating any values outside the acceptable
	// range. For example, adding 1 sec to 02:02:59
	// produces 02:02:60, an invalid time,
	// but calling adjust() correctly rolls it
	// over into 02:03:00.
	//
	// Corrections propagate up the chain such that
	// even, say, 2013-12-31 23:59:60, after an adjust() call,
	// becomes 2014-01-01 00:00:00.
	//
	// Any amount of time can be added or subtracted to 
	// any field before an adjust() call, making that
	// the preferred method of adjusting smart_tm.
	void adjust();
	
	// Return seconds since epoch
	size_t toEpoch() const;

	// Return seconds since epoch, including fractional seconds
	size_t toEpoch(double& fracSec) const;

private:

	// Corrected for leap day if needed
	short numDaysOfMonth() const;

	// Inclusive
	size_t leapDaysWalkedThroughSinceEpoch() const;

	// Inclusive
	size_t leapSecondsWalkedThroughSinceEpoch() const;

	// Corrected for leap second if needed
	short numSecondsOfMinute() const;

	// These calls all expect the larger units of time above them
	// to be valid. For example, fixDay() expects a valid year and month,
	// while fixMin() expects a valid year, month, day, and hour.
	void fixMon();
	void fixDay();
	void fixHr();
	void fixMin();
	void fixSec();
	void fixFracSec();

	// restore days to acceptable range by
	// stepping month back or forward as necessary
	void stepMon();

	// restore days to acceptable range by
	// stepping month back or forward as necessary,
	// ignoring leap correction
	//
	// Used in fixDay() for coarse adjustment by year,
	// correction for which is already applied by
	// leapDaysWalkedThroughFrom().
	void stepMonNoLeapCorrection();

	inline bool yrInLimits() const { return yr >= epoch.yr && yr <= END_YR; }
	inline bool monInLimits() const { return mon >= START_MON && mon <= END_MON; }
	inline bool dayInLimits() const { return day >= START_DAY && day <= START_DAY + numDaysOfMonth() - 1; };
	inline bool hrInLimits() const { return hr >= START_HR && hr <= END_HR; }
	inline bool minInLimits() const { return min >= START_MIN && min <= END_MIN; }
	inline bool secInLimits() const { return sec >= START_SEC && sec <= START_SEC + numSecondsOfMinute() - 1.0; }
	inline bool fracSecInLimits() const { return fracSec >= START_FRAC_SEC && fracSec < END_FRAC_SEC; }
};

inline bool operator< (const smart_tm& lhs, const smart_tm& rhs) {
	return (lhs.yr == rhs.yr) ?
		((lhs.mon == rhs.mon) ?
		((lhs.day == rhs.day) ?
		((lhs.hr == rhs.hr) ?
		((lhs.min == rhs.min) ?
		((lhs.sec == rhs.sec) ?
		(lhs.fracSec < rhs.fracSec) :
		(lhs.sec < rhs.sec)) :
		(lhs.min < rhs.min)) :
		(lhs.hr < rhs.hr)) :
		(lhs.day < rhs.day)) :
		(lhs.mon < rhs.mon)) :
		(lhs.yr < rhs.yr);
}

inline bool operator>(const smart_tm& lhs, const smart_tm& rhs) { return rhs < lhs; }
inline bool operator<=(const smart_tm& lhs, const smart_tm& rhs) { return !(lhs > rhs); }
inline bool operator>=(const smart_tm& lhs, const smart_tm& rhs) { return !(lhs < rhs); }

inline double operator-(const smart_tm& lhs, const smart_tm& rhs) {
	double lFracSec, rFracSec;
	long long integralDiff = lhs.toEpoch(lFracSec) - rhs.toEpoch(rFracSec);
	return static_cast<double>(integralDiff) + (lFracSec - rFracSec);
}

long long leapDaysWalkedThroughFrom(const smart_tm& start, const smart_tm& end);
long long leapSecondsWalkedThroughFrom(const size_t start, const size_t end);

// specifically for epoch checks, i.e. Jan 1 00:00:00 of both years
long long leapDaysWalkedThroughFrom(const size_t startYr, const size_t endYr);

// Specifically the number of seconds from one epoch (Jan 1 00:00:00) to another
// for converting the leap second file's epochs (referenced to year 1900) to
// a user-specified year
size_t numSecondsBetweenEpochs(size_t startYr, size_t endYr);

#endif