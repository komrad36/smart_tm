# smart_tm

Smart Time System (smart_tm) is a smart, leap-second- and leap-day-aware,
fast, 64-bit-capable replacement for the ctime 'tm' struct.

*******************************************************************
Smart Time System - Kareem Omar - Student Researcher, CSPAR - UAH Class of 2017
*******************************************************************

 Smart Time System is an improvement upon the ctime 'tm' struct,
 for certain purposes. It accounts for leap days and seconds, supports
 new constructors, functions, and operator overloads,
 handles fractional seconds and pretty (and overridable) printing,
 allows user selection of epoch year, and handles large (including
 64-bit) values intelligently. A primary application is
 conversion between UTC and MET (mission elapsed time)
 for spacecraft, as demonstrated in the example files 'UTC_MET.h'
 and 'UTC_MET.cpp' via the TimeConverter class.

 NOTE: because of its aerospace origins, smart_tm's toEpoch()
 call, returning seconds (and optionally fractional seconds)
 elapsed since epoch, DOES account for leap seconds, unlike Unix
 or NTP timestamps, thus avoiding the ambiguity of those systems
 and facilitating easy, unambiguous time representation and
 conversion, especially for METs and other monotonic counting schemes.

 Tested for 1900-2100 for every single possible conversion, i.e.
 one for every second in that window.

 Actively in use in scientific analysis of data from the Fermi
 space telescope mission.

 Initialize with an epoch year, such as 1900. All times thereafter
 must be in or after the epoch year. Epochs closer to the times
 in use do not increase performance, but they permit smaller
 epoch values and thus increase compatibility in 32-bit systems
 or for times far into the future.

 Also initialize with an up-to-date official IETF/IERS leap second
 file, typically named 'leap-seconds.list'. At the time of writing,
 this file was available here:
 http://www.ietf.org/timezones/data/leap-seconds.list

 Call adjust() to bring a smart_tm back into valid range
 after changing a field such as minutes or seconds.

 Example initialization call followed by creation of a smart_tm
 and printing of that time as well as the time advanced by 200 seconds:
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
