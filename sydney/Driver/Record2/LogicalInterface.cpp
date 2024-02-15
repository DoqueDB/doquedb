// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalInterface.cpp -- Implement interfaces of LogicalInterface.h
// 
// Copyright (c) 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
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
const char	srcFile[] = __FILE__;
const char	moduleName[] = "Record2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Checkpoint/Database.h"
#include "Common/IntegerArrayData.h"
#include "LogicalFile/TreeNodeInterface.h"

#include "Exception/FakeError.h"
//#include "Exception/BadArgument.h"
#include "Exception/FileAlreadyExisted.h"
#include "Exception/FileNotOpen.h"
#include "Exception/IllegalFileAccess.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Record2/Message_VerifyFailed.h"
#include "Record2/Message_DiscordObjectNum.h"
#include "Record2/Message_VerifyPhysicalFileStarted.h"
#include "Record2/Message_VerifyPhysicalFileFinished.h"
#include "Record2/Message_VerifyContentStarted.h"
#include "Record2/Message_VerifyContentFinished.h"

#include "Record2/Debug.h"
#include "Record2/OpenOption.h"
#include "Record2/File.h"
#include "Record2/ObjectManager.h"
#include "Record2/AreaManager.h"
#include "Record2/DataAccess.h"

#include "Record2/LogicalInterface.h"

_SYDNEY_USING

_SYDNEY_RECORD2_USING

namespace
{
	const ModUnicodeString _cstrZero("0");

	//
	//	CLASS
	//	AutoDetachPageAll -- 
	//
	//	NOTES
	//	DetachAll does the page at the end in scope. 
	//
	template< class T >
	class AutoDetachPageAll : FileCommon::AutoObject
	{
	public:
		AutoDetachPageAll(T* pFile_)
			: m_bSucceeded(false)
			, m_pFile(pFile_)
		{
		}
		~AutoDetachPageAll()
		{
			detachAll();
		}
		void succeeded() throw()
		{
			m_bSucceeded = true;
		}
		void detachAll()
		{
			// The page to which attach is done and all detach is done.
			// Two or more times of detachPageAll() are executable. 
			if (m_pFile) m_pFile->detachAll(m_bSucceeded);
		}
	private:
		bool m_bSucceeded;
		T* const m_pFile;
	};

	//DetachAll does the page/area at the end in scope. 
	class AutoDetacher : FileCommon::AutoObject
	{
	public:
		AutoDetacher(ObjectManager* pObjectManager_ ,
			AreaManager* pAreaManager_ = 0)
			: m_cObjectManager(pObjectManager_)
			, m_cAreaManager(pAreaManager_)
		{
		}
		~AutoDetacher()
		{
		}
		void succeeded() throw()
		{
			m_cObjectManager.succeeded();
			m_cAreaManager.succeeded();
		}
	private:
		AutoDetachPageAll<ObjectManager> m_cObjectManager;
		AutoDetachPageAll<AreaManager> m_cAreaManager;
	};
}

//
// PUBLIC METHOD
//

//	FUNCTION public
//	Record2::LogicalInterface::File -- Constructor
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID&	cFileOption_
//		Reference to variable-length record file option object
//
//	RETURN
//	None
//
//	EXCEPTIONS

LogicalInterface::
LogicalInterface(const LogicalFile::FileID&	cFileID_)
	: m_pObjectManager(0),
	  m_pAreaManager(0),
	  m_pFixedFile(0),
	  m_pVariableFile(0),
	  m_pDataAccess(0),
	  m_cFileID(cFileID_),
	  m_cOpenParam(FileCommon::OpenMode::Read,
				  false,	// Estimate flag of dummy
				  true,		// The order of sorting of dummy
				  false),	//field selected
	  m_iFetchObjectID(ObjectID::m_UndefinedValue)
{
	TRACEMSG("Record2::LogicalInterface::constructor");
	MSGLIN( SydRecordTraceMessage << "Record2::FileID: " << cFileID_.toString() << ModEndl );
}

//
//	FUNCTION public
//	Record2::LogicalInterface::~LogicalInterface -- Destructor
//
//	NOTES
//	Destructor
//
//	ARGUMENTS
//	none
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	[YET!]
//
LogicalInterface::
~LogicalInterface()
{
	TRACEMSG("Record2::LogicalInterface::destructor");

	//call it to free resource
	close();
}

//
//	FUNCTION public
//	Record2::LogicalInterface::initializeManager -- The record file driver is initialized
//
//	NOTES
//	initialized
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
//static
void
LogicalInterface::
initialize()
{
	; // do noting
}

//
//	FUNCTION public
//	Record2::LogicalInterface::terminateManager -- Termination
//
//	NOTES
//	Termination
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
//static
void
LogicalInterface::
terminate()
{
	; // do nothing
}

//
//	FUNCTION public
//	Record2::LogicalInterface::getFileID -- return the FileID
//
//	NOTES
//	The file-identification is returned.
//
//	ARGUMENTS
//	none
//
//	RETURN
//	const Logical::FileID&
//		Reference to own logical file-identification object
//
//	EXCEPTIONS
//
const LogicalFile::FileID&
LogicalInterface::getFileID() const
{
	TRACEMSG("Record2::LogicalInterface::getFileID");
	return m_cFileID;
}

//
//	FUNCTION public
//	Record2::LogicalInterface::getSize -- get the size of the record file
//
//	NOTES
// 	The record file returns the size of its underlying physical file. 
// 	The record file shoud open first when this function is called. 

//
//	ARGUMENTS
//	none
//
//	RETURN
//	ModUInt64
//		size byte of the record file [byte]
//
//	EXCEPTIONS
//	FileNotOpen
//		The file has not been opened
//
ModUInt64
LogicalInterface::
getSize() const
{
	TRACEMSG("Record2::LogicalInterface::getSize");
	if (!isPrepared()) 
	{
		_SYDNEY_THROW0(Exception::FileNotOpen);
	}

	//return fixed and variable size
	if (!m_pVariableFile) 
	{
		return m_pFixedFile->getSize();
	} 
	else 
	{
		; _SYDNEY_ASSERT(m_pVariableFile->isAttached());
		return m_pFixedFile->getSize() + m_pVariableFile->getSize();
	}
}

//
//	FUNCTION public
//	Record2::LogicalInterface::getCount -- get the number of tuples that has been inserted
//
//	NOTES
//	The record file returns the total count of tuples that inserted.
//	When this function is called,the record file should be opened first
//
//	ARGUMENTS
//	none
//
//	RETURN
//	ModInt64
//		Total count of tuples
//	EXCEPTIONS
//	FileNotOpen
//		The record file has not been opened. 
//
ModInt64
LogicalInterface::
getCount() const
{
	TRACEMSG("Record2::LogicalInterface::getCount");

	//prepare ObjectManager and AreaManager

	//can't convert "this" pointer from const Sydney::Record2::LogicalInterface"
	//to "Sydney::Record2::LogicalInterface &"
	const_cast<LogicalInterface*>(this)->initializeManagers(false);

	// Whether the ObjectManager has been opened is checked. 
	if (!isPrepared(false, true)) 
	{
		_SYDNEY_THROW0(Exception::FileNotOpen);
	}

	AutoDetacher detacher(m_pObjectManager);

	// The number of objects is obtained from the file header. 
	ModInt64 count = m_pObjectManager->getInsertedObjectNum();
	
	detacher.succeeded();

	return count;
}

//
//	FUNCTION public
//	Record2::LogicalInterface::getOverhead -- return the overhead when the object is retrieved
//
//	NOTES
//	The record file returns the estimated time when the object is retrieved [second] . 
//
//	ARGUMENTS
//	none
//
//	RETURN
//	double
//		Overhead second when object is retrieved
//
//	EXCEPTIONS
//
double
LogicalInterface::
getOverhead() const
{
	TRACEMSG("Record2::LogicalInterface::getOverhead");
	if (!isPrepared()) 
	{
		_SYDNEY_THROW0(Exception::FileNotOpen);
	}

	if (!m_pVariableFile) 
	{
		return m_pFixedFile->getOverhead();
	} 
	else 
	{
		; _SYDNEY_ASSERT(m_pVariableFile->isAttached());
		return m_pFixedFile->getOverhead() + m_pVariableFile->getOverhead();
	}
}

