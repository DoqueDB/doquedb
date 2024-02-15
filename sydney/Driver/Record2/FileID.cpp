// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.cpp -- Cpp File of FileID class which operate schema information for a table
// 
// Copyright (c) 2006, 2007, 2017, 2023 Ricoh Company, Ltd.
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

#include "Common/Message.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "FileCommon/OpenOption.h"

#include "Os/Limits.h"

#include "Record2/Parameter.h"
#include "Record2/FieldInfo.h"
#include "Record2/FileID.h"
//!!!this file header can't move up
#include "Record2/FileOption.h"
#include "Record2/VariableData.h"

_SYDNEY_USING
_SYDNEY_RECORD2_USING

namespace
{
	// Key word of hint
	const ModUnicodeString _HintCompressed("compressed");
	const ModUnicodeString _HintFixedSize("fixed");

	// Physical file type
	const PhysicalFile::Type _eFixedFileType = PhysicalFile::PageManageType;
	const PhysicalFile::Type _eVariableFileType = PhysicalFile::AreaManageType;

	//Number of bits used for bit map per byte
	const int _iBitsPerByte = 8;

	//
	const PhysicalAreaID _AreaIDMaxValue = Common::ObjectIDData::getMaxLatterValue();

	// Setting for MinimumObjectPerPage
	ParameterIntegerInRange _cMinimumObjectPerPage(
		"Record2_MinimumObjectPerPage",
		4,	// default
		4,	// min
		_AreaIDMaxValue,	// max
		false);
}

//
//	FUNCTION public
//	Record2::FileID::FileID -- copy constructor
//
//	NOTES
//		copy constructor
//
//	ARGUMENTS
//		Logical FileID is transfered from upper module
//
//	RETURN
//
//	EXCEPTIONS
//	
//
FileID::
FileID(const LogicalFileID& cLogicalFileID_)
	: LogicalFileID(cLogicalFileID_),	//Copy
	  m_uiObjectNumberPerPage(0),
	  m_uiFixedPageSize(0),
	  m_uiVariablePageSize(0),
	  m_bTemporary(false),
	  m_bReadOnly(false),
	  m_bCompress(false),
	  m_bHasVariable(false),
	  m_bIsVariableUsed(false),
	  m_bInitialized(false),
	  m_iVersion(Version::CurrentVersion)
{
//	m_cFixedTargetFields.m_vecFieldInfo.reserve(0);
//	m_cVariableTargetFields.m_vecFieldInfo.reserve(0);
}

//
//	FUNCTION public
//	Record2::FileID::~FileID -- destructor
//
//	NOTES
//		destructor
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//	
//
FileID::
~FileID()
{
	//release resource: fixed field information
	Utility::FieldNum fieldLength = m_cFixedTargetFields.getSize();
	for(Utility::FieldNum i = 0; i < fieldLength; ++ i)
	{
		delete m_cFixedTargetFields[i];
	}
	//maybe needn't do it
	m_cFixedTargetFields.clear();

	//release resource: variable field information
	fieldLength = m_cVariableTargetFields.getSize();
	for(Utility::FieldNum i = 0; i < fieldLength; ++ i)
	{
		delete m_cVariableTargetFields[i];
	}
	//maybe needn't do it
	m_cVariableTargetFields.clear();
}

//
//
//	FUNCTION public
//	Record2::FileID::create -- FileID is made. 
//
//	NOTES
//		only be called when creating a table
//
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS
//
void
FileID::
create()
{
	//first of all, we should set the file version with Versino5
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(
			FileCommon::FileOption::Version::Key), Version::CurrentVersion);

	// get all field count. 
	Utility::FieldNum iFieldNum = getInteger(_SYDNEY_FILE_PARAMETER_KEY(
									FileCommon::FileOption::FieldNumber::Key));

	// set local member and initialize it
	// avoid re-allocate them
	bool fixed = false;
	bool compress = false;
	Common::DataType::Type	fieldType;
	Utility::FieldLength fieldLength = 0;

	// only set fixed, compress and length, others will be set in initialization
	for (Utility::FieldNum i = 0; i < iFieldNum; ++i)
	{
		//get field type, compress, fixed and length
		fieldLength = makeFieldInfo(i, true, fixed, compress, fieldType);

		// set them to FileID.
		setBoolean(	_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				FileCommon::FileOption::FieldFixed::Key, i), fixed);

		setBoolean(	_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				Record2::FileOption::Compressed::Key, i), compress);

		setInteger(	_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				FileCommon::FileOption::FieldLength::Key, i), fieldLength);
	}

	//set others information
	initialize();
}

