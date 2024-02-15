// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoLogicalFile.h --
//		自動論理ファイル関連のクラス定義、関数宣言
// 
// Copyright (c) 2003, 2004, 2005, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_LOGICALFILE_AUTOLOGICALFILE_H
#define	__SYDNEY_LOGICALFILE_AUTOLOGICALFILE_H

#include "LogicalFile/Module.h"
#include "LogicalFile/File.h"
#include "LogicalFile/FileDriver.h"
#include "LogicalFile/FileID.h"
#include "LogicalFile/OpenOption.h"

#include "Common/AutoCaller.h"
#include "Common/Object.h"

#include "Admin/Verification.h"

#include "Schema/ObjectID.h"

namespace
{
	class _LatchName; // defined in cpp file
}

_SYDNEY_BEGIN

namespace Lock
{
	class Name;
}

_SYDNEY_LOGICALFILE_BEGIN

//	CLASS
//	LogicalFile::AutoLogicalFile --
//		オブジェクト生成時に自動的に論理ファイルをアタッチし、
//		破棄時に自動的にデタッチするクラス
//
//	NOTES

class SYD_LOGICALFILE_FUNCTION AutoLogicalFile
	: public Common::Object
{
public:
	// CLASS
	//	LogicalFile::AutoLogicalFile::AutoUnlatch -- auto unlatcher
	//
	// NOTES
	class AutoUnlatch
	{
	public:
		typedef Common::AutoCaller0<AutoLogicalFile> AutoCaller;

		// constructor
		AutoUnlatch(AutoLogicalFile* p_)
			: m_pFile(p_),
			  m_cAutoCaller(p_, &AutoLogicalFile::unlatch)
		{}
		AutoUnlatch(const AutoUnlatch& cOther_)
			: m_pFile(cOther_.m_pFile),
			  m_cAutoCaller(cOther_.m_cAutoCaller)
		{}

		// explicit latch
		void latch();
		// explicit unlatch
		void unlatch();

		AutoUnlatch& operator=(AutoUnlatch& cOther_)
		{
			m_pFile = cOther_.m_pFile;
			m_cAutoCaller = cOther_.m_cAutoCaller;
			return *this;
		}

	protected:
	private:
		AutoLogicalFile* m_pFile;
		AutoCaller m_cAutoCaller;
	};

	// コンストラクター
	AutoLogicalFile();
	AutoLogicalFile(FileDriver& cDriver_, const FileID& cFileID_);
	AutoLogicalFile(const AutoLogicalFile& cOther_);

	// デストラクター
	~AutoLogicalFile();

	// Fileのラッパー
	bool				getSearchParameter(const TreeNodeInterface*	pCondition_,
										   OpenOption&				cOpenOption_) const;
	bool				getProjectionParameter(const TreeNodeInterface*	pNode_,
											   OpenOption&				cOpenOption_) const;
	bool				getProjectionParameter(const Common::IntegerArrayData&	cProjection_,
											   OpenOption&						cOpenOption_) const;
	bool				getUpdateParameter(const Common::IntegerArrayData&	cUpdateFields_,
										   OpenOption&						cOpenOption_) const;
	bool				getSortParameter(const TreeNodeInterface* 	pNode_,
										 OpenOption&				cOpenOption_) const;
	bool				getSortParameter(const Common::IntegerArrayData&	cKeys_,
										 const Common::IntegerArrayData&	cOrders_,
										 OpenOption&						cOpenOption_) const;
	bool				getLimitParameter(const Common::IntegerArrayData&	cSpec_,
										  OpenOption&						cOpenOption_) const;

	void				open(Trans::Transaction&	cTrans_,
							 const OpenOption&		cOption_);
	void				close();

	const FileID&		create(Trans::Transaction& cTrans_);
	void				destroy(Trans::Transaction& cTrans_, bool bForce_ = true);
	void				destroy(Trans::Transaction& cTrans_, Schema::ObjectID::Value iDatabaseID_,
								bool bForce_ = true);
	void				undoDestroy(Trans::Transaction& cTrans_);

	const FileID&		mount(Trans::Transaction& cTransaction_);
	const FileID&		unmount(Trans::Transaction& cTransaction_);
	void				flush(Trans::Transaction& cTransaction_);
	void				startBackup(Trans::Transaction& cTransaction_, bool bRestorable_ = true);
	void				endBackup(Trans::Transaction& cTransaction_);
	void				recover(Trans::Transaction& cTransaction_, const Trans::TimeStamp& cPoint_);
	void				restore(Trans::Transaction& cTransaction_, const Trans::TimeStamp& cPoint_);
	void				sync(Trans::Transaction& trans, bool& incomplete, bool& modified);

	bool				getData(Common::DataArrayData* pData_);
	void				getProperty(Common::DataArrayData* pKey_,
									Common::DataArrayData* pValue_);
	Locator*			getLocator(const Common::DataArrayData* pKey_);

	void				compact(Trans::Transaction& trans, bool& incomplete, bool& modified);

	void				insert(Common::DataArrayData* pTuple_);
	void				update(const Common::DataArrayData* pKey_, Common::DataArrayData* pTuple_);
	void				expunge(const Common::DataArrayData* pKey_);
	void				undoExpunge(const Common::DataArrayData* pKey_);
	void				undoUpdate(const Common::DataArrayData* pKey_);
	void				fetch(const Common::DataArrayData* pOption_);

	void				move(Trans::Transaction&			cTransaction_,
							 const Common::StringArrayData&	cArea_);

	void				verify(Trans::Transaction& cTrans_,
							   Admin::Verification::Treatment::Value uiTreatment_,
							   Admin::Verification::Progress& cProgress_);
	ModInt64			getSize(Trans::Transaction& cTrans_);
	ModInt64			getCount(Trans::Transaction& cTrans_);
	void				getCost(Trans::Transaction& cTrans_, OpenOption& cOpenOption_,
								double& dblOverhead_, double& dblProcessCost_,
								double* pdblCount_ = 0);

	void				mark();
	void				rewind();
	void				reset();

	bool				isNeedLatch(File::Operation::Value iOperation_);
	bool				isKeepLatch();

	AutoUnlatch			latch();
	void				unlatch();

	ModUnicodeString	toString() const;

	// その他のメソッド
	bool				isOpened() const;		// オープンされているか
	File*				attach(FileDriver& cDriver_, const FileID& cFileID_);
	File*				attach(FileDriver& cDriver_, const FileID& cFileID_,
							   const Lock::Name& cLockName_,
							   Trans::Transaction& cTrans_,
							   bool bNoLatch_);
												// ファイルをアタッチする
	void				detach();				// ファイルをデタッチする
	bool				isAttached() const;		// ファイルがアタッチされているか
	bool				isAccessible(bool bForce_ = false);
												// ファイルがあるか
	bool				isAbleTo(File::Capability::Value iCapability_);
												// File driver supports some features
private:
	FileDriver*		m_pDriver;
	File*			m_pFile;
	bool			m_bOpened;
	bool			m_bIsUpdate;
	File::Operation::Value m_iNoLatchOperation;

	_LatchName*		m_pLatchName;
};

_SYDNEY_LOGICALFILE_END
_SYDNEY_END

#endif	// __SYDNEY_LOGICALFILE_AUTOLOGICALFILE_H

//
// Copyright (c) 2003, 2004, 2005, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