//
//	FUNCTION public
//	Record2::LogicalInterface::getProcessCost --
//		The process cost when it accesses the object is returned. 
//
//	NOTES
//	The record file returns the estimate process cost when it is accessed[second]. 
//
//	ARGUMENTS
//	none
//
//	RETURN
//	double
//		Process cost [second]
//
//	EXCEPTIONS
//	FileNotOpen
//		The record file has not been opened. 
//
double
LogicalInterface::
getProcessCost() const
{
	TRACEMSG("Record2::LogicalInterface::getProcessCost");
	if (!isPrepared()) 
	{
		_SYDNEY_THROW0(Exception::FileNotOpen);
	}

	if (!m_pVariableFile) 
	{
		return m_pFixedFile->getProcessCost();
	} 
	else 
	{
		; _SYDNEY_ASSERT(m_pVariableFile->isAttached());
		// getCount() and attachPage
		return m_pFixedFile->getProcessCost()
				+ m_pVariableFile->getProcessCost(getCount());
	}

	//detach all pages?
}

//
//	FUNCTION public
//	Record2::LogicalInterface::getSearchParameter -- The retrieval opening parameter is set. 
//
//	NOTES
//	When you acquire the object from the record file by get().
//	The object is acquired from the head one by one.  (Scan mode)
//	
//  When the Object ID is specified by the argument of fetch(), and the specific object is acquired. (fetch+get mode)
//	The object can be accesseed in two methods above. 
//	The search condition (argument pCondition _) that can be analyzed is a Fetch node looks like a tree structure.
//	Moreover, when pCondition_ = 0, the record file regard that
// 	the object should be acquisition from the head one by one.(scan mode) 
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface*	pCondition_
//		Pointer to retrieval condition object of tree structure
//	LogicalFile::OpenOption& cOpenOption_
//		Reference to record file opening option object
//
//	RETURN
//	bool
//		If the search condition show by the argument pCondition_ can be set into Record, 
//		true is returned, else false is returned. 
//
//	EXCEPTIONS
//	none
//
bool
LogicalInterface::
getSearchParameter(const LogicalFile::TreeNodeInterface* pCondition_,
	LogicalFile::OpenOption& cOpenOption_) const
{
	TRACEMSG("Record2::LogicalInterface::getSearchParameter");
	//All objects corresponding to the search condition are not maintained on the driver side. 
	cOpenOption_.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::CacheAllObject::Key), false);
	if (pCondition_ == 0)
	{
		// SCAN mode (mode to acquire object with get one by one)

		// An open mode is set to an open option (reference argument). 
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
			FileCommon::OpenOption::OpenMode::Key), FileCommon::OpenOption::OpenMode::Read);

		return true;
	}

	if (pCondition_->getType() == LogicalFile::TreeNodeInterface::Fetch)
	{
		// Fetch mode (mode for which object ID is specified by argument of fetch)

		if (pCondition_->getOptionSize() != 2)
		{
			//pCondition_ has only two operands, has a format of [LeftOperand operation RightOperand]
			_SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		// Column list of first element that Fetch is done
		const LogicalFile::TreeNodeInterface* pFetchedFields = pCondition_->getOptionAt(0);
		// The list treats oneself as an operand. 
		if (pFetchedFields->getOperandSize() != 1)
		{
			// (summary of mail from Mr. Kitagawa as follows)
			// Question: "Is Fetch mode can be done with out using object ID?That is using other Columns". 
			// Answer:Because the objects in record file are uniquely addressed by OID,so
            // it can only be fetched by OID  
			return false;
		}

		const LogicalFile::TreeNodeInterface* pField = pFetchedFields->getOperandAt(0);
		if (pField->getType() != LogicalFile::TreeNodeInterface::Field ||
			pField->getValue() != _cstrZero)
		{
			return false;
		}

		// When an open mode has already been set to an open option, 
		// it should be "Read. "Otherwise, exception. 
		int iValue;
		const bool bFind = cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(
						FileCommon::OpenOption::OpenMode::Key), iValue);
		if (bFind && iValue != FileCommon::OpenOption::OpenMode::Read)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		// An open mode is set to an open option (reference argument). 
		if (!bFind)
		{
			cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), 
				_SYDNEY_OPEN_PARAMETER_VALUE(FileCommon::OpenOption::OpenMode::Read));
		}

		return true;
	}

	return false;
}

//
//	FUNCTION public
//	Record2::LogicalInterface::getProjectionParameter -- The projection opening parameter is set. 
//
//	NOTES
//  Return the corresponding fields set by getProjectionParameter
//  By analysis the fields specified by cProjection_,we set them into record's openoption
//	For instance, if only the 2nd and 3rd field are acquired,the cProjection_ using a array to
//  keep the projection parameter like following:  
//
//	      cProjection_
//	   +------+-----+
//	   |         |      |   
//	   | [0]   |  2  |
//	   +------+----+
//	   | [1]   |  3  |
//	   |        |       |
//	   +-----+-----+
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	cProjection_
//		Reference to array object of field number
//	LogicalFile::OpenOption& cOpenOption_
//		Reference to open option object
//
//	RETURN
//	bool
//		True if we can set the cProjection_,else it returns false 
//
//	EXCEPTIONS
//	BadArgument
//		Illegal argument
//
bool
LogicalInterface::
getProjectionParameter(
	const Common::IntegerArrayData&	cProjection_,
	LogicalFile::OpenOption& cOpenOption_) const
{
	TRACEMSG("Record2::LogicalInterface::getProjectionParameter");
	// When an open mode has already been set to an open option, 
	// it should be "Read. "Otherwise, exception. 
	// (02/03: Mr. Kitagawa confirmed it to Mr./Ms. Ikeda. )
	int iValue;
	const bool bFind = cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), iValue);
	if (bFind && iValue != FileCommon::OpenOption::OpenMode::Read)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	if (cOpenOption_.getBoolean( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::GetByBitSet::Key)) )
	{
		// The record file cannot return the object that has been inserted by the bit set.
		return false;
	}

	// An open mode is set to an open option (reference argument). 
	if (!bFind)
	{
		cOpenOption_.setInteger( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), 
			_SYDNEY_OPEN_PARAMETER_VALUE(FileCommon::OpenOption::OpenMode::Read));
	}

	// The field selection specification is done is set. 
	cOpenOption_.setBoolean( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::FieldSelect::Key), true);

	// The field selection count is set. 
	const int iFieldNum = cProjection_.getCount();
	cOpenOption_.setInteger( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::TargetFieldNumber::Key), iFieldNum);

	// The digit number of the field that has been selected are set to the openoption. 
	for (int i = 0; i < iFieldNum; ++i)
	{
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				FileCommon::OpenOption::TargetFieldIndex::Key, i),
			cProjection_.getElement(i));
	}
	return true;
}

//
//	FUNCTION public
//	Record2::LogicalInterface::getUpdateParameter -- The update opening parameter is set. 
//
//	NOTES
//	The record file opening option in the update mode is set. 
//	To update only the corresponding fields,and these fields are 
//  	specified by argument cUpdateFields_, 
// 	and we set these parameters into the record file's open option. 
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	cProjection_
//		Reference to array object of field number
//	LogicalFile::OpenOption& cOpenOption_
//		Reference to open option object
//
//	RETURN
//	bool
//		True is returned if we can set the updateparameter. 
//
//	EXCEPTIONS
//	BadArgument
//		bad argument
//	[YET!]
//
bool
LogicalInterface::
getUpdateParameter(
	const Common::IntegerArrayData&	cProjection_,
	LogicalFile::OpenOption& cOpenOption_) const
{
	TRACEMSG("Record2::LogicalInterface::getUpdateParameter");
	// When an open mode has already been set to an open option, 
	// it should be "Update. "Otherwise, exception. 
	int iValue;
	const bool bFind = cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), iValue);
	if (bFind && iValue != FileCommon::OpenOption::OpenMode::Update)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// An open mode is set to an open option (reference argument). 
	if (!bFind)
	{
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), 
			_SYDNEY_OPEN_PARAMETER_VALUE(FileCommon::OpenOption::OpenMode::Update));
	}

	// The key shows that field select specification is done is set. 
	cOpenOption_.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::FieldSelect::Key), true);

	// The field selection specification is set. 
	const int iFieldNum = cProjection_.getCount();
	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::TargetFieldNumber::Key), iFieldNum);

	for (int i = 0; i < iFieldNum; ++i)
	{
		// The number of the updated field is acquired. 
		const int iPosition = cProjection_.getElement(i);
		if (iPosition == 0)
		{
			// The 0th field is object ID and it  cannot be changed, 
			// so false is returned. 
			return false;
		}

		// The position of the field that has been selected is set to an open option. 
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				FileCommon::OpenOption::TargetFieldIndex::Key, i), iPosition);
	}

	return true;
}