//
//
//	FUNCTION public
//	Record2::FileID::initialize -- FileID is initialized. 
//
//	NOTES
//		including FieldInfo, ObjectSize, PageSize...
//
//	ARGUMENTS
//	None
//
//	RETURN
//	None
//
//	EXCEPTIONS
//
void
FileID::
initialize()
{
	//have initialized
	if(m_bInitialized) return;

	//must equal to zero
	; _SYDNEY_ASSERT(m_cFixedTargetFields.getSize() == 0);
	; _SYDNEY_ASSERT(m_cVariableTargetFields.getSize() == 0);

	//get file version
	if (!getInteger(_SYDNEY_FILE_PARAMETER_KEY(
		FileCommon::FileOption::Version::Key), m_iVersion))
		m_iVersion = Version::CurrentVersion;

	// get all field count. 
	Utility::FieldNum iFieldNum = getInteger(_SYDNEY_FILE_PARAMETER_KEY(
									FileCommon::FileOption::FieldNumber::Key));

	//temporary variable avoid re-allocate them
	bool fixed = false;
	bool compress = false;
	Common::DataType::Type fieldType;
	Utility::FieldLength fieldLength = 0;
	FieldInfo* pFieldInfo = 0;

	for (Utility::FieldNum i = 0; i < iFieldNum; ++i)
	{
		//get field type, compress, fixed and length
		fieldLength = makeFieldInfo(i, false, fixed, compress, fieldType);

		// acquire the encoding method of the field
		int iTemp = 0;
		Common::StringData::EncodingForm::Value eFieldEncodingForm =
			(getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
			FileCommon::FileOption::FieldEncodingForm::Key, i), iTemp)) ?
			((static_cast<Common::StringData::EncodingForm::Value>(iTemp)
			 == Common::StringData::EncodingForm::UTF8) ?
			Common::StringData::EncodingForm::Unknown :
			static_cast<Common::StringData::EncodingForm::Value>(iTemp)) :
			Common::StringData::EncodingForm::UCS2;

		//create fieldInfo
		pFieldInfo = new FieldInfo(i, fieldType, fieldLength, eFieldEncodingForm, !fixed, compress);

		//set for Decimal Data scale
		int fieldScale = 0;
		if(getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
			FileCommon::FileOption::FieldFraction::Key,i), fieldScale))
			pFieldInfo->setFraction(fieldScale);

		//***********************Element Info Begin**********************************************

		// It's variable and array
		if (!fixed) 
		{
			// Variable-length or array
			; _SYDNEY_ASSERT(!Common::Data::isFixedSize(fieldType));

			if (fieldType == Common::DataType::Array) // Array
			{
				//the number of elements is specified by FieldLength. 
				makeElementInfo(pFieldInfo, fieldLength);
			}

			m_cVariableTargetFields.pushBack(pFieldInfo);
			//has variable fields, do it by adjustTargetFields
			m_bHasVariable = m_bIsVariableUsed = true;
		}
		else m_cFixedTargetFields.pushBack(pFieldInfo);
		//***********************Element Info End**********************************************
	}

	// get the fixed objectsize
	makeFixedObjectSize();

	// page size etc. are set to the file-identification.
	makeFixedPageSize();
	makeVariablePageSize();

	//set path
	getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
		FileCommon::FileOption::Area::Key,0), m_cPath);

	//set temporary
	getBoolean(_SYDNEY_FILE_PARAMETER_KEY(
		FileCommon::FileOption::Temporary::Key), m_bTemporary);

	// Whether ReadOnly is set to the file option or not?
	getBoolean(_SYDNEY_FILE_PARAMETER_KEY(
		FileCommon::FileOption::ReadOnly::Key), m_bReadOnly);

	//has initialized
	m_bInitialized = true;
}

