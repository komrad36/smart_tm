/*******************************************************************
*   FileIO.h
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

#ifndef FILEIO_H
#define FILEIO_H

#include <fstream>
#include <string>

#define DO_NOT_SKIP_WHITESPACE (true)

bool getNextLine(std::ifstream& is, std::string& line);

#endif