//
//	FUNCTION public
//	Record2::LogicalInterface::getSortParameter -- The sorting order parameter is set. 
//
//	NOTES
//  The tuples in record are sorted by the address(ObjectID).So we can only scan by
//  the order of OID.Scan by other field's order does not work.
//  By above statement ,we set the cKyes_ as the postion of OID,by default,OID is the 0th
//  field in record's tuple.cOrders_ can be set to 0(ASC) or 1(DESC) ,which means scan by  OID ASC or
//  scan by OID DESC. 
// 	 
//
//	    Argument cKeys _
//
//	    +-- Common::IntegerArrayData Object  --+
//	    |           +-------------------+                            |
//	    |  [0]    |          0              |                            |
//	    |           +-------------------+                            |
//	    -----------------------------------------------------
//
//	     cOrders_
//
//	    +-- Common::IntegerArrayData object   --+
//	    |           +-------------------+                            |
//	    |  [0]    |          0  or 1     |                            |
//	    |           +-------------------+                            |
//	    -----------------------------------------------------
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	cKeys_
//		Reference to row of field index that specifies the order of sorting
//	const Common::IntegerArrayData&	cOrders_
//		If it is  ascending order, 0 is set,else if it is a descending order, 1 is set. 
//	LogicalFile::OpenOption&				cOpenOption_
//		Reference to record file opening option object
//
//	RETURN
//	bool
// 	True is returned when the order of sorting only of object ID field is specified and when 
// 	is specified for the order of sorting of the field other than object ID field, false is returned. 
//	
//	EXCEPTIONS
//	BadArgument
//		Illegal argument
//
bool
LogicalInterface::
getSortParameter(
	const Common::IntegerArrayData&	cKeys_,
	const Common::IntegerArrayData&	cOrders_,
	LogicalFile::OpenOption& cOpenOption_) const
{
	TRACEMSG("Record2::LogicalInterface::getSortParameter");
	if (cKeys_.getCount() != cOrders_.getCount())
	{
		// Illegal argument
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// Whether only object ID field is specified is checked. 
	if (cKeys_.getCount() == 1 && cKeys_.getElement(0) == 0)
	{
		// The SortOrder parameter is set to an open option. 
		bool iSortOrder = (cOrders_.getElement(0) != 0);
		cOpenOption_.setBoolean(_SYDNEY_RECORD2_OPEN_PARAMETER_KEY(
				Record2::OpenOption::SortOrder::Key), iSortOrder);

		return true;
	}

	return false;
}

//
//	FUNCTION public
//	Record2::LogicalInterface::create -- The record file is generated. 
//
//	NOTES
//	The record file generates own physical file, 
//  and initializes the generated physical file. 
//
//	ARGUMENTS
//	const Trans::Transaction&	cTrans_
//		trans::ID
//
//	RETURN
//	const LogicalFile::FileID&
//		Reference to own logical file-identification object
//
//	EXCEPTIONS
//	FileAlreadyExisted
//		The record file already exists. 
//	[YET!]
//		The parameter that should be set to the file option is not set. 
//	Unexpected
//		Unexpected error
//	[YET!]
//
const LogicalFile::FileID&
LogicalInterface::
create(const Trans::Transaction& cTrans_)
{
	TRACEMSG("Record2::LogicalInterface::create");

	// FileID is set.
	m_cFileID.create();
	
	// The object to access the file for fixed length/variable-length is made. 
	initializeFiles(cTrans_);

	//m_cStorageStrategy.m_VersionFileInfo._mounted = true;
	//I think File::create only switch mounted so it's can't throw exception
	m_pFixedFile->create();
	if (m_pVariableFile) 
	{
		m_pVariableFile->create();
	}

	//Mounted immediately after generation. 
	m_cFileID.setBoolean(_SYDNEY_FILE_PARAMETER_KEY(
		FileCommon::FileOption::Mounted::Key), true);
	//Driver's version
	m_cFileID.setInteger(_SYDNEY_FILE_PARAMETER_KEY(
		FileCommon::FileOption::Version::Key), FileID::Version::CurrentVersion);

	return m_cFileID;
}

//
//	FUNCTION public
//	Record2::LogicalInterface::destroy -- The record file is destroy. 
//
//	NOTES
//	The record file destroy its own physical file.
//
//	ARGUMENTS
//	const Trans::Transaction&	cTrans_
//		Transaction descriptor
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	[YET!]
//
void
LogicalInterface::
destroy(const Trans::Transaction&	 cTrans_)
{
	TRACEMSG("Record2::LogicalInterface::destroy");
	// The object to access the file for fixed length/variable-length is made. 
	initializeFiles(cTrans_);

	m_pFixedFile->destroy();
	if (m_pVariableFile) 
	{
		m_pVariableFile->destroy();
	}
}

//
//	FUNCTION public
//	Record2::LogicalInterface::isAccessible --
//		Whether OS file that is substance exists is examined. 
// 
//	NOTES
//	The record file may not been opened though is opened. 
//	It may not be opened. 
//
//	ARGUMENTS
//	none
//
//	RETURN
//	bool
//		Whether it is generated or not?
//		true: It is generated. 
//		false: It is not generated. 
//
//	EXCEPTIONS
//	[YET!]
//
bool
LogicalInterface::
isAccessible(bool force) const
{
	TRACEMSG("Record2::LogicalInterface::isAccessible");

	//if not substance
	if (!m_pFixedFile) 
	{
		// It is safe because it is not used internally though the reference 
		// that indicates 0 that makes a temporary object is passed because open is not done. 

		Trans::Transaction* pTempTrans = 0;	//it is strange!
		return FileAccess(*pTempTrans, m_cFileID, true).isAccessible(force) &&
			(!m_cFileID.hasVariable() || FileAccess(*pTempTrans, m_cFileID, false).isAccessible(force));
	} 
	else
	{
		return m_pFixedFile->isAccessible(force) &&
			(!m_pVariableFile || m_pVariableFile->isAccessible(force));
	}
}

//	FUNCTION public
//	Record2::LogicalInterface::isMounted -- Whether the mount is done is examined. 
//
//	NOTES
//		The record file may not been opened. 
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
//			The mount is not done. 
//
//	EXCEPTIONS

bool
LogicalInterface::isMounted(const Trans::Transaction& cTrans_) const
{
	TRACEMSG("Record2::LogicalInterface::isMounted");

	//same as isAccessible
	return (!m_pFixedFile) 
		 ?
		(FileAccess(cTrans_, m_cFileID, true).isMounted(cTrans_) &&
		 (!m_cFileID.hasVariable() || FileAccess(cTrans_, m_cFileID, false).isMounted(cTrans_))) 
		 :
		(m_pFixedFile->isMounted(cTrans_) &&
		 (!m_pVariableFile || m_pVariableFile->isMounted(cTrans_)));
}

//
//	FUNCTION public
//	Record2::LogicalInterface::open -- The record file is opened. 
//
//	NOTES
//	The record file is opened. 
//
//	ARGUMENTS
//	const Trans::Transaction&	cTrans_
//		Transaction descriptor
//	const LogicalFile::OpenOption&	cOpenOption_
//		Reference to open option object
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	BadArgument
//		BadArgument
//	NotSupported
//		It has already been opened. 
//	[YET!]
//
void
LogicalInterface::
open(const Trans::Transaction& cTrans_,
	 const LogicalFile::OpenOption& cOpenOption_)
{
	TRACEMSG("Record2::LogicalInterface::open");
	MSGLIN( SydRecordTraceMessage << "Record2::OpenOption: " << cOpenOption_.toString() << ModEndl );

	//just for moduletest
	//m_cFileID.create();
	m_cFileID.initialize();

	// An open mode is preserved in the data member. 
	int	iOpenModeValue = cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key));
	if (iOpenModeValue == FileCommon::OpenOption::OpenMode::Initialize)
	{
		//Nop is disregarded
		return;
	}

	// An open mode is preserved, should locate before initializeFiles
	saveOpenOption(cOpenOption_ ,iOpenModeValue);

	// The object to access the file for fixed length/variable-length is made. 
	initializeFiles(cTrans_);

	// Have you already been opened?
	if (m_pFixedFile->isAttached())
	{
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	//if is batch in inserting
	bool isBatch = (m_cOpenParam.m_iOpenMode == FileCommon::OpenMode::Batch);
	// A physical file is opened. 
	m_pFixedFile->attachFile(isBatch);
	if (m_pVariableFile) 
	{
		m_pVariableFile->attachFile(isBatch);
	}
}