//	FUNCTION public
//	Record2::FileID::equals -- equals
//
//	NOTES
//		Inspection of equivalence
//		Only the meta data is compared. Therefore, logical file-identifications are not compared.
//
//	ARGUMENTS
//		another fileid
//
//	RETURN
//		true: equal; false: not equal
//
//	EXCEPTIONS

bool
FileID::
equals(const FileID& cFileID_) const
{
	// Only the directory path names are compared. 
	return m_cPath.compare(
		getDirectoryPath()) == Os::Path::CompareResult::Identical;
}

//
//	FUNCTION public
//	Record2::FileID::getLockName -- The lock name is obtained. 
//
//	NOTES
//
//	ARGUMENTS
//	None
//
//	RETURN
//	const Lock::FileName&
//		lock name
//
//	EXCEPTIONS
//
const Lock::FileName&
FileID::
getLockName() const
{
	if (m_cLockName.getDatabasePart() == ~static_cast<Lock::Name::Part>(0))
	{
		m_cLockName = FileCommon::IDNumber(*this).getLockName();
	}
	return m_cLockName;
}

// FUNCTION public
//	Record2::FileID::checkVersion -- check whether fileid has version for record2
//
// NOTES
//
// ARGUMENTS
//	const LogicalFile::FileID& cLogicalFileID_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
FileID::
checkVersion(const LogicalFile::FileID& cLogicalFileID_)
{
	int iVersion;
	if (!cLogicalFileID_.getInteger(_SYDNEY_FILE_PARAMETER_KEY(
			FileCommon::FileOption::Version::Key), iVersion))
		return true;
	return (iVersion >= Version::Version5) ? true : false;
	//return true;	//just for testing
}

//
//	FUNCTION public
//	Record2::FileID::isMounted -- Is the mount done?
//
//	NOTES
//
//	ARGUMENTS
//	None
//
//	RETURN
//	bool
//		False in case of mount isn't done
//
//	EXCEPTIONS
//
bool
FileID::isMounted() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(
			FileCommon::FileOption::Mounted::Key));
}

//	FUNCTION private
//	Record2::FileID::makeFixedPageSize -- make fixed page size
//
//	NOTES
//		--  make fixed page size
//
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
FileID::
makeFixedPageSize() 
{
	int iValue = 0;
	if (getInteger(_SYDNEY_FILE_PARAMETER_KEY(Record2::FileOption::DirectPageSize::Key), iValue) ||
		getInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key), iValue))
	{
		// Kbyte -> byte
		m_uiFixedPageSize = iValue << 10;
	}
	else 
	{
		// It requests it by the calculation. 
		calculateFixedPageSize();

		// set it in FileID. 
		// byte -> Kbyte
		iValue = m_uiFixedPageSize >> 10;
		setInteger(_SYDNEY_FILE_PARAMETER_KEY(Record2::FileOption::DirectPageSize::Key), iValue);
	}

	// The number of objects is calculated again here even if it is set to FileID or it calculates. 
	PhysicalFile::PageSize iAvailableSize =
			PhysicalFile::File::getPageDataSize(_eFixedFileType, m_uiFixedPageSize);

	// Because the number of objects is used for the header on each page, 
	// it decreases it from the size that can be used. 
	iAvailableSize -= sizeof(Utility::Size);

	m_uiObjectNumberPerPage =
		(iAvailableSize * _iBitsPerByte - _iBitsPerByte + 1)
		/ (m_uiFixedObjectSize * _iBitsPerByte + 1);
}

