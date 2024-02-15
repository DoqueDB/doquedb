// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriver.cpp -- LogicalFile call it to get the instance of LogicalInterface
// 
// Copyright (c) 2000, 2002, 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Record2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Record2/FileDriver.h"
#include "Record2/LogicalInterface.h"

#include "Exception/NotSupported.h"

#include "LogicalFile/FileDriverTable.h"
#include "LogicalFile/FileID.h"

#include "Os/AutoCriticalSection.h"
#include "Os/Library.h"

#ifndef SYD_DLL
#include "Record/FileDriver.h"
#endif

_SYDNEY_USING

_SYDNEY_RECORD2_USING

namespace
{
	//
	//	VARIABLE local
	//	_$$::_OldLibName
	//
	ModUnicodeString _OldLibName("SyDrvRcd");

	//
	//	VARIABLE local
	//	_$$::_DBGetName
	//
	ModUnicodeString _DBGetName("DBGetFileDriver");
}

//
//	FUNCTION public
//	Record2::FileDriver::FileDriver -- Constructor
//
//	NOTES
//	Constructor
//
//	ARGUMENTS
//	none
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	none
//
FileDriver::FileDriver()
	: LogicalFile::FileDriver(),
	  m_pOldFileDriver(0)
{
}

//
//	FUNCTION public
//	Record2::FileDriver::~FileDriver -- destructor
//
//	NOTES
//	destructor
//
//	ARGUMENTS
//	none
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	none
//
FileDriver::~FileDriver()
{
	delete m_pOldFileDriver, m_pOldFileDriver = 0;
}

//
//	FUNCTION public void
//	Record2::FileDriver::initialize --
//
//	NOTES
//	
//	ARGUMETNS
//		none
//	RETURN
//		none
//	EXCEPTIONS
//
void
FileDriver::initialize()
{
	Record2::LogicalInterface::initialize();
}

//
//	FUNCTION public void
//	Record2::FileDriver::terminate --
//
//	NOTES
//
//	ARGUMETNS
//		none
//	RETURN
//		none
//	EXCEPTIONS
//
void
FileDriver::terminate()
{
	Record2::LogicalInterface::terminate();
	if (m_pOldFileDriver) m_pOldFileDriver->terminate();
}

//
//	FUNCTION public
//	Record2::FileDriver::attachFile --
//	
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID&	cFileOption_
//
//	RETURN
//	LogicalFile::File*
//
//	EXCEPTIONS
//	YET!
//
LogicalFile::File*
FileDriver::attachFile(const LogicalFile::FileID&	cFileOption_) const
{
	if (FileID::checkVersion(cFileOption_) == true)
		return new LogicalInterface(cFileOption_);
	return getOld()->attachFile(cFileOption_);
}

//
//	FUNCTION public
//	Record2::FileDriver::attachFile --
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::File*	pFile_
//
//	RETURN
//	LogicalFile::File*
//
//	EXCEPTIONS
//	YET!
//
LogicalFile::File*
FileDriver::attachFile(const LogicalFile::File*	pFile_) const
{
	if (FileID::checkVersion(pFile_->getFileID()) == true)
		return new LogicalInterface(pFile_->getFileID());
	return getOld()->attachFile(pFile_);
}

//
//	FUNCTION public
//	Record2::FileDriver::detachFile --
//
//	NOTES
//
//	ARGUMENTS
//	LogicalFile::File*	pFile_
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	none
//
void
FileDriver::detachFile(LogicalFile::File* pFile_) const
{
	delete pFile_;
}

//
//	FUNCTION public
//	Record2::FileDriver::getDriverID --
//
//	NOTES
//
//	ARGUMENTS
//	none
//
//	RETURN
//	int
//
//	EXCEPTIONS
//	none
//
int
FileDriver::getDriverID() const
{
	return LogicalFile::FileDriverID::Record;
}

//
//	FUNCTION public
//	Record2::FileDriver::getDriverName --
//
//	NOTES
//
//	ARGUMENTS
//	none
//
//	RETURN
//	ModString
//
//	EXCEPTIONS
//	none
//
ModString
FileDriver::getDriverName() const
{
	return LogicalFile::FileDriverName::Record;
}

//
//	FUNCTION private
//	Record2::FileDriver::getOld -- Get older version FileDriver
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	LogicalFile::FileDriver*
//	   older version FileDriver
//
//	EXCEPTIONS
//
LogicalFile::FileDriver*
FileDriver::getOld() const
{
	Os::AutoCriticalSection cAuto(m_cLock);
	if (m_pOldFileDriver == 0)
	{
#ifdef SYD_DLL
		// Load old version library
		Os::Library::load(_OldLibName);

		// get function pointer of DBGetFileDriver
		LogicalFile::FileDriver* (*f)(void);
		f = (LogicalFile::FileDriver*(*)(void))
			Os::Library::getFunction(_OldLibName, _DBGetName);

		// Get file driver
		m_pOldFileDriver = (*f)();
#else
		// Get old version file driver
		m_pOldFileDriver = new Record::FileDriver;
#endif
		// Initialize
		m_pOldFileDriver->initialize();
	}
	return m_pOldFileDriver;
}

//
//	Copyright (c) 2000, 2002, 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