//
//	FUNCTION public
//	Record2::LogicalInterface::close -- The record file is closed. 
//
//	NOTES
//	The record file is closed. 
//
//	ARGUMENTS
//	none
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	[YET!]
//
void
LogicalInterface::
close()
{
	TRACEMSG("Record2::LogicalInterface::close");
	if (!m_pFixedFile) 
	{
		//It has not been opened yet or it has already been closed. 
		return;
	}

	if(m_pDataAccess)
	{
		//delete data access first
		delete m_pDataAccess; 
		m_pDataAccess = 0;
	}

	//free others resource
	if(m_pAreaManager)
	{
		m_pAreaManager->detachAll(true);
		delete m_pAreaManager;
		m_pAreaManager = 0;
	}

	if(m_pObjectManager)
	{
		// All the resources are liberated when it is Pined. 
		//if ((m_pTrans == 0 || m_pTrans->isNoVersion()))
		m_pObjectManager->detachAll(true);

		delete m_pObjectManager;
		m_pObjectManager = 0;
	}

	if(m_pVariableFile)
	{
		delete m_pVariableFile; 
		m_pVariableFile = 0;
	}

	if(m_pFixedFile)
	{
		delete m_pFixedFile; 
		m_pFixedFile = 0;
	}

	// notice:
	// - The file option (meta data : FileID) doesn't change even if it closes. 
	// - An open option changes if it closes. 
}

//
//	FUNCTION public
//	Record2::LogicalInterface::fetch -- The search condition (object ID) is set. 
//
//	NOTES
//	The search condition (object ID) is set. 
//	Actually data is requested with get() function. 
//
//	ARGUMENTS
//	const Common::DataArrayData*	pOption_
//      pOption_ has only one element which is OID,
//      and it means we want to get the tuple which is addressed by OID
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	FileNotOpen
//		The record file has not been opened. 
//	IllegalFileAccess
//		Access to illegal logical file
//	[YET!]
//
void
LogicalInterface::fetch(const Common::DataArrayData* pOption_)
{
	TRACEMSG("Record2::LogicalInterface::fetch");
	if (!isPrepared()) 
	{
		// The record file has not been opened. 
		_SYDNEY_THROW0(Exception::FileNotOpen);
	}

	if (m_cOpenParam.m_iOpenMode != FileCommon::OpenMode::Read)
	{
		// An open mode is wrong.
		_SYDNEY_THROW0(Exception::IllegalFileAccess);
	}

	// the tobe-fetched tuple's OID is kept in variable m_iFetchObjectID,and in 
    // get() funtion ,it use the m_iFetchObjectID to directly position to the address 
    //the of corresponding tuple.
	m_iFetchObjectID = convertFetchOptionToObjectID(pOption_);
}

//
//	FUNCTION public
//	Record2::LogicalInterface::get -- The object that has been inserted is returned. 
//
//	NOTES
//	The object that has been inserted is returned. 
//	When object ID specified with fetch is the one that doesn't exist, 0 is returned. 
//
//	ARGUMENTS
//	Common::DataArrayData* pTuple_
//		data array
//
//	RETURN
//	bool
//		True if we can return the tuple specified by OID
//
//	EXCEPTIONS
//	FileNotOpen
//		The record file has not been opened. 
//	IllegalFileAccess
//		IllegalFileAccess
//	BadArgument
//		BadArgument
//	[YET!]
//
bool
LogicalInterface::get(Common::DataArrayData* pTuple_)
{
	TRACEMSG("Record2::LogicalInterface::get");
	
	//prepare ObjectManager and AreaManager
	initializeManagers(false, true);

	if (!isPrepared(false, true)) 
	{
		// It is not opened.
		_SYDNEY_THROW0(Exception::FileNotOpen);
	}

	// initialize DataAccess
	if (!initializeDataAccess(pTuple_, FileCommon::OpenMode::Read))
	{
		_SYDNEY_THROW0(Exception::IllegalFileAccess);
	}

	// DetachPageAll is done at the end in scope. 
	ObjectManager* pObjectManager = m_pObjectManager;
	AreaManager* pAreaManager = m_pAreaManager;
	if (m_pFixedFile->getTransaction().isNoVersion() == false)
	{
		// Detach is not separately done at the retrieval that uses the version. 
		pObjectManager = 0;
		pAreaManager = 0;
	}
	AutoDetacher detacher(pObjectManager, pAreaManager);

	//read real data from DataAccess 
	bool result = m_pDataAccess->read(m_iFetchObjectID);

	// The specification of Fetch is cleared when reading. 
	m_iFetchObjectID = ObjectID::m_UndefinedValue;

#ifdef DEBUG
	if (result) 
	{
		SydRecordDebugMessage
			<< "LogicalInterface::get() result: "
			<< pTuple_->getString()
			<< ModEndl;
	}
#endif

	//free ObjectManager and AreaManager
	detacher.succeeded();

	//The data array that returns the read object is made. 
	return result;
}

//
//	FUNCTION public
//	Record2::LogicalInterface::insert --data insertion
//
//	NOTES
//		It inserts object. 
//
//	ARGUMENTS
//	Common::DataArrayData*	pTuple_
//		the data to be inserted
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	FileNotOpen
//		The record file has not been opened. 
//	IllegalFileAccess
//		Access to illegal logical file
//	[YET!]
//
void
LogicalInterface::insert(Common::DataArrayData* pTuple_)
{
	TRACEMSG("Record2::LogicalInterface::insert");
	
	//must input data
	; _SYDNEY_ASSERT(pTuple_ != 0);

	//prepare ObjectManager and AreaManager
	initializeManagers(true, true, true);

	if (!isPrepared(false, true)) 
	{
		// It is not opened. 
		_SYDNEY_THROW0(Exception::FileNotOpen);
	}

	//must larger 1 fields
	if (pTuple_->getCount() < 1)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// The field that stores ObjectID is obtained.
	LogicalFile::ObjectID* pID = _SYDNEY_DYNAMIC_CAST(LogicalFile::ObjectID*, pTuple_->getElement(0).get());
	; _SYDNEY_ASSERT(pID);

	//however the 1st element is ObjectID 
	if (pTuple_->getCount() == 1)
	{
		// insert rollback, call undoExpunge
		//expung or undoExpunge?
		m_pObjectManager->undoExpunge(pID->getValue());
		//m_pObjectManager->expunge(pID->getValue());

#ifdef DEBUG
		SydRecordDebugMessage
			<< "LogicalInterface::undoExpunge() key: "
			<< pID->getString()
			<< ModEndl;
#endif

	}
	else
	{
		//insert mode
		bool bIsBatch = (m_cOpenParam.m_iOpenMode == FileCommon::OpenMode::Batch);

		if (!initializeDataAccess(pTuple_, bIsBatch ? 
			FileCommon::OpenMode::Batch : FileCommon::OpenMode::Update))
		{
			// An open mode is wrong. 
			_SYDNEY_THROW0(Exception::IllegalFileAccess);
		}

		if(m_cOpenParam.m_bFieldSelect)
			m_cFileID.adjustDataIndex(false);

		AutoDetacher detacher( bIsBatch ? 0 : m_pObjectManager,
							bIsBatch ? 0 : m_pAreaManager );

		// The insertion processing is done. 
		ObjectID::Value ullObjectID = m_pDataAccess->insert();
		// Object ID is set. 
		pID->setValue(ullObjectID);
		
		if(m_cOpenParam.m_bFieldSelect)
			m_cFileID.adjustDataIndex(true);

		//free ObjectManager and AreaManager
		detacher.succeeded();

#ifdef DEBUG
	SydRecordDebugMessage
		<< "LogicalInterface::insert() result: "
		<< pTuple_->getString()
		<< ModEndl;
#endif
	}
}

//
//	FUNCTION public
//	Record2::LogicalInterface::update -- The object is updated. 
//
//	NOTES
//	The object is updated. 
//
//	ARGUMENTS
//	const Common::DataArrayData*	pKey_
//		Object ID that specifies updated object
//		It is set to the first element of pObject_. 
//	Common::DataArrayData*	pObject_
//		Pointer such as updated objects
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	FileNotOpen
//		The record file has not been opened. 
//	IllegalFileAccess 
//		Access to illegal logical file
//	[YET!]
//
void
LogicalInterface::update(const Common::DataArrayData* pKey_, Common::DataArrayData* pTuple_)
{
	TRACEMSG("Record2::LogicalInterface::update");

	; _SYDNEY_ASSERT(pTuple_);

	//prepare ObjectManager and AreaManager
	initializeManagers(true, true);

	AutoDetacher detacher(m_pObjectManager, m_pAreaManager );

	if (!isPrepared(false, true)) 
	{
		// It is not opened.
		_SYDNEY_THROW0(Exception::FileNotOpen);
	}

	if (!initializeDataAccess(pTuple_, FileCommon::OpenMode::Update))
	{
		// Open mode is wrong. 
		_SYDNEY_THROW0(Exception::IllegalFileAccess);
	}

	// The key is ObjectID. 
	const LogicalFile::ObjectID* pID = ObjectID::getLogicalObjectID(pKey_);
	if (ObjectID::isObjectIDType(pTuple_, 0) &&
			m_cFileID.getFieldIDbyDataIndex(0) == 0)
	{
		//roolback update, call updoUpdate
		m_pObjectManager->setDataAccess(m_pDataAccess);
		m_pObjectManager->undoUpdate(pID->getValue());

#ifdef DEBUG
		SydRecordDebugMessage
			<< "LogicalInterface::undoUpdate() key: "
			<< pKey_->getString() 
			<< ModEndl;
#endif

	}
	else
	{
		//for update, we must decrease it
		if(!m_cOpenParam.m_bFieldSelect)
			m_cFileID.adjustDataIndex(true);

		//do update 
		m_pDataAccess->update(pID->getValue());

		//should recover dataindex
		if(!m_cOpenParam.m_bFieldSelect)
			m_cFileID.adjustDataIndex(false);

#ifdef DEBUG
	SydRecordDebugMessage
		<< "LogicalInterface::update() key: "
		<< pKey_->getString()
		<< " data: "
		<< pTuple_->getString()
		<< ModEndl;
#endif

	}

	//detach all pages/areas
	detacher.succeeded();
}

