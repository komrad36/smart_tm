/*******************************************************************
*   smart_tm.cpp
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

#include "smart_tm.h"

std::vector<smart_tm> smart_tm::leapSecondTMs;
std::vector<size_t> smart_tm::leapSecondDeltas;
smart_tm smart_tm::epoch(DEFAULT_EPOCH_YEAR, START_MON, START_DAY, START_HR, START_MIN, START_SEC, START_FRAC_SEC);
bool smart_tm::initialized = false;

const short smart_tm::days[MONTHS_PER_YEAR] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

void smart_tm::init(const long long epochYr, const std::string& leapFile) {

	std::ifstream fLeap(leapFile);

	if (!fLeap) {
		std::cerr << "ERROR: Failed to open " << leapFile << '.' << std::endl;
		return;
	}

	leapSecondTMs.clear();
	leapSecondDeltas.clear();
	
	// leap file gives leap second times as seconds since epoch with year 1900,
	// but smart_tm allows users to specify an epoch, so we must get the number
	// of seconds in between and subtract that from each leap second
	const size_t defaultEpochToNewEpoch = numSecondsBetweenEpochs(DEFAULT_EPOCH_YEAR, epochYr);
	
	size_t defaultEpochToLeapSecond;
	size_t leapSecondsSoFar = 0;

	std::string strLine;
	initialized = true;
	while (getNextLine(fLeap, strLine)) {

		defaultEpochToLeapSecond = static_cast<size_t>(strtoull(strLine.substr(0, strLine.find_first_not_of("0123456789")).c_str(), nullptr, BASE_10_PLEASE));

		// the leap file gives time since 1900
		// Unix-style, i.e. ignoring leap seconds. 
		// smart_tm does not conform to this behavior, instead considering
		// epoch a monotonic counter INCLUDING leap seconds.
		// Therefore we must manually add 1 second to the leap file time
		// for each leap second we've already passed.
		if (defaultEpochToLeapSecond > defaultEpochToNewEpoch) {
			leapSecondDeltas.push_back(defaultEpochToLeapSecond - defaultEpochToNewEpoch + leapSecondsSoFar);
			leapSecondTMs.push_back(smart_tm(defaultEpochToLeapSecond + leapSecondsSoFar++));

			// this puts us at second '59' since the Unix/nonleap
			// counters repeat the '59' time during the leap second
			// (at '60') so we manually add 1 to put the second value at 60

			++leapSecondTMs.back().sec;
		}
	}

	fLeap.close();

	// now that all (valid) leap seconds have been added,
	// we can update the epoch according to the user's request
	epoch.yr = epochYr;
}

void smart_tm::checkInit() {
	if (!initialized) {
		std::cerr << "WARN: smart_tm not initialized! This means no leap second handling" << std::endl
			<< "and default epoch of " << DEFAULT_EPOCH_YEAR << '.' << std::endl;
	}
}

smart_tm::smart_tm(const tm& c_tm) {
	checkInit();
	yr = c_tm.tm_year + YEAR_OFFSET;
	mon = c_tm.tm_mon + MONTH_OFFSET;
	day = c_tm.tm_mday;
	hr = c_tm.tm_hour;
	min = c_tm.tm_min;
	sec = c_tm.tm_sec;
	fracSec = 0.0;
}

smart_tm::smart_tm(const tm& c_tm, const double fracSec) {
	checkInit();
	yr = c_tm.tm_year + YEAR_OFFSET;
	mon = c_tm.tm_mon + MONTH_OFFSET;
	day = c_tm.tm_mday;
	hr = c_tm.tm_hour;
	min = c_tm.tm_min;
	sec = c_tm.tm_sec;
	this->fracSec = fracSec;
}

smart_tm::smart_tm(const size_t sinceEpoch) {
	checkInit();
	*this = epoch;
	sec += sinceEpoch;
	adjust();
}

smart_tm::smart_tm(const size_t sinceEpoch, const double fracSec) {
	checkInit();
	*this = epoch;
	sec += sinceEpoch;
	this->fracSec += fracSec;
	adjust();
}

bool smart_tm::equalsWithFrac(const smart_tm& other) const {
	return yr == other.yr && mon == other.mon && day == other.day && hr == other.hr && min == other.min && sec == other.sec && fracSec == other.fracSec;
}

bool smart_tm::equalsNoFrac(const smart_tm& other) const {
	return yr == other.yr && mon == other.mon && day == other.day && hr == other.hr && min == other.min && sec == other.sec;
}

std::string smart_tm::dateToString(const char dateSeparator /* ='/' */) const {
	std::stringstream ss;
	ss << std::setfill('0') << std::setw(4) << yr << dateSeparator
		<< std::setw(2) << mon << dateSeparator
		<< std::setw(2) << day;
	return ss.str();

}

std::string smart_tm::timeToString() const {
	std::stringstream ss;
	double combinedSec = static_cast<double>(sec) + fracSec;
	ss << std::setfill('0') << std::setw(2) << hr << ':'
                << std::setw(2) << min << ':'
                << ((combinedSec >= 0.0 && combinedSec < 10.0) ? "0" : "") << std::setw(0) << std::setprecision(digits) << combinedSec;
        return ss.str();
}