//	FUNCTION private
//	Record2::FileID::calculateFixedPageSize -- calculate fixed page size
//
//	NOTES
//		-- make fixed page size
//
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
FileID::
calculateFixedPageSize()
{
	// The setting concerning the minimum values of the number of objects per page is obtained. 
	int iValue = 0;
	int iMinimumValue = 0;
	// When it is smaller than minimum value, it disregards it. 
	if (getInteger(_SYDNEY_FILE_PARAMETER_KEY(Record2::FileOption::MinimumObjectPerPage::Key), iValue)
		&& iValue >= _cMinimumObjectPerPage.get())
		iMinimumValue = iValue;
	else 
	{
		iMinimumValue = _cMinimumObjectPerPage.get();

		// It sets it in FileID. 
		setInteger(_SYDNEY_FILE_PARAMETER_KEY(Record2::FileOption::MinimumObjectPerPage::Key), iMinimumValue);
	}

	// Page size is calculated in the parts except the overhead. 
	Utility::Size defaultPageSize	= FileCommon::FileOption::PageSize::getDefault();
	Utility::Size iPageSize = defaultPageSize;

	PhysicalFile::PageSize iAvailableSize =
		PhysicalFile::File::getPageDataSize(_eFixedFileType, iPageSize);

	// Because the number of objects is used for the header on each page, 
	// it decreases it from the size that can be used. 
	iAvailableSize -= sizeof(Utility::Size);

	if (iAvailableSize / iMinimumValue < m_uiFixedObjectSize)
	{
		// IMinimumValue twice the size of the object are exceeded. 
		// It makes it to default page size multiple. 
		// It is confirmed that the calculation result doesn't exceed the maximum value of the type. 
		; _SYDNEY_ASSERT(Os::Limits<PhysicalFile::PageSize>::IsSpecialized
						 && (m_uiFixedObjectSize
							 < (Os::Limits<PhysicalFile::PageSize>::getMax()
								/ iMinimumValue - defaultPageSize)));

		PhysicalFile::PageSize iRequiredSize = m_uiFixedObjectSize * iMinimumValue;
		iPageSize = iRequiredSize - (iRequiredSize % defaultPageSize) + defaultPageSize;

		// The size that can be used in page new size is obtained. 
		iAvailableSize = PhysicalFile::File::getPageDataSize(_eFixedFileType, iPageSize);
	}

	// The number of objects per page is calculated. 

	// Maximum x that fills number of bytes >= (x+7)/8+ x*ObjectSize 
	// that can be used is a number of objects. 
	// --> 8*avail >= x+7+x*size*8
	// --> 8*avail - 7 >= x(size*8+1)
	int iObjectPerPage =
		(iAvailableSize * _iBitsPerByte - _iBitsPerByte + 1)
		/ (m_uiFixedObjectSize * _iBitsPerByte + 1);

	; _SYDNEY_ASSERT(iObjectPerPage > 0);

	// Because it is an error margin part of the header of which number ..
	// defaultPageSize.. increased, when falling below minimum value as a 
	// result of the calculation, it exceeds it by  
	if (iObjectPerPage < iMinimumValue)
		iPageSize += defaultPageSize;

	// Page size was fixed.
	m_uiFixedPageSize = iPageSize;
}

