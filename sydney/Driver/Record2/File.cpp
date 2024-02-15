// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.cpp -- manage physical file hander and execute normal file operation
// 
// Copyright (c) 2007, 2006, 2023 Ricoh Company, Ltd.
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

#include "Checkpoint/Database.h"
#include "LogicalFile/Estimate.h"
#include "Exception/FakeError.h"
#include "Exception/IllegalFileAccess.h"

#include "Record2/Debug.h"
#include "Record2/File.h"

_SYDNEY_USING

_SYDNEY_RECORD2_USING

namespace
{
	// Page managenet type of PhysicalFile for FixedFile and VariableFile
	const PhysicalFile::Type _eFixedPhysicalFileType = PhysicalFile::PageManageType;
	const PhysicalFile::Type _eVariablePhysicalFileType = PhysicalFile::DirectAreaType;

	// File name of FixedFile and VariableFile
	const ModUnicodeString _cstrFixedFileName(_TRMEISTER_U_STRING("Fixed"));
	const ModUnicodeString _cstrVariableFileName(_TRMEISTER_U_STRING("Variable"));

} // namespace


//constructor
File::
File(const Trans::Transaction& cTrans_, 
		   const FileID& cFileID_,
		   const bool bIsVariable_)
	: m_cTrans(cTrans_),
	  m_cFileID(cFileID_),
      m_bIsVariable(bIsVariable_),
	  m_bBatch(false),
	  m_pFile(0)
{
	//set storategy
	if(m_bIsVariable) setStorategy(_eVariablePhysicalFileType, _cstrVariableFileName);
	else setStorategy(_eFixedPhysicalFileType, _cstrFixedFileName);
}

//	FUNCTION public
//	Record2::File::~File -- destructor
//
//	NOTES
//
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

File::
~File()
{
	if (isAttached())
		detachFile();
}

//	FUNCTION public
//	Record2::File::attachFile -- attach the PhysicalFile for FixFile or VariableFile
//
//	NOTES
//
//	ARGUMENTS
//		const bool bBatch_ = false
//			isBatch
//
//	RETURN
//		None
//
//	EXCEPTIONS
void
File::
attachFile(const bool bBatch_)
{
	; _SYDNEY_ASSERT(!m_pFile);
	m_pFile = PhysicalFile::Manager::attachFile(m_cStorageStrategy, 
				m_cBufferingStrategy, m_cFileID.getLockName());
	; _SYDNEY_ASSERT(m_pFile);

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Attach file: "
			<< getPath()
			<< ModEndl;
#endif

	m_bBatch = bBatch_;
}

//	FUNCTION public
//	Record2::File::detachFile -- detach the PhysicalFile for FixFile or VariableFile
//
//	NOTES
//
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS
void
File::
detachFile()
{
	if (m_pFile) 
	{
		PhysicalFile::Manager::detachFile(m_pFile);
		; _SYDNEY_ASSERT(!m_pFile);

		m_bBatch = false;

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Detach file: "
			<< getPath()
			<< ModEndl;
#endif
	}
}

//	FUNCTION public
//	Record2::File::isAttached -- is the PhysicalFile attached?
//
//	NOTES
//
//	ARGUMENTS
//		None
//
//	RETURN
//		It is true if it is Atattied. 
//
//	EXCEPTIONS
bool
File::
isAttached() const
{
	return (m_pFile != 0);
}

//	FUNCTION public
//	Record2::File::isMounted -- Is the mount done?
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	cTrans_
//			Whether the mount is done is examined. 
//			Transaction descriptor of transaction
//
//	RETURN
//		true
//			The mount is done. 
//		false
//			The mount isn't done. 
//
//	EXCEPTIONS

bool
File::
isMounted(const Trans::Transaction& cTrans_) const
{
	return (isAttached()) ?
		m_pFile->isMounted(cTrans_) : 
		FileCommon::AutoPhysicalFile(
			m_cStorageStrategy, m_cBufferingStrategy,
			m_cFileID.getLockName())->isMounted(cTrans_);
}

//	FUNCTION public
//	Record2::File::isAccessible --
//		exam whether OS file exists
//
//	NOTES
//
//	ARGUMENTS
//		bool bForce_
//			true
//			The existence of OS file that composes is actually examined. 
//			When false or it is not specified
//			The existence of OS file that composes is examined if there is a necessity. 
//
//	RETURN
//		true
//			It exists. 
//		false
//			It isn't exist.
//
//	EXCEPTIONS
bool
File::
isAccessible(bool bForce_) const
{
	return (isAttached()) ?
		m_pFile->isAccessible(bForce_) : 
		FileCommon::AutoPhysicalFile(
			m_cStorageStrategy, m_cBufferingStrategy,
			m_cFileID.getLockName())->isAccessible(bForce_);
}

