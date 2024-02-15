// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalInterface.h -- Inherited from LogicalFile::File, provide interfaces for record2 module.
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

#ifndef __SYDNEY_RECORD2_LOGICALINTERFACE_H
#define __SYDNEY_RECORD2_LOGICALINTERFACE_H

#include "LogicalFile/File.h"

#include "Record2/Module.h"
#include "Record2/FileID.h"

_SYDNEY_BEGIN

_SYDNEY_RECORD2_BEGIN

class File;
class ObjectManager;
class AreaManager;
class DataAccess;

//If we use File, it'll confuse LogicalFile::File with Record2::File
//so define a alias
typedef File FileAccess;

//
//	CLASS
//	Record2::LogicalInterface -- Inherited from LogicalFile::File, provide interfaces for record module.
//
//	NOTES
//	All calls from outside must through this class
//
class SYD_RECORD2_FUNCTION_TESTEXPORT LogicalInterface : public LogicalFile::File
{
public:
	
	// Constructor, FileID transfers schema information
	LogicalInterface(const LogicalFile::FileID&	cFileID_);

	// Destructor
	~LogicalInterface();

	// Initialization/termination, now do nothing
	static void initialize();
	static void terminate();

//**********************************************************************

	//
	//	Schema Information
	//

	// obtain the file-identification
	const LogicalFile::FileID& getFileID() const;

	// obtain the size of the record file
	ModUInt64 getSize() const;

	// obtain the number of objects that has been inserted
	ModInt64 getCount() const;

	// get the overhead when the object is retrieved
	double getOverhead() const;

	// get the process cost when it accesses the object
	double getProcessCost() const;

	// set the retrieval opening parameter
	bool getSearchParameter(
			const LogicalFile::TreeNodeInterface* pCondition_,
			LogicalFile::OpenOption& cOpenOption_) const;

	// set the projection opening parameter
	bool getProjectionParameter(
			const Common::IntegerArrayData& cProjection_,
			LogicalFile::OpenOption& cOpenOption_) const;

	// set the update opening parameter
	bool getUpdateParameter(
			const Common::IntegerArrayData& cUpdateFields_,
			LogicalFile::OpenOption& cOpenOption_) const;

	// set the sorting order parameter
	bool getSortParameter(
			const Common::IntegerArrayData& cKeys_,
			const Common::IntegerArrayData& cOrders_,
			LogicalFile::OpenOption& cOpenOption_) const;

//**********************************************************************

	//
	//	Data Manipulation
	//

	// set the search condition (object ID)
	void fetch(const Common::DataArrayData* pOption_);

	// obtain the inserted object
	bool get(Common::DataArrayData* pTuple_);

	// insert object into table
	void insert(Common::DataArrayData* pTuple_);

	// update the object value
	void update(const Common::DataArrayData* pKey_, 
				Common::DataArrayData* pTuple_);

	// remove the object by marking this object
	void expunge(const Common::DataArrayData* pKey_);

	// undo expunge, the expunged data will recover
	void undoExpunge(const Common::DataArrayData* pKey_);
	
	// undo update, the updated data will recover
	void undoUpdate(const Common::DataArrayData* pKey_);

	// compact, the data will delete really
	void compact(const Trans::Transaction& cTrans_,
				 bool& bIncomplete_, bool& bModified_);

	// record the position of the rewinding
	void mark();

	// return the marked position
	void rewind();

	// reset the position
	void reset();

//**********************************************************************

	//
	// File operation
	//

	// generate record file but not substance until insert a tuple
	const LogicalFile::FileID& create(const Trans::Transaction& cTrans_);

	// the record file is annulled.
	void destroy(const Trans::Transaction& cTrans_);

	// whether OS file is substance exists
	bool isAccessible(bool force = false) const;
	
	// whether the mount is done
	bool isMounted(const Trans::Transaction& cTrans_) const;

	// open the record file before data operation
	void open(const Trans::Transaction& cTrans_,
			  const LogicalFile::OpenOption&	cOpenOption_);

	// close the record file
	void close();

	// comparison, now only by its path
	bool equals(const Common::Object*	pOther_) const;

	// take synchronization, it will call compact automatically
	void sync(const Trans::Transaction& cTrans_, 
				bool& bIncomplete_, bool& modified);

	// move the file 
	void move(const Trans::Transaction& cTrans_,
			  const Common::StringArrayData&	cArea_);