//	FUNCTION private
//	Record2::FileID::makeVariablePageSize -- makeVariablePageSize
//
//	NOTES
//		-- makeVariablePageSize
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
FileID::
makeVariablePageSize()
{
	int iValue;
	if (getInteger(_SYDNEY_FILE_PARAMETER_KEY(Record2::FileOption::VariablePageSize::Key), iValue))
		// From Version4, VariablePageSize does not refer PageSize::Key
	{
		// get the value from FileID
		// Kbyte -> byte
		m_uiVariablePageSize = iValue << 10;
	}
	else 
	{
		// calculate proper page size by calculation

		// add all the limit length
		Utility::Size uiSize = 0;
		Utility::FieldNum n = m_cVariableTargetFields.getSize();
		Utility::Size uiHeaderSize = Utility::m_AreaIDNumArchiveSize;
		//suppose variable field be devided into 2 DirectArea 
		uiHeaderSize += ObjectID::m_ArchiveSize;
		//get position size
		uiHeaderSize += VariableData::getPositionSize(n, false);
		uiSize += uiHeaderSize;

		for (Utility::FieldNum i = 1; i < n; ++i) 
		{
			// data length
			uiSize += m_cVariableTargetFields[i]->getLength();
		}
		
		if(Os::Limits<int>::getMax() < uiSize) 
		{
			// unlimited -> use max value
			uiSize = Os::Limits<int>::getMax();
		}

		// get available size
		Os::Memory::Size defaultPageSize
			= FileCommon::FileOption::PageSize::getDefault();
		PhysicalFile::PageSize iAvailableSize =
			PhysicalFile::File::getPageDataSize(_eVariableFileType, uiSize);
		// make size multiple of defaultPageSize
		PhysicalFile::PageSize iPageSize = iAvailableSize - (iAvailableSize % defaultPageSize) + defaultPageSize;
		m_uiVariablePageSize = iPageSize;

		// Set the value to FileID
		// byte -> Kbyte
		iValue = m_uiVariablePageSize >> 10;
		setInteger(_SYDNEY_FILE_PARAMETER_KEY(Record2::FileOption::VariablePageSize::Key), iValue);
	}
}

//	FUNCTION private
//	Record2::FileID::makeFixedObjectSize -- make fixed object size
//
//	NOTES
//		-- make fixed object size
//
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
FileID::
makeFixedObjectSize()
{
	// Because the size of the object is able not to be controlled by giving 
	// it from the high rank, it always calculates. 
	Utility::Size iObjectSize = 0;

	//record linked ObjectID
	iObjectSize += ObjectID::m_ArchiveSize;
	
	//TrasactionID
	iObjectSize += Utility::m_TransIDArchiveSize;

	// The size of object ID to indicate it is added if there is variable-length. 
	if (hasVariable()) 
	{
		iObjectSize += ObjectID::m_ArchiveSize;

	} 
	else 
	{
		// > m_ObjectIDArchiveSize forever
	}

	Utility::FieldNum n = m_cFixedTargetFields.getSize();
	// The number of fields in which the size for null bitmap is 
	// obtained subtracts the amount of ObjectID. 
	//iObjectSize += FieldData::getBitmapSize(n - 1);
	iObjectSize += FieldData::getBitmapSize(n);

	// The size of the fixed-length field is obtained. 
	for (Utility::FieldNum i = 0; i < n; ++i) 
	{
		iObjectSize += m_cFixedTargetFields[i]->getLength();
	}


	m_uiFixedObjectSize = iObjectSize;
}

//	FUNCTION private
//	Record2::FileID::getFieldInfos -- getFieldInfos
//
//	NOTES
//		-- getFieldInfos
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

const FileID::TargetFields&
FileID::
getTargetFields(bool bIsVariable_) const
{
	return bIsVariable_ ? m_cVariableTargetFields : m_cFixedTargetFields;
}

//	FUNCTION private
//	Record2::FileID::resetTargetFields -- resetTargetFields
//
//	NOTES
//		-- reset TargetFields, remove the unwanted field info and add some extra infomation
//
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

