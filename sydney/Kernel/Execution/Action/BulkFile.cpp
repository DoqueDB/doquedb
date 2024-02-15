// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BulkFile.cpp --
// 
// Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Execution::Action";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Execution/Action/BulkFile.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "Exception/FileAlreadyExisted.h"
#include "Exception/FileNotFound.h"
#include "Exception/ModLibraryError.h"
#include "Exception/PermissionDenied.h"

#include "Opt/Algorithm.h"
#include "Opt/Configuration.h"

#include "Os/Memory.h"

#include "ModOsDriver.h"
#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

// FUNCTION public
//	execution::Action::BulkFile::BulkFile -- constructors
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cPath_
//	bool bIsRead_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

BulkFile::
BulkFile(const ModUnicodeString& cPath_,
		 bool bIsRead_)
	: Super(cPath_),
	  m_pBuffer(0),
	  m_pTop(0),
	  m_pTail(0),
	  m_iBufferSize(Opt::Configuration::getBulkBufferSize()),
	  m_bIsRead(bIsRead_),
	  m_bIsEof(false),
	  m_iTailPosition(0),
	  m_cParentPath()
{}

// FUNCTION public
//	execution::Action::BulkFile::~BulkFile -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

BulkFile::
~BulkFile()
{
	try {
		terminate();
	} catch (...) {
		// ignore exceptions in destructor
		;
	}
}

// FUNCTION public
//	execution::Action::BulkFile::read -- read next block
//
// NOTES
//
// ARGUMENTS
//	const char* pRest_ /* = 0 */
//	
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
BulkFile::
read(const char* pRest_ /* = 0 */)
{
	open();

	; _SYDNEY_ASSERT(m_pTop);
	char* pTop = m_pTop;
	int iSize = m_iBufferSize;

	if (pRest_ && (pRest_ - m_pTop) < m_iBufferSize) {
		// move rest data to top
		int iRestSize = m_iBufferSize - static_cast<int>(pRest_ - m_pTop);

		// if no more data or rest data is larger than half of buffer, do nothing
		if (m_bIsEof || (iRestSize > m_iBufferSize / 2)) {
			return pRest_;
		}

		Os::Memory::move(m_pTop, pRest_, iRestSize);
		iSize -= iRestSize;
		pTop += iRestSize;
	}

	int iReadSize = Super::read(pTop, iSize);
	m_pTail = pTop + iReadSize;
	m_iTailPosition += iReadSize;

	if (iReadSize < iSize) {
		// reached to the end of file
		m_bIsEof = true;
	}

	return m_pTop;
}

// FUNCTION public
//	execution::Action::BulkFile::write -- write char string
//
// NOTES
//
// ARGUMENTS
//	const char* pData_
//	int iLength_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkFile::
write(const char* pData_, int iLength_)
{
	open();

	; _SYDNEY_ASSERT(m_pTop);
	; _SYDNEY_ASSERT(m_pTail);

	if (pData_ == 0) {
		// 0 means end of writing
		flush();

	} else {
		while (iLength_) {
			int iPutLength = MIN(iLength_,
								 m_iBufferSize - static_cast<int>(
									 m_pTail - m_pTop));
			Os::Memory::copy(m_pTail, pData_, iPutLength);
			iLength_ -= iPutLength;
			m_pTail += iPutLength;
			pData_ += iPutLength;

			if (m_pTail - m_pTop == m_iBufferSize) {
				(void) Super::write(m_pTop, m_iBufferSize);
				m_pTail = m_pTop;
			}
		}
	}
}

// FUNCTION public
//	execution::Action::BulkFile::putData -- put data from file (for error file)
//
// NOTES
//
// ARGUMENTS
//	BulkFile& cFile_
//	ModFileOffset iTop_
//	ModFileOffset iTail_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkFile::
putData(BulkFile& cFile_, ModFileOffset iTop_, ModFileOffset iTail_)
{
	cFile_.open();
	open();

	// seek file
	cFile_.seek(iTop_, ModFile::seekSet);

	ModFileOffset iTotalSize = iTail_ - iTop_;

	while (iTotalSize > 0) {
		int iSize = static_cast<int>(
			MIN(static_cast<ModFileOffset>(m_iBufferSize), iTotalSize)); // ??
		// read from file
		int iReadSize = cFile_.Super::read(m_pTop, iSize);
		// write to file
		Super::write(m_pTop, iReadSize);

		if (iReadSize < iSize ) break;

		iTotalSize -= iReadSize;
	}
}

// FUNCTION public
//	execution::Action::BulkFile::flush -- flush data
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkFile::
flush()
{
	if (m_pTail > m_pTop) {
		(void) Super::write(m_pTop, static_cast<int>(m_pTail - m_pTop));
	}
	m_pTail = m_pTop;
}

// FUNCTION public
//	execution::Action::BulkFile::isEndOfFile -- all data has read?
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkFile::
isEndOfFile()
{
	return m_bIsEof;
}