//	FUNCTION public
//	Record2::File::getAutoFile --
//
//	NOTES
//
//	ARGUMENTS
//		None
//
//	RETURN
//		
//
//	EXCEPTIONS
//FileCommon::AutoPhysicalFile&
//File::
//getAutoFile() const
//{
//	return FileCommon::AutoPhysicalFile(
//			m_cStorageStrategy, m_cBufferingStrategy,
//			m_cFileID.getLockName());
//}

//	FUNCTION public
//	Record2::File::getPathPart -- get the file path part for FixFile or VariableFile 
//
//	NOTES
//
//	ARGUMENTS
//		None
//
//	RETURN
//		
//
//	EXCEPTIONS

const ModUnicodeString&
File::
getPathPart() const
{
	return m_bIsVariable ? _cstrVariableFileName : _cstrFixedFileName;
}

//	FUNCTION public
//	Record::FileBase::getPath -- get the file path for master file for version
//
//	NOTES
//
//	ARGUMENTS
//		None
//
//	RETURN
//		
//
//	EXCEPTIONS

const ModUnicodeString&
File::
getPath() const
{
	return m_cStorageStrategy.m_VersionFileInfo._path._masterData;
}

//	FUNCTION public
//	Record2::File::getOverhead --
//
//	NOTES
//
//	ARGUMENTS
//		None
//
//	RETURN
//		Overhead
//
//	EXCEPTIONS

double
File::
getOverhead() const
{
	if(m_bIsVariable || !isMounted(m_cTrans))
		return 0.0;

	// It is the same as ?
	// ProcessCost additionally made a cost for one in mounting Btree2. 
	return getProcessCost();
}

//	FUNCTION public
//	Record2::File::getProcessCost --
//
//	NOTES
//
//	ARGUMENTS
//		None
//
//	RETURN
//		Time that hangs to processing for one
//
//	EXCEPTIONS

double
File::
getProcessCost(ModInt64 iCount_) const
{
	if (!isMounted(m_cTrans))
		return 0.0;

	const double dCostFileToMemory =	static_cast<const double>(
		LogicalFile::Estimate::getTransferSpeed(LogicalFile::Estimate::File));

	if(m_bIsVariable)
	{
		// The average size of the object is obtained. 
		return static_cast<double>(getSize()) / (iCount_ ? iCount_ : 1) / dCostFileToMemory;
	}
	else
	{
		// One obtained cost is almost equal to the cost obtained by one page.
		return static_cast<double>(m_cStorageStrategy.m_VersionFileInfo._pageSize) /
			m_cFileID.getObjectNumberPerPage() / dCostFileToMemory;
	}
}


//	FUNCTION public
//	Record2::File::getSize -- get the size of FixFile or VariableFile
//
//	NOTES
//
//	ARGUMENTS
//		None
//
//	RETURN
//		Size of file
//
//	EXCEPTIONS

ModUInt64 
File::
getSize() const
{
	if (isMounted(m_cTrans)) 
	{
		; _SYDNEY_ASSERT(m_pFile);
		return m_pFile->getSize();
	}
	return 0;
}

//	FUNCTION public
//	Record2::File::create --
//
//	NOTES
//
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
File::
create()
{
	m_cStorageStrategy.m_VersionFileInfo._mounted = true;
}

//	FUNCTION public
//	Record2::File::destroy --
//
//	NOTES
//
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
File::
destroy()
{
	enum 
	{
		None,
		File
	} eStatus = None;

	AutoAttachFile file(*this);
	; _SYDNEY_ASSERT(m_pFile);

	// Anyway, it deletes it confirming neither the presence of the 
	// mount nor the presence of the existence of substance. 
	//
	//Attention	Otherwise, it manages in the subordinate position layer. 
	// Information is not maintained. 
	try 
	{
		// A physical file that is substance is annulled. 

		m_pFile->destroy(m_cTrans);
		eStatus = File;

		// The sub directory is removed. 

		ModOsDriver::File::rmAll(getPath(), ModTrue);

		_SYDNEY_FAKE_ERROR("Record2::File::destroy",Exception::IllegalFileAccess(moduleName, srcFile, __LINE__));
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		try 
		{
			// It tries again. 
			switch (eStatus) 
			{
				case None:
				default:
					m_pFile->destroy(m_cTrans);// thru
				case File:
					ModOsDriver::File::rmAll(getPath(), ModTrue);
			}
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			// It makes it because it failed as a trial again to cannot use. 
			Checkpoint::Database::setAvailability(
				m_cFileID.getLockName(), false);
		}
		_SYDNEY_RETHROW;
	}

	m_cStorageStrategy.m_VersionFileInfo._mounted = false;
}