bool  
FileID::
resetTargetFields(const LogicalFile::OpenOption& cOpenOption_, FileCommon::OpenMode::Mode& eOpenMode_)
{
	// All fields are sure to be selected. 
	bool bFieldSelect = false;
	if (!cOpenOption_.getBoolean(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::FieldSelect::Key), bFieldSelect))
	{
		bFieldSelect = false;
	}

	//if is updating and hasn't select fields
	//reset field index & field selected
	adjustTargetFields(!bFieldSelect);

	//use it to judge if has variable
	Utility::FieldNum iVariableNum = 1;
	m_bIsVariableUsed = true;
	//if select or update point to some fields
	if(bFieldSelect)
	{
		; _SYDNEY_ASSERT(eOpenMode_ == FileCommon::OpenMode::Read || eOpenMode_ == FileCommon::OpenMode::Update);

		int iSelectedFieldCount = 0;
		if (! cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::TargetFieldNumber::Key), iSelectedFieldCount))
		{
			// How many did you specify it though were done the field selection specification?
			// The shown value is not set. 
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		//here only set selected, maybe remove the unwanted filedinfo is better.
		//reset it to zero for it's initvalue is one
		iVariableNum = 0;

		int iFieldID = 0, iIndex = 0;
		bool bFind = false;
		for (int i = 0; i < iSelectedFieldCount; ++i)
		{
			//is this fieldid found?
			bFind = cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				FileCommon::OpenOption::TargetFieldIndex::Key, i), iFieldID);

			//if exist set data index
			if (bFind && !setSelectedByFieldID(iFieldID, i)) ++iVariableNum;
		}
	
		//reset has variable fields
		if(iVariableNum == 0 && eOpenMode_ != FileCommon::OpenMode::Update) 
			m_bIsVariableUsed = false;
	}

	return bFieldSelect;
}
//	FUNCTION private
//	Record2::FileID::setSelectedByFieldID -- setSelectedByFieldID
//
//	NOTES
//		-- setSelectedByFieldID
//	ARGUMENTS
//		None
//
//	RETURN
//		false:	fixed field
//		true:  variable field
//
//	EXCEPTIONS

bool  
FileID::
setSelectedByFieldID(const int iFieldID_, const int iDataIndex_)
{
	//find it in fixed fields fristly
	int fieldSize = m_cFixedTargetFields.getSize();
	for(int i = 0; i < fieldSize; ++i)
	{
		if(iFieldID_ == m_cFixedTargetFields[i]->getFieldID())
		{
			m_cFixedTargetFields[i]->setIsSelected(true);
			m_cFixedTargetFields[i]->setDataIndex(iDataIndex_);
			return true;
		}
	}

	//if not find in fixed, we'll search in variable
	fieldSize = m_cVariableTargetFields.getSize();
	for(int i = 0; i < fieldSize; ++i)
	{
		if(iFieldID_ == m_cVariableTargetFields[i]->getFieldID())
		{
			m_cVariableTargetFields[i]->setIsSelected(true);
			m_cVariableTargetFields[i]->setDataIndex(iDataIndex_);
			return false;
		}
	}

	//if can't find it then throw 
	_SYDNEY_THROW0(Exception::BadArgument);
}


//	FUNCTION private
//	Record2::FileID::adjustTargetFields -- adjustTargetFields
//
//	NOTES
//		adjust index and fieldcount
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void 
FileID::
adjustTargetFields(bool bSelected_/* = true*/)
{
	//adjust fixed info
	Utility::FieldNum uiFieldNum = m_cFixedTargetFields.getSize();
	for(Utility::FieldNum i = 0; i < uiFieldNum; ++ i)
	{
		//reset index, it's position of fields
		m_cFixedTargetFields[i]->setFieldIndex(i);
		m_cFixedTargetFields[i]->setIsSelected(bSelected_);
	}

	//adjust variable info
	uiFieldNum = m_cVariableTargetFields.getSize();
	//should load variable file
	for(Utility::FieldNum i = 0; i < uiFieldNum; ++ i)
	{
		//reset index, it's position of fields
		m_cVariableTargetFields[i]->setFieldIndex(i);
		m_cVariableTargetFields[i]->setIsSelected(bSelected_);
	}
}

//	FUNCTION private
//	Record2::FileID::decreaseDataIndex -- decreaseDataIndex
//
//	NOTES
//		-- decreaseDataIndex
//	ARGUMENTS
//		bool bDecreased_:
//			true : dataindex--  false : dataindex++
//
//	RETURN
//		None
//
//	EXCEPTIONS

