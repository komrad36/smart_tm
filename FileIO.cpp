/*******************************************************************
*   FileIO.cpp
*   Smart Time System
*	Kareem Omar
*	Student Researcher, CSPAR - UAH Class of 2017
*
*	6/4/2015
*   This program is entirely my own work.
*
*	Use and modification by NASA, CSPAR, and UAH is fully and
*	freely permitted with credit to original developer.
*******************************************************************/

// This module provides platform-independent, fast parsing of ASCII
// text files one line at a time, avoiding the newline troubles of
// the system getline() call. It is also able to retrive data from
// the last line of a file even if the file does not have a trailing
// newline.

#include "FileIO.h"

std::istream& safeGetline(std::istream& is, std::string& line) {
	line.clear();

	// streambuf for speed
	// requires stream sentry to guard buffer
	std::istream::sentry se(is, DO_NOT_SKIP_WHITESPACE);
	std::streambuf* sb = is.rdbuf();

	short c;

	for (;;) {
		c = sb->sbumpc();
		switch (c) {
		case '\n':
			return is;
		case '\r':
			if (sb->sgetc() == '\n')
				sb->sbumpc();
			return is;
		case EOF:
			// Also handle the case when the last line has no line ending
			if (line.empty())
				is.setstate(std::ios::eofbit);
			return is;
		default:
			line += static_cast<char>(c);
		}
	}
}

bool getNextLine(std::ifstream& is, std::string& line) {
	bool more = true;

	while (more) {
		safeGetline(is, line);

		if (!is.eof()) {
			if (line.empty() || line[0] == '#') {
				continue;
			}
			more = false;
		}
		else {
			line.clear();
			return false;
		}
	}
	return true;
}