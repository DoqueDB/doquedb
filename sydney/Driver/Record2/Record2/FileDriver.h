// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriver.h -- LogicalFile call it to get the instance of LogicalInterface
// 
// Copyright (c) 2000, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD2_FILEDRIVER_H
#define __SYDNEY_RECORD2_FILEDRIVER_H

#include "Record2/Module.h"

#include "LogicalFile/FileDriver.h"

#include "Os/CriticalSection.h"

_SYDNEY_BEGIN

_SYDNEY_RECORD2_BEGIN

//
//	CLASS
//	Record2::FileDriver -- LogicalFile call it to get the instance of LogicalInterface
//
//	NOTES
//	LogicalFile call it to get the instance of LogicalInterface
//
class SYD_RECORD2_FUNCTION_TESTEXPORT FileDriver : public LogicalFile::FileDriver
{
public:

	// Constructor
	FileDriver();

	// Destructor
	~FileDriver();

	// The record file driver is initialized. 
	void initialize();

	// The record file driver is postprocessed.
	void terminate();

	// Get the instance of LogicalInterface
	LogicalFile::File* attachFile(const LogicalFile::FileID&	cFileOption_) const;
	LogicalFile::File* attachFile(const LogicalFile::File*	pFile_) const;

	// delete the instance of LogicalInterface
	void detachFile(LogicalFile::File*	pFile_) const;

	// get driver identify
	int getDriverID() const;

	// get the driver name
	ModString getDriverName() const;

private:
	// The parents class and the class name evade operator = problem when it is the same. 
	FileDriver&	operator= (const FileDriver&	cObject_);

	// Get old version driver
	LogicalFile::FileDriver* getOld() const;

	// Mutual exclusion for following data member
	mutable Os::CriticalSection m_cLock;
	// Old version driver
	mutable LogicalFile::FileDriver* m_pOldFileDriver;
};

_SYDNEY_RECORD2_END // end of namespace Record2

_SYDNEY_END

#endif // __SYDNEY_RECORD2_FILEDRIVER_H

//
//	Copyright (c) 2000, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