void 
FileID::
adjustDataIndex(bool bDecreased_)
{
	int iDelta = bDecreased_ ? -1 : 1;
	//adjust fixed info
	Utility::FieldNum uiFieldNum = m_cFixedTargetFields.getSize();
	//if has been decreased, don't do it again
	for(Utility::FieldNum i = 0; i < uiFieldNum; ++ i)
	{
		if(m_cFixedTargetFields[i]->getFieldIndex() == 0) continue;
		//increase data index
		m_cFixedTargetFields[i]->setDataIndex(
			m_cFixedTargetFields[i]->getDataIndex() + iDelta);
	}

	//adjust variable info
	uiFieldNum = m_cVariableTargetFields.getSize();
	//should load variable file
	for(Utility::FieldNum i = 0; i < uiFieldNum; ++ i)
	{
		//increase data index
		m_cVariableTargetFields[i]->setDataIndex(
			m_cVariableTargetFields[i]->getDataIndex() + iDelta);
	}

}

//	FUNCTION private
//	Record2::FileID::getFieldIDbyDataIndex -- getFieldIDbyDataIndex
//
//	NOTES
//		get fieldid by data index
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS


int 
FileID::
getFieldIDbyDataIndex(int iDataIndex_)
{
	//adjust fixed info
	Utility::FieldNum uiFieldNum = m_cFixedTargetFields.getSize();
	for(Utility::FieldNum i = uiFieldNum - 1; i >= 0; --i)
	{
		//compare data index
		if(m_cFixedTargetFields[i]->getDataIndex() == iDataIndex_)
			return m_cFixedTargetFields[i]->getFieldIndex();
	}

	//adjust variable info
	uiFieldNum = m_cVariableTargetFields.getSize();
	//should load variable file
	for(Utility::FieldNum i = uiFieldNum - 1; i >= 0; --i)
	{
		//compare data index
		if(m_cVariableTargetFields[i]->getDataIndex() == iDataIndex_)
			return m_cVariableTargetFields[i]->getFieldIndex();
	}

	return -1;
}


//	FUNCTION private
//	Record2::FileID::makeFieldInfo -- makeFieldInfo
//
//	NOTES
//		-- makeFieldInfo
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

Utility::FieldLength
FileID::
makeFieldInfo(int iFieldID_, bool bCreated_, bool& bFixed_, bool& bCompress_, Common::DataType::Type& fieldType)
{
	// 1. get fieldtype
	int	iDummy;
	if(!getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
		 FileCommon::FileOption::FieldType::Key, iFieldID_),	iDummy))
	{
		// There is no default value in the field type. 
		// It sends it the exception if it doesn't set it. 
		SydErrorMessage
			<< "FIELD[" << iFieldID_ << "] : must be set FieldType"
			<< ModEndl;
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	fieldType = static_cast<Common::DataType::Type>(iDummy);

	if (bCreated_ && iFieldID_ == 0)
	{
		// This field must be absolutely object ID. 
		if (fieldType != LogicalFile::ObjectID().getType())
		{
			// The data type is strange. 
			SydErrorMessage
				<< "FIELD[" << iFieldID_
				<< "] : ObjectID must be LogicalFile::ObjectID"
				<< ModEndl;
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}

	// 2. get fieldlength
	// The field length is 0 when it's array
	// The detail info refer to makeElementInfo
	Utility::FieldLength	fieldLength = 0;
	if (getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
		 FileCommon::FileOption::FieldLength::Key, iFieldID_), iDummy)) 
	{
		fieldLength = static_cast<Utility::FieldLength>(iDummy);
	}

	//	3. get the fixed type and reset filedlength
	bool bDummy;
	bFixed_ = false;
	if (Common::Data::isFixedSize(fieldType)) 
	{
		// fixed length
		bFixed_ = true;

		Utility::FieldLength requestedLength =
			FileCommon::DataManager::getFixedCommonDataArchiveSize(	fieldType);

		if (fieldType == Common::DataType::Decimal)
		{
			// Decimal data should have fieldLength specified
			if (fieldLength == 0) 
			{
				_SYDNEY_THROW0(Exception::NotSupported);
			}
		}
		else if (fieldLength == 0)
		{
			// The default value is set when not set. 
			fieldLength = requestedLength;
		}
		else if (fieldLength != requestedLength)
		{
			// The value that the user set is wrong. 
			SydErrorMessage
				<< "FIELD[" << iFieldID_ << "] : fieldType = " << fieldType
				<< ", length = " << fieldLength
				<< ". length must be" << requestedLength
				<< ModEndl;

			_SYDNEY_THROW0(Exception::NotSupported);
		}

	} 
	else if (!bCreated_)	//array is specail variable field
	{
		if(getBoolean(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
			FileCommon::FileOption::FieldFixed::Key,iFieldID_),	bDummy))
		{
			bFixed_ = bDummy;
			if(bFixed_ && (fieldLength == 0 ||fieldType == Common::DataType::Array))
			{
				bFixed_ = false;
			}
		}
	}


	// 4. get the compress information
	// if hint is fixed but field type is variable, it prefer to which?
	bCompress_ = false;
	if (bCreated_)
	{
		// get the hint info
		ModUnicodeString	strDummy;
		if (getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
			FileCommon::FileOption::FieldHint::Key,iFieldID_), strDummy)) 
		{
			FileCommon::HintArray hints(strDummy);
			FileCommon::HintArray::Iterator iterator = hints.begin();
			const FileCommon::HintArray::Iterator& end = hints.end();
			for (; iterator != end; ++iterator) 
			{
				if ((*iterator)->CompareToKey(strDummy, _HintCompressed, _HintCompressed.getLength()))
					bCompress_ = true;
				else if ((*iterator)->CompareToKey(strDummy, _HintFixedSize, _HintFixedSize.getLength()))
					bFixed_ = true;
			}
		}
	}
	else
	{
		if (getBoolean(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
			Record2::FileOption::Compressed::Key, iFieldID_), bDummy))
		{
			bCompress_ = bDummy;
		}
	}
	
	return fieldLength;
}