//	FUNCTION public
//	Record2::File::mount --
//
//	NOTES
//
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
File::
mount()
{
	AutoAttachFile file(*this);

	if (!isMounted(m_cTrans)) 
	{

		// If the mount is not done, the mount is done. 

		; _SYDNEY_ASSERT(m_pFile);
		m_pFile->mount(m_cTrans);

		// It is recorded that the mount was done. 

		m_cStorageStrategy.m_VersionFileInfo._mounted = true;
	}
}

//	FUNCTION public
//	Record2::File::unmount --
//
//	NOTES
//
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
File::
unmount()
{
	// Anyway, it Ammaunts it confirming neither the presence of the mount 
	// nor the presence of the existence of substance. 
	//
	//【Attention】	Otherwise, it manages in the subordinate position layer. 
	// Information is not maintained. 

	AutoAttachFile file(*this);

	; _SYDNEY_ASSERT(m_pFile);
	m_pFile->unmount(m_cTrans);

	m_cStorageStrategy.m_VersionFileInfo._mounted = false;
}

//	FUNCTION public
//	Record2::File::flush --
//
//	NOTES
//
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
File::
flush()
{
	AutoAttachFile file(*this);

	if (isMounted(m_cTrans)) {
		; _SYDNEY_ASSERT(m_pFile);
		m_pFile->flush(m_cTrans);
	}
}

//	FUNCTION public
//	Record2::File::startBackup --
//
//	NOTES
//
//	ARGUMENTS
//		const bool bRestorable_
//			To return it to the version seen for true at that time, it backs up. 
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
File::
startBackup(const bool bRestorable_)
{
	AutoAttachFile file(*this);

	if (isMounted(m_cTrans)) 
	{
		; _SYDNEY_ASSERT(m_pFile);
		m_pFile->startBackup(m_cTrans, bRestorable_);
	}
}

//	FUNCTION public
//	Record2::File::endBackup --
//
//	NOTES
//
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
File::
endBackup()
{
	AutoAttachFile file(*this);

	if (isMounted(m_cTrans)) 
	{
		; _SYDNEY_ASSERT(m_pFile);
		m_pFile->endBackup(m_cTrans);
	}
}

//	FUNCTION public
//	Record2::File::recover --
//
//	NOTES
//
//	ARGUMENTS
//		const Trans::TimeStamp& cPoint_
//			Point that recovers trouble
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
File::
recover(const Trans::TimeStamp& cPoint_)
{
	AutoAttachFile file(*this);

	if (isMounted(m_cTrans)) 
	{
		; _SYDNEY_ASSERT(m_pFile);
		m_pFile->recover(m_cTrans, cPoint_);

		if (!isAccessible())
		{
			// Result of recovery
			// Because OS file that was substance did not exist, 
			// the subdirectory is deleted. 
			ModOsDriver::File::rmAll(getPath(), ModTrue);
		}
	}
}

//	FUNCTION public
//	Record2::File::restore --
//
//	NOTES
//
//	ARGUMENTS
//		const Trans::TimeStamp& cPoint_
//			Point that recovers trouble
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
File::
restore(const Trans::TimeStamp& cPoint_)
{
	AutoAttachFile file(*this);

	if (isMounted(m_cTrans)) 
	{
		; _SYDNEY_ASSERT(m_pFile);
		m_pFile->restore(m_cTrans, cPoint_);
	}
}

//	FUNCTION public
//	Record2::File::move --
//
//	NOTES
//
//	ARGUMENTS
//	 bool bUndo_ = false
//		Because it is move for error processing for true, no error processing. 
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
File::
move(bool bUndo_)
{
	// It moves making a new path name. 
	//
	//【Attention】	Because the reference that getPath returns in move is changed
	// After it copies it, the path name before it moves calls move. 
	Os::Path oldPath(getPath());
	Os::Path newPath(m_cFileID.getDirectoryPath());
	newPath.addPart(getPathPart());

	move(oldPath, newPath, bUndo_);
}