std::string smart_tm::toString(const char dateSeparator /* ='/' */) const {
	return dateToString(dateSeparator) + ' ' + timeToString();
}

std::ostream& operator<<(std::ostream& os, const smart_tm& time) {
	os << time.toString();
	return os;
}

bool isLeapYear(size_t yr) {
	// Leap years occur on years evenly divisible by 4, except on
	// years divisible by 100 but not by 400.
	return (yr % 4 == 0) && ((yr % 400 == 0) || (yr % 100 != 0));
}

bool smart_tm::isLeapYear() const {
	// Leap years occur on years evenly divisible by 4, except on
	// years divisible by 100 but not by 400.
	return (yr % 4 == 0) && ((yr % 400 == 0) || (yr % 100 != 0));
}

bool smart_tm::isLeapMinute() const {
	for (auto&& leapSecond : leapSecondTMs) {
		if (yr == leapSecond.yr && mon == leapSecond.mon && day == leapSecond.day && hr == leapSecond.hr && min == leapSecond.min)
			return true;
	}
	return false;
}

short smart_tm::numDaysOfMonth() const {
	// constant and dependent only on month...
	// ...except leap years, where Feb has 29 instead of 28 days.
	return ((mon - 1 == Month::Feb) && isLeapYear()) ? days[Month::Feb] + 1 : days[mon - 1];
}

short smart_tm::numSecondsOfMinute() const {
	// constant (60)...
	// ...except leap minutes, where the minute has 60 seconds instead.
	return isLeapMinute() ? 61 : 60;
}

size_t numSecondsBetweenEpochs(const size_t startYr, const size_t endYr) {
	return (endYr - startYr)*SECONDS_PER_YEAR + leapDaysWalkedThroughFrom(startYr, endYr)*SECONDS_PER_DAY;
}

size_t smart_tm::toEpoch() const {
	size_t sum = 0;
	// step through the months since they vary in number of days
	// does not correct for leap days since that would only account
	// for the months of the same year
	// and not for all the whole years stepped through on the 
	// way from epoch to current time.
	//
	// For example, if epoch is 1900 and the time is April 1912,
	// this step would only count the 1 leap year in Feb 2012.
	//
	// Instead, no correction is applied here, and the full correction
	// occurs below via leapDaysWalkedThroughSinceEpoch().
	for (long long i = 0; i < mon - 1; ++i) {
		sum += days[i] * SECONDS_PER_DAY;
	}

	// the remainder of terms can be added directly
	// with correction applied for leap days and seconds
	sum += (yr - epoch.yr)*SECONDS_PER_YEAR + (day - START_DAY)*SECONDS_PER_DAY + (hr - START_HR)*SECONDS_PER_HOUR + (min - START_MIN)*TYPICAL_SECONDS_PER_MINUTE + leapDaysWalkedThroughSinceEpoch()*SECONDS_PER_DAY + leapSecondsWalkedThroughSinceEpoch();
	return sum + sec - START_SEC;
}

size_t smart_tm::toEpoch(double& fracSec) const {
	fracSec = this->fracSec;
	return toEpoch();
}

size_t smart_tm::leapSecondsWalkedThroughSinceEpoch() const {
	size_t ret = 0;
	for (auto&& leapSecond : leapSecondTMs) {
		if (*this > leapSecond && !equalsNoFrac(leapSecond)) ++ret;
	}

	return ret;
}

size_t smart_tm::leapDaysWalkedThroughSinceEpoch() const {
	return static_cast<size_t>(leapDaysWalkedThroughFrom(epoch, *this));
}

long long leapDaysWalkedThroughFrom(const size_t startYr, const size_t endYr) {
	// if this year is a leap year
	// subtract 1 from year for calculation purpose to avoid counting that leap day.
	long long calcYr = isLeapYear(startYr) ? startYr - 1 : startYr;

	// compute leap days since year 0 for 'start'
	long long startCount = calcYr / 4 + calcYr / 400 - calcYr / 100;

	// same adjustment as above
	calcYr = isLeapYear(endYr) ? endYr - 1 : endYr;

	// compute leap days since year 0 for 'end' and return difference
	return (calcYr / 4 + calcYr / 400 - calcYr / 100) - startCount;
}

long long leapDaysWalkedThroughFrom(const smart_tm& start, const smart_tm& end) {
	// if this year is a leap year but we are before leap day (Feb 29),
	// subtract 1 from year for calculation purpose to avoid counting that leap day.
	long long calcYr = (start.isLeapYear() && ((start.mon - 1 == smart_tm::Month::Feb && start.day <= 29) || start.mon - 1 == smart_tm::Month::Jan)) ? start.yr - 1 : start.yr;

	// compute leap days since year 0 for 'start'
	long long startCount = calcYr / 4 + calcYr / 400 - calcYr / 100;

	// same adjustment as above
	calcYr = (end.isLeapYear() && ((end.mon - 1 == smart_tm::Month::Feb && end.day <= 29) || end.mon - 1 == smart_tm::Month::Jan)) ? end.yr - 1 : end.yr;

	// compute leap days since year 0 for 'end' and return difference
	return (calcYr / 4 + calcYr / 400 - calcYr / 100) - startCount;
}