//
//	FUNCTION public
//	Record2::LogicalInterface::expunge -- The object is deleted. 
//
//	NOTES
//	The object is deleted. 
//
//	ARGUMENTS
//	const Common::DataArrayData*	pKey_
//		Object ID that specifies deleted object
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	FileNotOpen
//		The record file has not been opened. 
//	IllegalFileAccess
//		Access to illegal logical file
//	[YET!]
//
void
LogicalInterface::expunge(const Common::DataArrayData* pKey_)
{
	TRACEMSG("Record2::LogicalInterface::expunge");

	//do expunge really
	doExpunge(pKey_, false);

#ifdef DEBUG
	SydRecordDebugMessage
		<< "LogicalInterface::expunge() key: "
		<< pKey_->getString()
		<< ModEndl;
#endif

}

//
//	FUNCTION privite
//	Record2::LogicalInterface::undoUpdate -- updo update. 
//
//	NOTES
//		undo update, the updated data will recover		
//
//	ARGUMENTS
//	const Common::DataArrayData*	pKey_
//		the OID to be undo update
//
//	RETURN
//	none
//
//	EXCEPTIONS
//

void
LogicalInterface::
undoUpdate(const Common::DataArrayData* pKey_)
{
	TRACEMSG("Record2::LogicalInterface::undoUpdate");

	//prepare ObjectManager and AreaManager
	initializeManagers(true, true);

	if (!isPrepared(false, true)) 
	{
		// It is not opened.
		_SYDNEY_THROW0(Exception::FileNotOpen);
	}

	if (!initializeDataAccess(0, FileCommon::OpenMode::Update))
	{
		// Open mode is wrong. 
		_SYDNEY_THROW0(Exception::IllegalFileAccess);
	}

#ifdef DEBUG
	SydRecordDebugMessage
		<< "LogicalInterface::undoUpdate() key: "
		<< pKey_->getString()
		<< ModEndl;
#endif

	AutoDetacher detacher(m_pObjectManager, m_pAreaManager );

	// the OID is get for delete
	const LogicalFile::ObjectID* pID = ObjectID::getLogicalObjectID(pKey_);

	// do undo update really
	m_pObjectManager->setDataAccess(m_pDataAccess);
	m_pObjectManager->undoUpdate(pID->getValue());

	//free ObjectManager and AreaManager
	detacher.succeeded();
}

//
//	FUNCTION privite
//	Record2::LogicalInterface::undoExpunge -- updo expunge. 
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData*	pKey_
//		the OID to be undo expunge
//
//	RETURN
//	none
//
//	EXCEPTIONS
//

void
LogicalInterface::
undoExpunge(const Common::DataArrayData* pKey_)
{
	TRACEMSG("Record2::LogicalInterface::undoExpunge");

	//undo expunge really
	doExpunge(pKey_, true);

#ifdef DEBUG
	SydRecordDebugMessage
		<< "LogicalInterface::undoExpunge() key: "
		<< pKey_->getString()
		<< ModEndl;
#endif
}

//
//	FUNCTION public
//	Record2::LogicalInterface::expunge -- The object is deleted. 
//
//	NOTES
//	The object is deleted. 
//
//	ARGUMENTS
//	const Common::DataArrayData*	pKey_
//		Object ID that specifies deleted object
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	FileNotOpen
//		The record file has not been opened. 
//	IllegalFileAccess
//		Access to illegal logical file
//	[YET!]
//
void
LogicalInterface::doExpunge(const Common::DataArrayData* pKey_, bool bUndo_/* = false*/)
{
	//prepare ObjectManager and AreaManager
	initializeManagers(true);

	if (!isPrepared(false, true)) 
	{
		// It is not opened.
		_SYDNEY_THROW0(Exception::FileNotOpen);
	}

	//need data accessing
	if (m_cOpenParam.m_iOpenMode != FileCommon::OpenMode::Update)
	{
		// Open mode is wrong. 
		_SYDNEY_THROW0(Exception::IllegalFileAccess);
	}

	AutoDetacher detacher(m_pObjectManager, m_pAreaManager );

	// the OID is get for delete
	const LogicalFile::ObjectID* pID = ObjectID::getLogicalObjectID(pKey_);

	// It deletes it. 
	if(bUndo_)	m_pObjectManager->undoExpunge(pID->getValue());
	else m_pObjectManager->expunge(pID->getValue());

	//free ObjectManager and AreaManager
	detacher.succeeded();
}


//
//	FUNCTION public
//	Record2::LogicalInterface::compact -- do compact,the data will delete really
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTrans_
//		
//  bool& bIncomplete_
//
//  bool& bModified_
//
//	RETURN
//	none
//
//	EXCEPTIONS
//
void 
LogicalInterface::
compact(const Trans::Transaction& cTrans_,
			bool& bIncomplete_, bool& bModified_)
{
	TRACEMSG("Record2::LogicalInterface::compact");

	//attach file
	LogicalFile::OpenOption openOption;
	openOption.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
		FileCommon::OpenOption::OpenMode::Key), FileCommon::OpenOption::OpenMode::Update);

	open(cTrans_, openOption);

	//should check whether the file is mounted
	if(isMounted(cTrans_))
	{
		//prepare ObjectManager and AreaManager
		initializeManagers(true, true);

		if (!isPrepared(false, true)) 
		{
			// It is not opened.
			_SYDNEY_THROW0(Exception::FileNotOpen);
		}

		if (!initializeDataAccess(0, FileCommon::OpenMode::Update))
		{
			// Open mode is wrong. 
			_SYDNEY_THROW0(Exception::IllegalFileAccess);
		}

		AutoDetacher detacher(m_pObjectManager, m_pAreaManager );

		//do really
		m_pObjectManager->setDataAccess(m_pDataAccess);
		Utility::ObjectNum uiCount = m_pObjectManager->compact();

		//free ObjectManager and AreaManager
		detacher.succeeded();

	#ifdef DEBUG
		SydRecordDebugMessage
			<< "LogicalInterface::compact() : "
			<< " compact object number :"
			<< uiCount
			<< ModEndl;
	#endif
		
		//set status
		if (cTrans_.isCanceledStatement() && !bIncomplete_)
			bIncomplete_ = true;

		if(uiCount > 0) bModified_ = true;
		else bModified_ = false;
	}
}

//
//	FUNCTION public
//	Record2::LogicalInterface::mark -- The position of the rewinding is recorded. 
//
//	NOTES
//	The position of the rewinding is recorded
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
void
LogicalInterface::
mark()
{
	TRACEMSG("Record2::LogicalInterface::mark");
	; _SYDNEY_ASSERT(m_pObjectManager);
	// It only has to do mark to the representative objectfile. 
	m_pObjectManager->mark();
}

//
//	FUNCTION public
//	Record2::LogicalInterface::rewind -- It returns to the recorded position. 
//
//	NOTES
//	It returns to the recorded position maintain in mark()
//
//	ARGUMENTS
//	none
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	FileNotOpen
//		The record file has not been opened. 
//	[YET!]
//
void
LogicalInterface::
rewind()
{
	TRACEMSG("Record2::LogicalInterface::rewind");
	; _SYDNEY_ASSERT(m_pObjectManager);

	AutoDetacher detacher(m_pObjectManager);

	// It only has to do rewind to the representative objectfile. 
	m_pObjectManager->rewind();

	detacher.succeeded();
}

//
//	FUNCTION public
//	Record2::LogicalInterface::reset -- The cursor is reset. 
//
//	NOTES
//	The cursor is reset
//
//	ARGUMENTS
//	none
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	FileNotOpen
//		The record file has not been opened. 
//	[YET!]
//
void
LogicalInterface::
reset()
{
	TRACEMSG("Record2::LogicalInterface::reset");
	; _SYDNEY_ASSERT(m_pObjectManager);

	// It only has to do reset to the representative objectfile. 
	m_pObjectManager->reset();
}