//	FUNCTION private
//	Record2::FileID::makeFieldInfo -- makeFieldInfo
//
//	NOTES
//		-- makeFieldInfo
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void 
FileID::
makeElementInfo(FieldInfo* pFieldInfo, Utility::ElementNum uiElementNum_)
{
	; _SYDNEY_ASSERT(pFieldInfo);
	//; _SYDNEY_ASSERT(pFieldInfo->getDataType()._name == Common::DataType::Array);

	int iFieldID = pFieldInfo->getFieldID();

	// acquire the data type of the array element
	int	elementTypeDummy;
	if (!getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
		FileCommon::FileOption::ElementType::Key,iFieldID), elementTypeDummy)) 
	{
		// There is no default value in the data type of the array element. 
		// It sends exception if it doesn't set it. 
		SydErrorMessage
			<< "FIELD[" << iFieldID << "] : must be set ElementType"
			<< ModEndl;

		_SYDNEY_THROW0(Exception::Unexpected);
	}

	// Maximum length of array element data
	int	elementLengthDummy = 0;
	getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
		FileCommon::FileOption::ElementLength::Key,iFieldID), elementLengthDummy);

	// acquire the encoding method of the value of the array element
	// It's smae as eFieldEncodingForm so that used it directly
	int iDummy = 0;
	Common::StringData::EncodingForm::Value eElementEncodingForm = 
		Common::StringData::EncodingForm::UCS2;
	if(getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
		FileCommon::FileOption::ElementEncodingForm::Key, iFieldID), iDummy))
	{
		eElementEncodingForm = (static_cast<Common::StringData::EncodingForm::Value>(iDummy)
			== Common::StringData::EncodingForm::UTF8) ?
			Common::StringData::EncodingForm::Unknown :
			static_cast<Common::StringData::EncodingForm::Value>(iDummy);
	}

	// set element info
	// the number of elements is specified by FieldLength. 
	pFieldInfo->setElementInfo(elementTypeDummy, elementLengthDummy, uiElementNum_, eElementEncodingForm);

	//should set field length to 0
	pFieldInfo->setLength(0, true);	
}

//
//	Copyright (c) 2006, 2007, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