	// The operation with an unnecessary latch is returned.
	Operation::Value getNoLatchOperation();

	// Capabilities of file driver
	LogicalFile::File::Capability::Value getCapability();

	// get the character string of file identification
	ModUnicodeString toString() const;

//**********************************************************************

	//
	// Method for operation management
	//

	// mount the record file
	const LogicalFile::FileID& mount(const Trans::Transaction&	cTrans_);

	// unmount the record file
	const LogicalFile::FileID& unmount(const Trans::Transaction& cTrans_);

	// flash the record file.
	void flush(const Trans::Transaction&	cTrans_);

	// The backup beginning is notified to the record file. 
	void startBackup(const Trans::Transaction&	cTrans_,
					const bool Restorable_);

	// The backup end is notified to the record file. 
	void endBackup(const Trans::Transaction&	cTrans_);

	// The record file recovers in the trouble.
	void recover(const Trans::Transaction& cTrans_,
					const Trans::TimeStamp&	cPoint_);

	// The version to which the transaction only for the readout begun at the point 
	// its refers is assumed to be the latest version. 
	void restore(const Trans::Transaction& cTrans_,
					const Trans::TimeStamp&	cPoint_);

//**********************************************************************

	//
	// Method for correspondence inspection
	//

	// Method for correspondence inspection
	void verify(const Trans::Transaction& cTrans_,
				const unsigned int	 uiTreatment_,
				Admin::Verification::Progress& cProgress_);

private:

	// The parents class and the class name evade operator = problem when it is the same. 
	LogicalInterface& operator= (const LogicalInterface& cObject_);

	//do expunge and undo expugne really
	void doExpunge(const Common::DataArrayData* pKey_, bool bUndo_ = false);

	// Convert option to object ID during fetch mode
	ObjectID::Value convertFetchOptionToObjectID(const Common::DataArrayData* pOption_) const;

	// An open option is preserved in the data member.
	// the meta information adjust also
	void saveOpenOption(const LogicalFile::OpenOption& cOpenOption_ ,int& iOpenModeValue_);

	// initialize Record2 File
	void initializeFiles(const Trans::Transaction& cTrans_);

	// create ObjectManager and AreaManager
	// load managers' initialize data
	void initializeManagers(bool bForUpdate_, bool bNeedDoArea_ = false, bool bSubstance_ = false);

	// initialize DataAccess
	bool initializeDataAccess(Common::DataArrayData* pTuple_, FileCommon::OpenMode::Mode iOpenMode);

	// check file opened or not
	// file attached or not
	// data access prepared or not
	bool isPrepared(bool bIsVariable_ = false, bool bData_ = false) const;

//**********************************************************************

	// Class which maintains open option
	struct OpenParameter
	{
		// file's open mode(insert and expunge, etc.)
		FileCommon::OpenMode::Mode m_iOpenMode;

		// True: Operation doesn't change the real data
		// False: change into the record file
		bool m_bEstimate;

		// If it's false when get is done in the SCAN mode, 
		// the object is taken out in ascending order. 
		bool m_bSortOrder;	

		// If is select field in geting or updating
		bool m_bFieldSelect;

		//constructor
		OpenParameter(const FileCommon::OpenMode::Mode iOpenMode_,
						const bool  bEstimate_,
						const bool bSortOrder_,
						const bool bFieldSelect_)
						:m_iOpenMode(iOpenMode_),
						m_bEstimate(bEstimate_),
						m_bSortOrder(bSortOrder_),
						m_bFieldSelect(bFieldSelect_)
		{ 
			; // do nothing
		}
	} m_cOpenParam;

//**********************************************************************
	//
	// Data member
	//

	// Object ID specified with fetch
	ObjectID::Value m_iFetchObjectID;

	// object manager, allocate object and iterate it
	ObjectManager* m_pObjectManager;

	// AreaManager, allocate and free DirectArea including data operation
	AreaManager* m_pAreaManager;

	// file access, including fixed and variable file
	FileAccess* m_pFixedFile;
	FileAccess* m_pVariableFile;

	// data access manager during data operation
	DataAccess* m_pDataAccess;

	//FileID, file idetification
	FileID m_cFileID;

}; // end of class LogicalInterface

_SYDNEY_RECORD2_END
_SYDNEY_END

#endif // __SYDNEY_RECORD2_LOGICALINTERFACE_H

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