//
//	FUNCTION public
//	Record2::LogicalInterface::equals -- compare
//
//	NOTES
//	Argument pOther_ is compared with oneself, and the comparison result is returned. 
//	*** It checks whether the same object is not checked, and each member is equal (Is it equivalent?). 
//	   - It is noted not to compare all the values. 
//  
//	ARGUMENTS
//	const Common::Object*	pOther_
//		Pointer to object of comparison object
//
//	RETURN
//	bool
//		If argument pOther _ is equivalent, true is returned and, 
//		otherwise, false is returned with oneself. 
//
//	EXCEPTIONS
//	none
//
bool
LogicalInterface::
equals(const Common::Object*	pOther_) const
{
	TRACEMSG("Record2::LogicalInterface::equals");
	// If argument pOther_ is a null pointer, it is not equal
	if (pOther_ == 0) return false;

	// It is not equal if the argument pOther_ is not an instance object of the Record2::File class. 
	const Record2::LogicalInterface*	pOther = 0;
	if ((pOther = dynamic_cast<const Record2::LogicalInterface*>(pOther_)) == 0)
	{
		return false;
	}

	// If the meta data is different, both consider that they are different. 
	// (Judge according to mounting of the MetaData class, that is, "The meta data is different". )
	if (!m_cFileID.equals(pOther->m_cFileID))
	{
		return false;
	}

	return true;
}

//	FUNCTION public
//	Record2::LogicalInterface::sync -- The synchronization of the record file is taken. 
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	cTrans_
//			Transaction descriptor of transaction that takes synchronization of record file
//		bool&				bIncomplete_
//			true
//				partially processed. 
//			false
//				completely processed. 
//
//				Whether the record file was left by processing it as a result of synchronous 
//				processing is set. 
//		bool&				bModified_
//			true
//				A part of the object with the record file in the processing of the same period of 
//				this time has already been updated.
//				
//			false
//				The object with the record file in the processing of the same period of this time 
//				has not been updated yet. 
//
//				Whether the record file was updated as a result of synchronous processing is set. 
//
//	RETURN
//		none
//
//	EXCEPTIONS

void
LogicalInterface::sync(const Trans::Transaction& cTrans_, bool& bIncomplete_, bool& bModified_)
{
	TRACEMSG("Record2::LogicalInterface::sync");
	// The object to access the file for fixed length/variable-length is made. 
	initializeFiles(cTrans_);

	if (isMounted(cTrans_) == true)
	{
		if (m_pFixedFile)
			m_pFixedFile->sync(cTrans_, bIncomplete_, bModified_);
		if (m_pVariableFile)
			m_pVariableFile->sync(cTrans_, bIncomplete_, bModified_);
	}
}

//
//	Utility
//

//
//	FUNCTION public
//	Record2::LogicalInterface::move -- The file is moved. 
//
//	NOTES
//  The file is moved. 
//
//	ARGUMENTS
//	const Trans::Transaction&		cTrans_
//		transactoin descriptor
//	const Common::StringArrayData&	cArea_
//		Reference to record file storage directory path structure after it moves
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	[YET!]
//
void
LogicalInterface::
move(const Trans::Transaction& cTrans_,
	 const Common::StringArrayData& cArea_)
{
	TRACEMSG("Record2::LogicalInterface::move");
	// The object to access the file for fixed length/variable-length is made. 
	initializeFiles(cTrans_);

	// After a present path is stored, it sets it to a new path
	const Os::Path& cstrOldPath = m_cFileID.getDirectoryPath();
	const ModUnicodeString& cstrNewPath = cArea_.getElement(0);

	// It executes it only when passing is different. 
	if (Os::Path::compare(cstrOldPath, cstrNewPath)
		== Os::Path::CompareResult::Unrelated) 
	{
		m_cFileID.setDirectoryPath(cstrNewPath);

		//temporary enum for exception
		enum 
		{
			None,
			DirectMoved,
			VariableMoved,
			ValueNum
		} eStatus = None;

		try 
		{
			m_pFixedFile->move();
			eStatus = DirectMoved;
			if (m_pVariableFile) 
			{
				m_pVariableFile->move();
				eStatus = VariableMoved;
			}

			_SYDNEY_FAKE_ERROR("Record2::LogicalInterface::move",
				Exception::IllegalFileAccess(moduleName, srcFile, __LINE__));

		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			try 
			{
				// It returns based on the passing specification and it moves again. 
				m_cFileID.setDirectoryPath(cstrOldPath);
				switch (eStatus) 
				{
				case VariableMoved:
					m_pVariableFile->move(true /* undo */);
					// thru
				case DirectMoved:
					m_pFixedFile->move(true /* undo */);
					// thru
				case None:
				default:
					break;
				}
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
			_SYDNEY_RETHROW;
		}
	}
}

//
//	FUNCTION public
//	Record2::LogicalInterface::getNoLatchOperation --
//		The operation with an unnecessary latch is returned. 
//
//	NOTES
//	The operation with an unnecessary latch is returned. 
//
//	ARGUMENTS
//	none
//
//	RETURN
//
//	EXCEPTIONS
//	[YET!]
 
LogicalFile::File::Operation::Value
LogicalInterface::
getNoLatchOperation()
{
	return Operation::Open
		| Operation::Close
		| Operation::Fetch;
}

// FUNCTION public
//	Record2::LogicalInterface::getCapability -- Capabilities of file driver
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	LogicalFile::File::Capability::Value
//
// EXCEPTIONS

LogicalFile::File::Capability::Value
LogicalInterface::
getCapability()
{
	return LogicalFile::File::Capability::Undo;
}

//
//	FUNCTION public
//	Record2::LogicalInterface::toString --
//		The character string to identify the file is returned. 
//
//	NOTES
//	The character string to identify the file is returned. 
//
//	ARGUMENTS
//	none
//
//	RETURN
//	ModUnicodeString
//		Character string to identify file
//
//	EXCEPTIONS
//	[YET!]
//
ModUnicodeString
LogicalInterface::
toString() const
{
	TRACEMSG("Record2::LogicalInterface::toString");
	return m_cFileID.getDirectoryPath();
}

//
//	FUNCTION public
//	Record2::LogicalInterface::mount -- the record file is mount. 
//		
//  
//	NOTES
//	the record file is mount
//
//	ARGUMENTS
//	const Trans::Transaction&	cTrans_
//		transaction descriptor
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	[YET!]
//
const LogicalFile::FileID&
LogicalInterface::
mount(const Trans::Transaction&	cTrans_)
{
	TRACEMSG("Record2::LogicalInterface::mount");
	// The object to access the file for fixed length/variable-length is made. 
	initializeFiles(cTrans_);

	if (!isMounted(cTrans_)) 
	{
		// If the mount is not done, the unmount is done. 
		int st = 0;
		try 
		{
			m_pFixedFile->mount();
			++st;//1
			if (m_pVariableFile)
				m_pVariableFile->mount();
			++st;//2
		}

#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			switch (st) 
			{
			case 2:
				if (m_pVariableFile)
					m_pVariableFile->unmount();
				//fall thru
			case 1:
				m_pFixedFile->unmount();
				//fall thru
			default:
				;//nop
			}
			_SYDNEY_RETHROW;
		}

		// It is recorded that the mount was done in the file identifier. 
		m_cFileID.setBoolean(_SYDNEY_FILE_PARAMETER_KEY(
				FileCommon::FileOption::Mounted::Key), true);
	}

	return m_cFileID;
}

//
//	FUNCTION public
//	Record2::LogicalInterface::unmount -- The record file is unmounted
//
//	NOTES
//	The record file is Ammaunted. 
//
//	ARGUMENTS
//	const Trans::Transaction&	cTrans_
//		transaction descriptor
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	[YET!]
//
const LogicalFile::FileID&
LogicalInterface::
unmount(const Trans::Transaction&	cTrans_)
{
	TRACEMSG("Record2::LogicalInterface::unmount");
	// The object to access the file for fixed length/variable-length is made. 
	initializeFiles(cTrans_);

	// Anyway, it Ammaunts it. 
	int st = 0;
	try 
	{
		m_pFixedFile->unmount();
		++st;//1
		if (m_pVariableFile)
			m_pVariableFile->unmount();
		++st;//2
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		switch (st) 
		{
		case 2:
			if (m_pVariableFile)
				m_pVariableFile->mount();
			//fall thru
		case 1:
			m_pFixedFile->mount();
			//fall thru
		default:
			;//nop
		}
		_SYDNEY_RETHROW;
	}

	; _SYDNEY_ASSERT(!isMounted(cTrans_));

	// The Unmaunt is recorded in the file identifier.
	m_cFileID.setBoolean(
		_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key),
		false);

	return m_cFileID;
}

