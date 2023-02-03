/***************************************************************************
 *   Copyright (C) 2017 by DTU (Christian Andersen)   *
 *   jca@elektro.dtu.dk   *
 *
 *  $Rev: 1960 $
 *  $Id: ulogfile.h 1960 2012-07-25 08:40:55Z jca $
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef ULOGFILE_H
#define ULOGFILE_H

#include <stdio.h>
#include <mutex>
#include "utime.h"

#define MAX_FILENAME_SIZE 200


/**
A few functions to open and close a logfile and to store the name and status of the file

	@author Christian Andersen <chrand@mail.dk>
*/
class ULogFile
{
public:
  /**
   * Constructor */
  ULogFile();
  /**
   * DEstructor */
  ~ULogFile();
  /**
   * flag to flush all open logfiles */
  static int timeToFlush;
  int flushed;
  /**
   * Open logfile with default name, i.e. resource name */
  bool openLog();
  /** Set logfile pre-name, and optionally the extension as other than .log.
   * for a plug-in the logname should typically be the plug-in name.
   * \param resName a pointer to a valid character string.
   * \param ext an optional alternative extension  
   * \param logPath sets a pointer to a log-path sored elsewhere */
  void setLogName(const char * resName, const char * extension = "txt");
  /** Set logfile path
   * \param logPath sets a pointer to a log-path stored elsewhere */
  void setLogPath(const char * path);
  /**
  Open or close log in one call
  \param doOpen opens log if true, else close.
  \returns true of open was an succes. */
  bool openLog(bool doOpen);
  /**
   *  Open logfile with default name name.txt in the dataPath */
//   bool openLog(const char * logName);
  /**
  Close logfile for variable modifications */
  void closeLog();
  /**
  Get logfile name - full logname including path. */
  inline const char * getLogFileName()
  { return logFileName; };
  /**
  Get log name, that is the name of the logfile without the path and the '.log' extension. */
  inline const char * getLogName()
  { return logName; };
  /**
  Is logfile open - the file itself */
  inline bool isOpen()
  { 
    return logFile != NULL;
  };
  /**
   * Save this string to the logfile.
   * The string is preceded with a timestamp and terminated with a 'newline'.
   * \param logString is the data that need to be logged (with a timestamp) */
  void toLog(UTime t, const char * logString);
  /**
   * Should any 'toLog' be flushed to disk after writing to file?
   * default is true.
   * \param doFlush if true the toLog will do a fflush with each toLog command. */
  void setLogFlush(bool doFlush)
  { logFlush = doFlush; };
  /**
   * Get file handle itself for own printing */
  FILE * getF();
  /**
   * Set logtime for subsequent toLog calls
   * \param timestamp to use */
  void setLogTime(UTime timestamp)
  {
    logTime = timestamp;
    logTimeUse = true;
  };
  /**
   * Set flag for use of specified logTime rather than current time
   * \param value when true latest specified logTime is used, else current time is used */
  void useLogTime(bool value)
  {
    logTimeUse = value;
  };
  /**
   * REname this file (or file mask) to include date and time, and add an extra 'g' to the filename
   * \param name is the filename, either in current directory, or with full path, or as a mask
   * The functopn uses stat to get the file modified time (mtime), and constructs a
   * string like yyyymmdd_hhmmss (year-month-day_hour-minute-second), and inserts this
   * before the final '.filetype' and appends a 'g' at the end of the file.
   * This should allow a '*.log' specification to rename all logfiles, and not
   * catch the already renamed files.
   * \returns true if successful and false, if no sourcefile or rename failed. */
  bool logRename(const char * name);
  /**
   * Flush unsaved data to disk - if file is open */
  void doFlush();
  /**
   * When using the file-handle it this file lock-unlock should be used too, to avoid conflicts */
  void logLock()
  { logFileLock.lock(); };
  /**
   * When using the file-handle it this file lock-unlock should be used too, to avoid conflicts */
  void logUnlock()
  { logFileLock.unlock(); };

protected:
  /**
   * \brief add a number to the logfile name,
   * That is if the logName is 'svs' and number is 2 (and logPath is '/rhome/demo'),
   * then logfilename is set to '/rhome/demo/svs_2.log'.
   * The purpose is to have an alternative, if the file renaming fails
   * (if the file system is locked or something).
   * \param number is the integer number to add. */
  void setLogNameNumber(int number);
  
  
protected:
  /**
   * additional data path */
  const char * dataPath = "";
  /**
   * Log name, i.e. the resource name for the owner of the logfile,
   * e.g. "abc" for a log file name of "abc.log" */
  char logName[MAX_FILENAME_SIZE] = "uninitialized";
  /**
   * file extension */
  const char * ext = "txt";
  /**
    * Logfile for variable updates */
  FILE * logFile;
  /**
   * is file attempted open, and should be
   * opened when write is attempted. */
  //bool shouldBeOpen;
  /**
    * Filename for logfile including path */
  char logFileName[MAX_FILENAME_SIZE];
  /**
    * Resource lock for variable logfile */
  std::mutex logFileLock;
  /**
   * Should any 'toLog' be flushed to disk after writing to file?
   * \param doFlush if true the toLog will do a fflush. */
  bool logFlush;
  /**
   * Use a specific logtime set by setTime, else use current time from a gettimeofday() call */
  bool logTimeUse;
  /**
   * A specific logtime */
  UTime logTime;
  /**
   * count of open failures since lase success */
  int failCnt;

private:
  /**
  Buffer for logname */
//  static const int MLNL = 42;
//  char logNameBuffer[MLNL];
};

/**
A lockfile with a lock */
// class ULLogFile : public ULogFile, public ULock
// {
// };


#endif
