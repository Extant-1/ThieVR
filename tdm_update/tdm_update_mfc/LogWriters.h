/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod Updater (http://www.thedarkmod.com/)
 
 $Revision: 6031 $ (Revision of last commit) 
 $Date: 2014-06-12 22:34:47 +0000 (Thu, 12 Jun 2014) $ (Date of last commit)
 $Author: taaaki $ (Author of last commit)
 
******************************************************************************/

#pragma once

#include <string>
#include <stdexcept>
#include <boost/shared_ptr.hpp>

namespace tdm
{

class FileOpenException : 
    public std::runtime_error 
{
public:
    FileOpenException() : std::runtime_error("File open failure") { }
};

class FileLogWriter :
	public tdm::ILogWriter
{
private:
	FILE* _logFile;
public:
	FileLogWriter(const std::string& path) :
		_logFile(NULL)
	{
		_logFile = fopen(path.c_str(), "w");
		if (_logFile == NULL) {
			throw FileOpenException();
		}

        time_t rawtime;
		time(&rawtime);

		fputs("TDM Update Logfile created: ", _logFile);
		fputs(asctime(localtime(&rawtime)), _logFile);
		fputs("\n", _logFile);
	}

	~FileLogWriter()
	{
		time_t rawtime;
		time(&rawtime);

		fputs("TDM Update Logfile closed: ", _logFile);
		fputs(asctime(localtime(&rawtime)), _logFile);
		fputs("\n", _logFile);

		fclose(_logFile);
		fflush(_logFile);
	}

	void WriteLog(LogClass lc, const std::string& str)
	{
		// Don't write progress stuff into the logfile
		if (lc != LOG_PROGRESS)
		{
			fputs(str.c_str(), _logFile);
			fflush(_logFile);
		}
	}
};

void RegisterLogWriters()
{
	boost::shared_ptr<tdm::FileLogWriter> logWriter(new tdm::FileLogWriter("tdm_update.log"));
    tdm::TraceLog::Instance().Register(logWriter);
}

} // namespace