//
//	FUNCTION public
//	Record2::LogicalInterface::flush -- the record file is flashed. 
//
//	NOTES
//	the record file is flashed。
//
//	ARGUMENTS
//	const Trans::Transaction&	cTrans_
//		transaction descriptor
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	[YET!]
//
void
LogicalInterface::
flush(const Trans::Transaction&	cTrans_)
{
	TRACEMSG("Record2::LogicalInterface::flush");
	// The object to access the file for fixed length/variable-length is made. 
	initializeFiles(cTrans_);

	if (isMounted(cTrans_) == true)
	{
		m_pFixedFile->flush();
		if (m_pVariableFile) 
		{
			m_pVariableFile->flush();
		}
	}
}

//
//	FUNCTION public
//	Record2::LogicalInterface::startBackup --
//		The backup beginning is notified to the record file. 
//
//	NOTES
//	The backup beginning is notified to the record file. 
//
//	ARGUMENTS
//	const Trans::Transaction&	cTrans_
//		Reference to transaction descriptor
//	const bool					Restorable_
//		It is enabled that it is a change for the version to become the latest version. 
//			true  : When you Restore it the content backed up.
//			       The change is enabled so that the version to which 
//					the transaction only for the readout begun at the point 
//					that a certain time stamp shows refers may become the latest version. 
//			false : It makes it to a possible recovery at the time of 
//					that begins to be backed up when the content backed up is Ristoaed. 
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	[YET!]
//
void
LogicalInterface::
startBackup(const Trans::Transaction&	cTrans_,
			const bool				Restorable_)
{
	TRACEMSG("Record2::LogicalInterface::startBackup");
	// The object to access the file for fixed length/variable-length is made. 
	initializeFiles(cTrans_);

	if (isMounted(cTrans_) == true)
	{
		int st = 0;
		try 
		{
			m_pFixedFile->startBackup(Restorable_);
			++st;//1
			if (m_pVariableFile) 
			{
				m_pVariableFile->startBackup(Restorable_);
			}
			++st;//2
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			switch (st) 
			{
			case 2:
				if (m_pVariableFile) 
				{
					m_pVariableFile->endBackup();
				}
				//fall thru
			case 1:
				m_pFixedFile->endBackup();
				//fall thru
			case 0:
			default:
				;//nop
			}
			_SYDNEY_RETHROW;
		}
	}
}

//
//	FUNCTION public
//	Record2::LogicalInterface::endBackup --
//		The backup end is notified to the record file. 
//
//	NOTES
//	The backup end is notified to the record file. 
//
//	ARGUMENTS
//	const Trans::Transaction&	cTrans_
//		Reference to transaction descriptor
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	[YET!]
//
void
LogicalInterface::
endBackup(const Trans::Transaction&	cTrans_)
{
	TRACEMSG("Record2::LogicalInterface::endBackup");
	// The object to access the file for fixed length/variable-length is made. 
	initializeFiles(cTrans_);

	if (isMounted(cTrans_) == true)
	{
		m_pFixedFile->endBackup();
		if (m_pVariableFile) 
		{
			m_pVariableFile->endBackup();
		}
	}
}

//
//	FUNCTION public
//	Record2::LogicalInterface::recover --
//		The record file is recovered in the trouble.
//
//	NOTES
//	The record file recovers in the trouble.
//
//	ARGUMENTS
//	const Trans::Transaction&	cTrans_
//		Reference to transaction descriptor
//	const Trans::TimeStamp&		cPoint_
//		Time stamp of point that returns version file
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	[YET!]
//
void
LogicalInterface::
recover(const Trans::Transaction&	 cTrans_,
		const Trans::TimeStamp& cPoint_)
{
	TRACEMSG("Record2::LogicalInterface::recover");
	// The object to access the file for fixed length/variable-length is made. 
	initializeFiles(cTrans_);

	if (isMounted(cTrans_) == true)
	{
		m_pFixedFile->recover(cPoint_);
		if (m_pVariableFile) 
		{
			m_pVariableFile->recover(cPoint_);
		}
	}
}

//
//	FUNCTION public
//	Record2::LogicalInterface::restore --
//		The version file is changed so that 
//		the version to which the transaction only for the readout begun 
//		at the point that a certain time stamp shows refers may become the latest version. 
//
//	NOTES
//	The version file is changed so that the version to 
//	which the transaction only for the readout begun at the point 
//	that a certain time stamp shows refers may become the latest version. 
//
//	ARGUMENTS
//	const Trans::Transaction&	cTrans_
//		Reference to transaction descriptor
//	const Trans::TimeStamp&		cPoint_
//		The version file is changed so that the version to 
//		which the transaction only for the readout begun at the point 
//		that a certain time stamp shows refers may become the latest version. 
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	[YET!]
//
void
LogicalInterface::
restore(const Trans::Transaction& cTrans_,
		const Trans::TimeStamp& cPoint_)
{
	TRACEMSG("Record2::LogicalInterface::restore");
	// The object to access the file for fixed length/variable-length is made. 
	initializeFiles(cTrans_);

	if (isMounted(cTrans_) == true)
	{
		m_pFixedFile->restore(cPoint_);
		if (m_pVariableFile) 
		{
			m_pVariableFile->restore(cPoint_);
		}
	}
}

//
//	FUNCTION public
//	Record2::LogicalInterface::verify -- The correspondence is inspected. 
//
//	NOTES
//	The correspondence is inspected. 
//
//	ARGUMENTS
//	const Trans::Transaction&		cTrans_
//		Reference to transaction descriptor
//	const unsigned int				uiTreatment_
//		Inspection method of correspondence inspection
//		const Admin::Verification::Treatment::Value
//		const unsigned intValue that is Cast
//	Admin::Verification::Progress&	cProgress_
//		Reference to process of correspondence inspection
//
//	RETURN
//	none
//
//	EXCEPTIONS
//	[YET!]
//
void
LogicalInterface::
verify(const Trans::Transaction& cTrans_,
	   const unsigned int uiTreatment_,
	   Admin::Verification::Progress& cProgress_)
{
	TRACEMSG("Record2::LogicalInterface::verify");
	if (! cProgress_.isGood()) return;

	// The object to access the file for fixed length/variable-length is made. 
	initializeFiles(cTrans_);

	// A physical file is opened. 
	m_pFixedFile->attachFile(false);
	if (m_pVariableFile) m_pVariableFile->attachFile(false);
	//AutoAttachFile autoFixedFile(*m_pFixedFile);
	//maybe needn't variable file
	//AutoAttachFile autoVariableFile(m_pVariableFile);

	// initialize ObjectManager
	initializeManagers(false, true);
	
	if (!initializeDataAccess(0, FileCommon::OpenMode::Read))
	{
		// An open mode is wrong. 
		_SYDNEY_THROW0(Exception::IllegalFileAccess);
	}
	
	//detach by hand
	//AutoDetacher detacher(m_pObjectManager, m_pAreaManager);
	m_pObjectManager->setDataAccess(m_pDataAccess);

	bool bNeedToEnd = false;
	try
	{
		// The correspondence to a physical file is inspected. 
		_SYDNEY_VERIFY_INFO(cProgress_, m_cFileID.getDirectoryPath(), Message::VerifyPhysicalFileStarted(), uiTreatment_);
		//start verification
		m_pFixedFile->startVerification(uiTreatment_, cProgress_);
		if(m_pVariableFile) m_pVariableFile->startVerification(uiTreatment_, cProgress_);
		bNeedToEnd = true;

		// The correspondence to a physical file is inspected. 
		Utility::ObjectNum uiInsertedObjectNum = m_pObjectManager->verifyBody(uiTreatment_, cProgress_);

		if(m_pVariableFile) m_pVariableFile->endVerification(cProgress_);
		m_pFixedFile->endVerification(cProgress_);
		bNeedToEnd = false;

		_SYDNEY_VERIFY_INFO(cProgress_, m_cFileID.getDirectoryPath(), Message::VerifyPhysicalFileFinished(), uiTreatment_);
		if (cProgress_.isGood()) 
		{
			_SYDNEY_VERIFY_INFO(cProgress_, m_cFileID.getDirectoryPath(), Message::VerifyContentStarted(), uiTreatment_);
			// The correspondence to contents of the file is inspected. 
			m_pObjectManager->verifyHeader(uiInsertedObjectNum, uiTreatment_, cProgress_);
			_SYDNEY_VERIFY_INFO(cProgress_, m_cFileID.getDirectoryPath(), Message::VerifyContentFinished(), uiTreatment_);
		}

		//free resource
		close();
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		if (bNeedToEnd) 
		{
			// The end of the correspondence inspection is directed to a physical file manager. 
			if(m_pVariableFile) m_pVariableFile->endVerification(cProgress_);
			m_pFixedFile->endVerification(cProgress_);
		}

		//free resource
		close();

		_SYDNEY_VERIFY_ABORTED(cProgress_,
							   m_cFileID.getDirectoryPath(),
							   Message::VerifyFailed());
		_SYDNEY_RETHROW;
	}
}