//	FUNCTION public
//	Record2::File::sync -- Synchronization is taken. 
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	cTrans_
//			Transaction descriptor of transaction that takes synchronization
//		bool&				bInComplete_
//			true
//				It has the file in the processing of the same period of this time. 
//				There is leaving by processing it partially of the object. 
//			false
//				It has the file in the processing of the same period of this time. 
//				The object has been completely processed. 
//
//				Whether the file was left by processing it as a result of synchronous processing is set. 
//		bool&				bModified_
//			true
//				It has the file in the processing of the same period of this time. 
//				A part of the object has already been updated. 
//			false
//				It has the file in the processing of the same period of this time. 
//				The object has not been updated yet. 
//
//				Whether the file was updated as a result of synchronous processing is set. 
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
File::
sync(const Trans::Transaction& cTrans_, bool& bInComplete_, bool& bModified_)
{
	if (isMounted(m_cTrans))
	{
		if (isAttached())
			m_pFile->sync(cTrans_, bInComplete_, bModified_);
		else
			FileCommon::AutoPhysicalFile(
			m_cStorageStrategy, m_cBufferingStrategy,
			m_cFileID.getLockName())->sync(cTrans_, bInComplete_, bModified_);
	}
}

//	FUNCTION private
//	Record2::File::move -- move  file
//
//	NOTES
//
//	ARGUMENTS
//   ModUnicodeString& cstrOldPath_
//		Path name before it moves
//   ModUnicodeString& cstrNewPath_
//		Path name for its destination
//	 bool bUndo_ = false
//		Because it is move for error processing for true, no error processing. 
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
File::
move(ModUnicodeString& cstrOldPath_, ModUnicodeString& cstrNewPath_,
	 bool bUndo_)
{
	// attention  The file should be attached before a new path name is set. 
	AutoAttachFile file(*this);

	bool bMoved = false;
	const bool accessible = isAccessible();

	// A new path name is set. 
	m_cStorageStrategy.m_VersionFileInfo._path._masterData = cstrNewPath_;
	if (!m_cFileID.isTemporary())	
	{
		m_cStorageStrategy.m_VersionFileInfo._path._versionLog = cstrNewPath_;
		m_cStorageStrategy.m_VersionFileInfo._path._syncLog = cstrNewPath_;
	}

	try 
	{
		// The file is moved. 
		m_pFile->move(m_cTrans, m_cStorageStrategy.m_VersionFileInfo._path);
		bMoved = true;

		if (accessible)
		{
			// An old directory is clear. 
			ModOsDriver::File::rmAll(cstrOldPath_);
		}

		_SYDNEY_FAKE_ERROR("Record2::File::move",Exception::IllegalFileAccess(moduleName, srcFile, __LINE__));
	} 
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		if (!bUndo_)
		{
			// It returns it. 
			try 
			{
				if (bMoved)
				{
					move(cstrNewPath_, cstrOldPath_, true);
				}
				else if (accessible)
				{
					// A new directory is remove only when it is substance. 
					//
					// Attention The subdirectory
					//	When physical file that is substance is moved
					// The new directory is necessary to be deleted  by function(rmAll) though it is 
					// generated, because it is not deleted when an error comes. 
					ModOsDriver::File::rmAll(cstrNewPath_);
				}
			} 
		#ifdef NO_CATCH_ALL
			catch (Exception::Object&)
		#else
			catch (...)
		#endif
			{
				// Because it was not possible to return , it makes it  cannot be use. 
				Checkpoint::Database::setAvailability(
					m_cFileID.getLockName(), false);
			}
			_SYDNEY_RETHROW;
		}
	}
}

//	FUNCTION public
//	Record2::File::startVerification -- The correspondence inspection of a physical file begins. 
//
//	NOTES
//
//	ARGUMENTS
//	AAdmin::Verification::Treatment::Value iTreatment_
//		Inspection method of correspondence inspection
//	Admin::Verification::Progress&	cProgress_
//		Reference to process of correspondence inspection
//
//	RETURN
//	None
//
//	EXCEPTIONS
//
void 
File::
startVerification(const unsigned int iTreatment_,
						Admin::Verification::Progress& cProgress_)
{
	if (isMounted(m_cTrans))
	{
		try 
		{
			m_pFile->startVerification(m_cTrans, iTreatment_, cProgress_);
		} 
		catch (...) 
		{
			m_pFile->endVerification(m_cTrans, cProgress_);
			_SYDNEY_RETHROW;
		}
	}
}

//	FUNCTION public
//	Record2::File::endVerification -- The correspondence inspection of a physical file is ended. 
//
//	NOTES
//
//	ARGUMENTS
//	Admin::Verification::Progress&	cProgress_
//		Reference to process of correspondence inspection
//
//	RETURN
//	None
//
//	EXCEPTIONS
//
void 
File::
endVerification(Admin::Verification::Progress&	cProgress_)
{
	if (isMounted(m_cTrans)) {
		// It doesn't depend on contents of cProgress _ and endVerification() is executed. 
		m_pFile->endVerification(m_cTrans, cProgress_);
	}
}