void smart_tm::fixMon() {
	if (monInLimits()) return;

	long long count;

	count = static_cast<long long>(floor(static_cast<double>(mon - START_MON) / static_cast<double>(MONTHS_PER_YEAR)));
	yr += count;
	mon -= count * MONTHS_PER_YEAR;
}

void smart_tm::stepMonNoLeapCorrection() {
	while (day > START_DAY + days[mon - 1] - 1) {
		day -= days[mon - 1];
		++mon;
		fixMon();
	}

	while (day < START_DAY) {
		--mon;
		fixMon();
		day += days[mon - 1];
	}
}

void smart_tm::stepMon() {
	while (day > START_DAY + numDaysOfMonth() - 1) {
		day -= numDaysOfMonth();
		++mon;
		fixMon();
	}

	while (day < START_DAY) {
		--mon;
		fixMon();
		day += numDaysOfMonth();
	}
}

void smart_tm::fixDay() {
	if (dayInLimits()) return;

	long long count;

	// compute by marching forward from *first* day of month...
	smart_tm old(yr, mon, START_DAY, START_HR, START_MIN, START_SEC, START_FRAC_SEC);

	// ...first moving whole years since they do not vary like months
	// thus improving compute speed versus stepping 1 month at a time all the way
	// (although this does introduce leap day error, corrected below)
	count = static_cast<long long>(static_cast<double>(day - START_DAY) / static_cast<double>(TYPICAL_DAYS_PER_YEAR));
	yr += count;
	day -= count * TYPICAL_DAYS_PER_YEAR;
	stepMonNoLeapCorrection();

	// ...now correct leap day error...
	day -= leapDaysWalkedThroughFrom(old, *this);
	// ...and bring the days and months in range.
	stepMon();
}

void smart_tm::fixHr() {
	if (hrInLimits()) return;

	long long count;

	count = static_cast<long long>(floor(static_cast<double>(hr - START_HR) / static_cast<double>(HOURS_PER_DAY)));
	day += count;
	hr -= count * HOURS_PER_DAY;

	fixDay();
}

void smart_tm::fixMin() {
	if (minInLimits()) return;

	long long count;

	count = static_cast<long long>(floor(static_cast<double>(min - START_MIN) / static_cast<double>(MINUTES_PER_HOUR)));
	hr += count;
	min -= count * MINUTES_PER_HOUR;

	fixHr();
}

void smart_tm::fixSec() {
	if (secInLimits()) return;

	long long count;
	long long addSec = sec;
	sec = 0;
	// compute by marching forward from *first* second of minute...
	size_t oldEpoch = toEpoch();

	// ...moving as many minutes as possible in one go (typical minutes, i.e. ignoring leap seconds)
	// which is vastly faster than stepping a second or a minute at a time.
	// it's constant time, in fact. It does, however, introduce leap second error...
	count = static_cast<long long>(floor(static_cast<double>(addSec - START_SEC) / static_cast<double>(TYPICAL_SECONDS_PER_MINUTE)));
	min += count;
	addSec -= count * TYPICAL_SECONDS_PER_MINUTE;

	fixMin();
	sec = addSec;
	size_t newEpoch = toEpoch();

	// ...which is corrected here...
	sec -= leapSecondsWalkedThroughFrom(oldEpoch, newEpoch);

	fixMin();

	// ...before finally bringing the corrected sec in range.
	while (sec > START_SEC + numSecondsOfMinute() - 1) {
		sec -= numSecondsOfMinute();
		++min;
		fixMin();
	}

	while (sec < START_SEC) {
		--min;
		fixMin();
		sec += numSecondsOfMinute();
	}
}

void smart_tm::fixFracSec() {
	if (fracSecInLimits()) return;

	long long count;

	count = static_cast<long long>(floor(fracSec - START_FRAC_SEC));
	sec += count;
	fracSec -= static_cast<double>(count);

	fixSec();
}

void smart_tm::adjust() {
	if (isValid()) return;

	// each of these calls propagates back up the chain after every change,
	// ensuring accurate rollover of time changes to any field.
	fixMon();
	fixDay();
	fixHr();
	fixMin();
	fixSec();
	fixFracSec();
}

bool smart_tm::isValid() const {
	return yrInLimits() && monInLimits() && dayInLimits() && hrInLimits() && minInLimits() && secInLimits() && fracSecInLimits();
}

long long leapSecondsWalkedThroughFrom(const size_t start, const size_t end) {
	long long ret = 0;
	if (end > start) {
		for (auto leapSecond : smart_tm::leapSecondDeltas) {
			if (end >= leapSecond && start <= leapSecond) ++ret;
		}
	}
	else {
		for (auto leapSecond : smart_tm::leapSecondDeltas) {
			if (start >= leapSecond && end <= leapSecond) --ret;
		}
	}

	return ret;
}