//
// PRIVATE METHOD
//

//
//	FUNCTION private
//	Record2::LogicalInterface::convertFetchOptionToObjectID --
//		Conversion from option to fetch OID
//
//	NOTES
//	It is called from Record2::LogicalInterface::fetch, 
//  and this function analyzes argument pOption _, and returns set object ID. 
//
//	ARGUMENTS
//	const Common::DataArrayData*	pOption_
//		Record2::LogicalInterface::fetch : Option
//
//	RETURN
//	ModUInt64
//		Object ID However, when argument pOption _ is a null pointer, 
//		UndefinedObject ID is returned because it is an access in  Scan mode. 
//
//	EXCEPTIONS
//	BadArgument
//		BadArgument
//	[YET!]

ObjectID::Value
LogicalInterface::convertFetchOptionToObjectID(const Common::DataArrayData* pOption_) const
{
	TRACEMSG("Record2::LogicalInterface::convertFetchOpt");

	// There must have to be one element. 
	if (!pOption_ || pOption_->getCount() != 1)
		_SYDNEY_THROW0(Exception::BadArgument);


	// The array element should be an instance object of the LogicalFile::ObjectID(=Common::ObjectIDData) class. 
	//const Common::DataArrayData::Pointer& pElement = pOption_->getElement(0);
	if (!ObjectID::isObjectIDType(pOption_, 0))
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	const LogicalFile::ObjectID* pIDData =
		_SYDNEY_DYNAMIC_CAST(const LogicalFile::ObjectID*, pOption_->getElement(0).get());
	; _SYDNEY_ASSERT(pIDData);

	return pIDData->getValue();
}

//
//	FUNCTION private
//	Record2::LogicalInterface::saveOpenOption -- 
//		An open option is preserved in the data member. 
//
//	NOTES
//	An open option is stored in the member variable and projection information. 
//
//	ARGUMENTS
//	const LogicalFile::OpenOption&	cOpenOption_
//		Reference to open option
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	[YET!]
//
void
LogicalInterface::
saveOpenOption(const LogicalFile::OpenOption& cOpenOption_ ,int& iOpenModeValue_)
{
	TRACEMSG("Record2::LogicalInterface::saveOpenOption");
	// An open mode is preserved in the data member. 
	if (iOpenModeValue_ == FileCommon::OpenOption::OpenMode::Read)
	{
		m_cOpenParam.m_iOpenMode = FileCommon::OpenMode::Read;
	}
	else if (iOpenModeValue_ == FileCommon::OpenOption::OpenMode::Update)
	{
		m_cOpenParam.m_iOpenMode = FileCommon::OpenMode::Update;
	}
	else if (iOpenModeValue_ == FileCommon::OpenOption::OpenMode::Batch)
	{
		m_cOpenParam.m_iOpenMode = FileCommon::OpenMode::Batch;
	}
	else
	{
		// Illegal argument
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// The estimate specification is preserved in the data member. 
	if (!cOpenOption_.getBoolean(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::Estimate::Key), m_cOpenParam.m_bEstimate))
	{
		// When the user is not setting up the value, the default value is set. 
		m_cOpenParam.m_bEstimate = false;
	}

	// set selected fields and other info
	m_cOpenParam.m_bFieldSelect = m_cFileID.resetTargetFields(cOpenOption_, m_cOpenParam.m_iOpenMode);

	// Sorting order
	bool bSortOrder;
	if ( cOpenOption_.getBoolean(_SYDNEY_RECORD2_OPEN_PARAMETER_KEY(Record2::OpenOption::SortOrder::Key), bSortOrder) )
	{
		m_cOpenParam.m_bSortOrder = bSortOrder;
	}
	else
	{
		// The default value is false. (ascending order)
		m_cOpenParam.m_bSortOrder = false;
	}
}

//	FUNCTION private
//	Record2::LogicalInterface::initializeFiles -- 
//		The class that shows the file for fixed length/variable-length is generated. 
//
//	NOTES
//
//	ARGUMENTS
//		const Trans::Transaction& cTrans_
//			Operated transaction descriptor
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
LogicalInterface::
initializeFiles(const Trans::Transaction& cTrans_)
{
	// The FileID is initialized. 
	m_cFileID.initialize();
	
	// The object to access the file for fix-length is made. 
	if (!m_pFixedFile) 
	{
		m_pFixedFile = new FileAccess(cTrans_, m_cFileID, false);
	}
	if (m_cFileID.hasVariable() && !m_pVariableFile) 
	{
		// The object to access the file for variable-length is made. 
		m_pVariableFile = new FileAccess(cTrans_, m_cFileID, true);
	}
}

//	FUNCTION private
//	Record2::LogicalInterface::initializeManagers -- 
//		
//
//	NOTES
//		Create ObjectManager and AreaManager
//		Load managers' initialize data
//	ARGUMENTS
//		const Trans::Transaction& cTrans_
//			Operated transaction descriptor
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
LogicalInterface::
initializeManagers(bool bForUpdate_, bool bNeedDoArea_/* = false*/, bool bSubstance_/* = false*/)
{
	; _SYDNEY_ASSERT(m_pFixedFile/* && m_pFixedFile->isMounted(cTrans_)*/);

	if (!m_pObjectManager) 
	{
		Manager::Operation::Value eValue = Manager::Operation::Read;
		if(bForUpdate_)
		{
			eValue = m_pFixedFile->isBatch() ? Manager::Operation::Batch : Manager::Operation::Update;
		}

		m_pObjectManager = new ObjectManager(*m_pFixedFile, eValue);

		//substance file
		if(bSubstance_) m_pObjectManager->substanceFile();
		
		m_pObjectManager->reload(bForUpdate_);
	} 
	else 
	{
		//should reset physicalfile only for verification
		//m_pObjectManager->setPhysicalFile(m_pFixedFile->getFile());
		// Every time, it tries to read for the transaction for reading and writing. 
		if(bForUpdate_) 
		{
			m_pObjectManager->reload(true);
		}
		else if (m_pFixedFile->getTransaction().getCategory() == Trans::Transaction::Category::ReadWrite) 
		{
			m_pObjectManager->reload(false /* do not repair */);
		}
	}

	if(bNeedDoArea_ && m_cFileID.hasVariable())
	{
		; _SYDNEY_ASSERT(m_pVariableFile/* && m_pVariableFile->isMounted(cTrans_)*/);

		if (!m_pAreaManager) 
		{
			m_pAreaManager = new AreaManager(*m_pVariableFile);
			//substance file
			if(bSubstance_) m_pAreaManager->substanceFile();
		}
		//should reset physicalfile only for verification
		//else m_pAreaManager->setPhysicalFile(m_pVariableFile->getFile());
	}
}

//	FUNCTION private
//	Record2::LogicalInterface::initializeDataAccess -- 
//		
//
//	NOTES
//
//	ARGUMENTS
//		const Trans::Transaction& cTrans_
//			Operated transaction descriptor
//
//	RETURN
//		None
//
//	EXCEPTIONS

bool
LogicalInterface::
initializeDataAccess(Common::DataArrayData* pTuple_, FileCommon::OpenMode::Mode iOpenMode)
{
	if (m_cOpenParam.m_iOpenMode != iOpenMode) 
		return false;

	// The object to access the data is made. 
	if (!m_pDataAccess) 
	{
		m_pDataAccess = new DataAccess(m_pObjectManager, m_pAreaManager, pTuple_);

		//initializeFieldData
		m_pDataAccess->initializeFieldData(
			m_cFileID.getTargetFields(false),
			m_cFileID.getTargetFields(true),
			m_cFileID.hasVariable());
	}
	else m_pDataAccess->resetTuple(pTuple_);

	return true;
}

//	FUNCTION private
//	Record2::LogicalInterface::isPrepared -- 
//		
//
//	NOTES
//		verify file opened or not
//		file attached or not
//		data access prepared or not
//	ARGUMENTS
//
//	RETURN
//		None
//
//	EXCEPTIONS

bool 
LogicalInterface::
isPrepared(bool bIsVariable_/* = false*/, bool bData_/* = false*/) const
{
	if(!bIsVariable_)
	{
		//file attached or not
		if (!m_pFixedFile || !m_pFixedFile->isAttached()) 
			return false;
		
		//page attached or not
		if (bData_ && !m_pObjectManager)
			return false;
	}
	else
	{
		//file attached or not
		if (!m_pVariableFile || !m_pVariableFile->isAttached()) 
			return false;
		
		//area attached or not
		if (bData_ && !m_pAreaManager)
			return false;
	}
	
	return true;
}

//
//	Copyright (c) 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
