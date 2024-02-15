// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.h -- File.h manage physical file hander and execute normal file operation
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

#ifndef __SYDNEY_RECORD2_FILE_H
#define __SYDNEY_RECORD2_FILE_H

#include "Record2/FileID.h"

_SYDNEY_BEGIN

_SYDNEY_RECORD2_BEGIN

//
//	CLASS
//	AutoFile -- a template for attach/detach automatically
//
//	NOTES
//	Attach/detach files it before and behind scope. 
//
template< class T >
class AutoFile : FileCommon::AutoObject
{
public:
	AutoFile(T& cFile_)
		: m_pFile(&cFile_)
	{
		m_pFile->attachFile();
	}
	AutoFile(T* pFile_)
		: m_pFile(pFile_)
	{
		if (m_pFile) m_pFile->attachFile();
	}
	~AutoFile()
	{
		detach();
	}
	void detach()
	{
		if (m_pFile) m_pFile->detachFile();
		m_pFile = 0;
	}
private:
	T* m_pFile;
};

class File;
// Supplementary class to do attach/detach correctly
typedef AutoFile<File> AutoAttachFile;

//
//	CLASS
//	Record2::File -- manage physical file hander and normal file operation
//		File
//	NOTES
//	Implement file and admin operations such as mount, sync.
//	Normally, all methods include two parts, fixed and variable physical files.
//
class File : public Common::Object
{
	//here need modify
	//friend class Manager;
	//friend class ObjectManager;
	//friend class AreaManager;

public:
    //constructor
    File(const Trans::Transaction& cTrans_, 
			const FileID& cFileID_, 
			const bool bIsVariable_);

	//destructor
	~File();

	// get trasaction
	const Trans::Transaction& getTransaction() const;

	// attach physical file. 
    void attachFile(const bool bBatch_ = false);

    //detach physical file
    void detachFile();

	//file is attached
	bool isAttached() const;

    //whether the mount is done
    bool isMounted(const Trans::Transaction& cTrans_) const;

    //whether OS file is accessible
    bool isAccessible(bool bForce_ = false) const;

	//is variable file
	bool isVariable() const;

	//is batch mode
	bool isBatch() const;

	//get physical file handle
	PhysicalFile::File* getFile() const;

	//get the meta file
	const FileID& getFileID()const;

	//obtain the path name of the directory
	virtual const ModUnicodeString& getPathPart() const;

	//obtain the path name where the file is stored
	const ModUnicodeString& getPath() const;

    //get the file size
	ModUInt64 getSize() const;

    //return the overhead when retrieving
    double getOverhead() const;

    //return one object access time
    double getProcessCost(ModInt64 iCount_ = 0) const;

//**********************************************************************************

	//create table
	void create();

	//The file is annulled. 
	void destroy();

    //mount and unmount
    void mount();
    void unmount();

    //flush
    void flush();

    //Backup beginning and end
    void startBackup(const bool bRestorable_);
    void endBackup();

    //Recovery
    void recover(const Trans::TimeStamp& cPoint_);

    //The version of the transaction begun at the point that is is made the latest. 
    void restore(const Trans::TimeStamp& cPoint_);

    //The file is moved. 
    void move(bool bUndo_ = false);

    //sync, use it's own transaction
    void sync(const Trans::Transaction& cTrans_, bool& bInComplete_, bool& bModified_);

	// the validaty is verified
	void startVerification(const unsigned int uiTreatment_, Admin::Verification::Progress& cProgress_);
	void endVerification(Admin::Verification::Progress& cProgress_);

//**********************************************************************************

	//create a physical file 
	void substantiate();

private:	

	// move the file
	void move(ModUnicodeString& cstrOldPath_,
			  ModUnicodeString& cstrNewPath_,
			  bool bUndo_ = false);

	//set the strategy for a physical file from the meta data. 
    void setStorategy(const PhysicalFile::Type& ePhysicalFileType_, const ModUnicodeString& cstrFileName_);

//**********************************************************************************

	//trasaction
    const Trans::Transaction& m_cTrans;

	//fileid
	const FileID& m_cFileID;

	//physical file handle
	PhysicalFile::File* m_pFile;

	//physical file stratery
	PhysicalFile::File::StorageStrategy		m_cStorageStrategy;
	PhysicalFile::File::BufferingStrategy	m_cBufferingStrategy;

	//fixed or variable
	bool m_bIsVariable;

	//isBatch
	bool m_bBatch;
};

// get trasaction
inline 
const Trans::Transaction& 
File::
getTransaction() const
{ 
	return m_cTrans; 
}

//get the underlying PhysicalFile
inline 
PhysicalFile::File*
File::
getFile() const
{
	return m_pFile;
}

//get the underlying PhysicalFile
inline 
const FileID&
File::
getFileID() const
{
	return m_cFileID;
}

//isVariable
inline 
bool
File::
isVariable() const
{
	return m_bIsVariable;
}

//is Batch mode
inline
bool
File::
isBatch() const
{
	return m_bBatch;
}


_SYDNEY_RECORD2_END
_SYDNEY_END

#endif // __SYDNEY_RECORD2_FILE_H

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