//
//	FUNCTION public
//	Record2::File::setStorategy -- The strategy for a physical file is set from the meta data.. 
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Type& ePhysicalFileType_
//		Management type for PhysicalFile
//	const ModUnicodeString& cstrFileName_
//		FileName part
//
//	RETURN
//	None
//
//	EXCEPTIONS
//
void
File::
setStorategy(const PhysicalFile::Type& ePhysicalFileType_, const ModUnicodeString& cstrFileName_)
{
	// The storage strategy is set.
	m_cStorageStrategy.m_PhysicalFileType = ePhysicalFileType_;
	m_cStorageStrategy.m_PageUseRate = PhysicalFile::ConstValue::DefaultPageUseRate;
	m_cStorageStrategy.m_VersionFileInfo._pageSize = 
		m_bIsVariable ? m_cFileID.getVariablePageSize() : m_cFileID.getFixedPageSize();

	Os::Path cstrPath(m_cFileID.getDirectoryPath());
	cstrPath.addPart(cstrFileName_);

	m_cStorageStrategy.m_VersionFileInfo._path._masterData = cstrPath;

	if (!m_cFileID.isTemporary())
	{
		m_cStorageStrategy.m_VersionFileInfo._path._versionLog = cstrPath;
		m_cStorageStrategy.m_VersionFileInfo._path._syncLog = cstrPath;
	}

	m_cStorageStrategy.m_VersionFileInfo._sizeMax._masterData =
		PhysicalFile::ConstValue::DefaultFileMaxSize;
	m_cStorageStrategy.m_VersionFileInfo._sizeMax._versionLog =
		PhysicalFile::ConstValue::DefaultFileMaxSize;
	m_cStorageStrategy.m_VersionFileInfo._sizeMax._syncLog =
		PhysicalFile::ConstValue::DefaultFileMaxSize;

	m_cStorageStrategy.m_VersionFileInfo._extensionSize._masterData =
		PhysicalFile::ConstValue::DefaultFileExtensionSize;
	m_cStorageStrategy.m_VersionFileInfo._extensionSize._versionLog =
		PhysicalFile::ConstValue::DefaultFileExtensionSize;
	m_cStorageStrategy.m_VersionFileInfo._extensionSize._syncLog =
		PhysicalFile::ConstValue::DefaultFileExtensionSize;

	//if ( m_cFileID.isMounted()) 
	bool bMounted;
	if(m_cFileID.getBoolean(_SYDNEY_FILE_PARAMETER_KEY(
		FileCommon::FileOption::Mounted::Key), bMounted)) 
	{
		m_cStorageStrategy.m_VersionFileInfo._mounted = bMounted;
	} 
	else 
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// set the buffer type for version file
	m_cBufferingStrategy.m_VersionFileInfo._category =
		m_cFileID.isTemporary() ?
		Buffer::Pool::Category::Temporary
		: (m_cFileID.isReadOnly() ?
		   Buffer::Pool::Category::ReadOnly
		   : Buffer::Pool::Category::Normal);
}


//	FUNCTION private
//	Record2::File::substantiate -- Make a substance
//
//	NOTES
//
//	ARGUMENTS
// 	None
//
//	RETURN
//	None
//
//	EXCEPTIONS

void
File::
substantiate()
{
	; _SYDNEY_ASSERT(!isMounted(m_cTrans));

	const bool bIsAttached = isAttached();
	if (!bIsAttached) 
	{
		attachFile();
		; _SYDNEY_ASSERT(m_pFile);
	}

	try 
	{
		// A physical file that is substance is generated. 
		m_pFile->create(m_cTrans);

		_SYDNEY_FAKE_ERROR("Record2::File::substantiate",Exception::IllegalFileAccess(moduleName, srcFile, __LINE__));
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		try 
		{
			// The subdirectory is annulled. 
			// 【 attention 】 subdirectory :. 
			// It is necessary to delete it by this function though it is generated
			// if necessary when a physical file that is substance is generated 
			// because it is not deleted when making an error. 
			ModOsDriver::File::rmAll(getPath(), ModTrue);
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			// Because it was not possible to return it, it makes it to cannot use. 
			Checkpoint::Database::setAvailability(
				m_cFileID.getLockName(), false);
		}

		if (!bIsAttached) detachFile();
		_SYDNEY_RETHROW;
	}

	if (!bIsAttached) detachFile();
}

//
//	Copyright (c), 2007, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
