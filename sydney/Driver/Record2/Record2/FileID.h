// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.h -- Header file of FileID class which reserves schema information for a table
// 
// Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD2_FILEID_H
#define __SYDNEY_RECORD2_FILEID_H

#include "ModMap.h"

#include "FileCommon/AutoAttach.h"
#include "FileCommon/OpenMode.h"
#include "LogicalFile/FileID.h"
#include "LogicalFile/OpenOption.h"

#include "Record2/FieldInfo.h"

_SYDNEY_BEGIN
_SYDNEY_RECORD2_BEGIN

//
//	TYPEDEF
//	Record2::LogicalFileID --
//  To avoid confusing Record::FileID with LogicalFile::FileID
typedef LogicalFile::FileID LogicalFileID;

//class FieldInfo;

//
//	CLASS
//	Record2::FileID -- class of FileID which reserves schema information for a table
//
//	NOTES
// Class that corresponds to FileID which can efficiently access meta data of record file. 
// The default value returns when the user isn't specifying information
//
class SYD_RECORD2_FUNCTION_TESTEXPORT FileID : public LogicalFileID
{

public:

	// Version information on file
	struct Version
	{
		enum Value
		{
			// --> old version
			Version1 = 0,
			Version2,
			Version3,
			Version4,
			// <-- old version
			Version5,		//	Record2 initial version
			VersionNum,		//	Number of versions
			CurrentVersion = VersionNum - 1	// Present version
		};
	};
	
	// default constructor, needn't implement it
	//FileID();

	// copy constructor
	FileID(const LogicalFileID& cLogicalFileID_);

	// destructor
	virtual ~FileID();

	// create FileID, 
	// only be called when creating a table
	void create();

	// initialize, including FieldInfo, ObjectSize, PageSize...
	void initialize();

//**********************************************************************

	// is the mount done?
	bool isMounted() const;

	// is temporary operation?
	bool isTemporary() const;

	// is read only or write
	bool isReadOnly() const;

	// has variable file/fields
	bool hasVariable() const;

	// has compress fields?
	//bool hasCompress() const;

	// get lockname
	const Lock::FileName& getLockName() const;

	// get the directory path at the record file storage destination 
	const Os::Path& getDirectoryPath() const;

	// set the directory path at the record file storage destination
	void setDirectoryPath(const ModUnicodeString&	cstrPath_);

	// obtain the number of objects reserved in a fixed page
	Utility::ObjectNum getObjectNumberPerPage() const;

	// obtain the size of the object in the fixed length part
	Utility::Size getObjectSize() const;

	// Physical page size is acquired including fixed and variable file
	Utility::Size getFixedPageSize() const;
	Utility::Size getVariablePageSize() const;

	// check the version number
	static bool checkVersion(const LogicalFile::FileID& cLogicalFileID_);

	// Inspection of equivalence(Only the m_cPath is compared. 
	// Therefore, logical file-identifications are not compared. )
	bool equals(const FileID& cFileID_) const;

//**********************************************************************
	// define TargetFields
	typedef ModVector<FieldInfo*> TargetFields;

	// get TargetFields including fixed and variable fields
	const TargetFields& getTargetFields(bool bIsVariable_ = false) const;

	// reset TargetFields, remove the unwanted field info and add some extra infomation
	bool resetTargetFields(const LogicalFile::OpenOption& cOpenOption_, FileCommon::OpenMode::Mode& eOpenMode_);

	// when updating and has not select fields
	void adjustDataIndex(bool bDecreased_);

	// get fieldid by data index
	int getFieldIDbyDataIndex(int iDataIndex_);

private:

	// make FieldInfo such as type, length, etc.
	Utility::FieldLength makeFieldInfo(int iFieldID_, 
										bool bCreated_, 
										bool& bFixed_, 
										bool& bCompress_, 
										Common::DataType::Type& fieldType);

	//make element info for array
	void makeElementInfo(FieldInfo* pFieldInfo, Utility::ElementNum uiElementNum_);

	// make page size
	void makeFixedPageSize() ;
	void calculateFixedPageSize();
	void makeVariablePageSize();

	// set the size of the object
	void makeFixedObjectSize();
	
	//find index by FieldID and set selected
	//return true is fixed data, else is variable
	bool setSelectedByFieldID(const int iFieldID_, const int iDataIndex_);

	//adjust index and fieldcount
	void adjustTargetFields(bool bSelected_ = true);

//**********************************************************************

	// Top path name of directory that stores file
	Os::Path m_cPath;

	// Lock name of File
	mutable Lock::FileName m_cLockName;

	// Number of objects a page of fixed length parts
	Utility::ObjectNum m_uiObjectNumberPerPage;

	// Size of object in fixed length part
	Utility::Size m_uiFixedObjectSize;

	// Page size specified (Or, default). 
	Utility::Size m_uiFixedPageSize;
	Utility::Size m_uiVariablePageSize;

	// Version of file
	int m_iVersion;

	// Is it temporarily a table?
	bool m_bTemporary;

	// Is it only for reading?
	bool m_bReadOnly;
	
	// Has compress or not
	bool m_bCompress;

	// Has variable file/fields or not
	bool m_bHasVariable;
	bool m_bIsVariableUsed;

	//field fixed and variable info
    TargetFields m_cFixedTargetFields;
    TargetFields m_cVariableTargetFields;

	//have initialized, prevent initialized multi-times
	bool m_bInitialized;
};

// Set absolute path name
inline
void
FileID::setDirectoryPath(const ModUnicodeString& cstrPath_)
{
	m_cPath = cstrPath_;
}

// Obtain the number of objects a page of fixed length parts
inline
Utility::ObjectNum
FileID::getObjectNumberPerPage() const
{
	return m_uiObjectNumberPerPage;
}

// Size of object in fixed length part
inline
Utility::Size
FileID::getObjectSize() const
{
	return m_uiFixedObjectSize;
}

// acquire physical page size
inline
Utility::Size
FileID::getFixedPageSize() const
{
	return m_uiFixedPageSize;
}

// acquire variable physical page size
inline
Utility::Size
FileID::getVariablePageSize() const
{
	return m_uiVariablePageSize;
}

// get the directory path at the record file storage destination
inline
const Os::Path&
FileID::getDirectoryPath() const
{
	return m_cPath;
}

// is temporary operation?
inline
bool
FileID::isTemporary() const
{
	return m_bTemporary;
}

// is only for reading?
inline
bool
FileID::isReadOnly() const
{
	return m_bReadOnly;
}

// has variable file/fields?
inline
bool
FileID::hasVariable() const
{
	return m_bHasVariable && m_bIsVariableUsed;
}


// Has compress or not
//inline
//bool
//FileID::hasCompress() const
//{
//	return m_bCompress;
//}

_SYDNEY_RECORD2_END
_SYDNEY_END

#endif // __SYDNEY_RECORD2_FILEID_H

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