// FUNCTION public
//	execution::Action::BulkFile::reset -- reset iteration
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkFile::
reset()
{
	// reset the seek position
	if (isOpened()) {
		seek(0, ModFile::seekSet);
	}
	m_bIsEof = false;
	m_pTop = syd_reinterpret_cast<char*>(m_pBuffer);
	m_pTail = m_pTop;
	m_iTailPosition = 0;
}

// FUNCTION public
//	execution::Action::BulkFile::setPath -- change path name
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cPath_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkFile::
setPath(const ModUnicodeString& cPath_)
{
	// reset object
	reset();

	// close file if opened
	if (isOpened()) {
		close();
	}

	// change the path name
	/* Super::*/pathName = cPath_;

	// clear parent path cache
	m_cParentPath.clear();
}

// FUNCTION public
//	execution::Action::BulkFile::getFullPath -- obtain full path name from the parent path of the file
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cPath_
//	ModUnicodeString& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkFile::
getFullPath(const ModUnicodeString& cPath_, ModUnicodeString& cResult_)
{
	if (ModOsDriver::File::isFullPathName(cPath_)) {
		// no need to convert
		cResult_ = cPath_;
	} else {
		setParentPath();
		; _SYDNEY_ASSERT(m_cParentPath.getLength() > 0);
		ModOsDriver::File::getFullPathName(m_cParentPath, cPath_, cResult_);
	}
}

// FUNCTION public
//	execution::Action::BulkFile::open -- open
//
// NOTES
//
// ARGUMENTS
//	bool bThrow_ /* = true */
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
BulkFile::
open(bool bThrow_ /* = true */)
{
	bool bResult = true;

	if (isOpened()) {
		return true;
	}

	try {
		// call superclass's implementation
		Super::open((m_bIsRead ? readMode : writeMode),
					(m_bIsRead ? 0 : (createFlag | truncateFlag)));

		m_iTailPosition = 0;

	} catch (ModException& e) {

		SydInfoMessage << "Bulk file("
					   << getFullPathNameW()
					   << "): "
					   << Exception::ModLibraryError(moduleName, srcFile, __LINE__, e)
					   << ModEndl;

		if (bThrow_) {
			switch (e.getErrorNumber()) {
			case ModOsErrorFileNotFound:
				{
					_SYDNEY_THROW1(Exception::FileNotFound, getFullPathNameW());
				}
			case ModOsErrorPermissionDenied:
				{
					_SYDNEY_THROW1(Exception::PermissionDenied, getFullPathNameW());
				}
			case ModOsErrorFileExist:
				{
					_SYDNEY_THROW1(Exception::FileAlreadyExisted, getFullPathNameW());
				}
			default:
				{
					break;
				}
			}
			_SYDNEY_THROW1(Exception::ModLibraryError, e);

		} else {
			// ignore the error
			ModErrorHandle::reset();

			bResult = false;
		}
	}
	return bResult;
}

// FUNCTION public
//	execution::Action::BulkFile::initialize -- inilialize
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkFile::
initialize()
{
	// if the file is created for writing, create parent directry
	if (!m_bIsRead) mkdir();
	initializeBuffer();
}

// FUNCTION public
//	execution::Action::BulkFile::initializeBuffer -- initialize buffer object
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkFile::
initializeBuffer()
{
	if (m_pBuffer == 0) {
		// create buffer object
		m_pBuffer = Os::Memory::allocate(m_iBufferSize);
		Os::Memory::reset(m_pBuffer, m_iBufferSize);
	}
	m_pTop = syd_reinterpret_cast<char*>(m_pBuffer);
	m_pTail = m_pTop;
}

// FUNCTION public
//	execution::Action::BulkFile::terminate -- terminating
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkFile::
terminate()
{
	if (isOpened()) {
		close();
	}
	Os::Memory::free(m_pBuffer);
	; _SYDNEY_ASSERT(m_pBuffer == 0);
	m_pTop = 0;
	m_pTail = 0;
}

// FUNCTION public
//	execution::Action::BulkFile::mkdir -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkFile::
mkdir()
{
	setParentPath();
	if (ModOsDriver::File::isNotFound(m_cParentPath) == ModTrue) {
		ModOsDriver::File::mkdir(m_cParentPath,
								 ownerReadPermission
								 | ownerWritePermission
								 | ownerExecutePermission
								 | groupReadPermission
								 | groupWritePermission
								 | groupExecutePermission
								 | otherReadPermission
								 | otherWritePermission
								 | otherExecutePermission,
								 ModTrue /* recursive */);
	}
}

// FUNCTION public
//	execution::Action::BulkFile::getTopPosition -- get current position of buffer top
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModFileOffset
//
// EXCEPTIONS

ModFileOffset
BulkFile::
getTopPosition()
{
	return m_iTailPosition - (m_pTail - m_pTop);
}

// FUNCTION private
//	execution::Action::BulkFile::setParentPath -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkFile::
setParentPath()
{
	if (m_cParentPath.getLength() == 0) {
		(void)ModOsDriver::File::getParentPathName(getFullPathNameW(), m_cParentPath);
	}
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
