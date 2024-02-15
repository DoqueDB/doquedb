// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoLogicalFile.cpp --
//		自動論理ファイル関連の関数定義
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "LogicalFile";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "LogicalFile/AutoLogicalFile.h"
#include "LogicalFile/OpenOption.h"
#include "LogicalFile/Parameter.h"

#include "Checkpoint/FileDestroyer.h"

#include "Common/Assert.h"
#include "Common/AutoCaller.h"

#include "Exception/BadArgument.h"
#include "Exception/FakeError.h"
#include "Exception/FileNotFound.h"
#include "Exception/Unexpected.h"

#include "Trans/Transaction.h"

#include "ModException.h"

_SYDNEY_USING
_SYDNEY_LOGICALFILE_USING

namespace
{
	// CLASS local
	//	$$$::_LatchName -- used to automatic latch
	//
	// NOTES
	class _LatchName
		: public Common::Object
	{
	public:
		_LatchName(const Lock::Name& cLockName_,
				   Trans::Transaction& cTrans_)
			: m_cLockName(cLockName_),
			  m_cTrans(cTrans_),
			  m_iLatched(0),
			  m_eKeepUntil(File::Operation::None)
		{}

		void latch()
		{
			if (m_iLatched++ == 0) {
				m_cTrans.latch(m_cLockName);
			}
		}
		void unlatch()
		{
			if (--m_iLatched == 0) {
				m_cTrans.unlatch(m_cLockName);
			}
		}
		bool isLatched() {return m_iLatched > 0;}

		void keepUntil(File::Operation::Value eOperation_)
		{
			m_eKeepUntil = eOperation_;
		}
		bool isKeptUntil(File::Operation::Value eOperation_)
		{
			return m_eKeepUntil != File::Operation::None
				&& eOperation_ == m_eKeepUntil;
		}
	protected:
	private:
		Lock::Name m_cLockName;
		Trans::Transaction& m_cTrans;
		int m_iLatched;
		File::Operation::Value m_eKeepUntil;
	};

	// CLASS local
	//	$$$::_AutoLatch -- automatic latch/unlatch class
	//
	// NOTES
	class _AutoLatch
	{
	public:
		_AutoLatch(_LatchName* pLatchName_,
				   AutoLogicalFile* pFile_ = 0,
				   File::Operation::Value eOperation_ = File::Operation::None,
				   File::Operation::Value eKeepUntil_ = File::Operation::None)
			: m_pLatchName(pLatchName_),
			  m_eOperation(eOperation_)
		{
			if (pLatchName_ 
				&& (pFile_ == 0 || pFile_->isNeedLatch(eOperation_))) {
				latch(eKeepUntil_);
			} else {
				m_pLatchName = 0; // no unlatch
			}
		}
		~_AutoLatch()
		{
			unlatch(m_eOperation);
		}
		void latch(File::Operation::Value eKeepUntil_)
		{
			if (m_pLatchName) {
				m_pLatchName->latch();
				if (eKeepUntil_ != File::Operation::None) {
					// latch again
					m_pLatchName->latch();
					// set keep until operation
					m_pLatchName->keepUntil(eKeepUntil_);
				}
			}
		}
		void unlatch(File::Operation::Value eOperation_)
		{
			if (m_pLatchName) {
				m_pLatchName->unlatch();
				if (m_pLatchName->isKeptUntil(eOperation_)) {
					// unlatch again
					m_pLatchName->unlatch();
				}
			}
		}
	protected:
	private:
		_LatchName* m_pLatchName;
		File::Operation::Value m_eOperation;
	};
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::AutoLogicalFile -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

AutoLogicalFile::
AutoLogicalFile()
	: m_pDriver(0), m_pFile(0), m_bOpened(false),
	  m_iNoLatchOperation(0), m_bIsUpdate(false),
	  m_pLatchName(0)
{ }

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::AutoLogicalFile -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		FileDriver& cDriver_
//			ファイルドライバー
//		const FileID& cFileID_
//			アタッチするファイルのファイルID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

AutoLogicalFile::
AutoLogicalFile(FileDriver& cDriver_, const FileID& cFileID_)
	: m_pDriver(&cDriver_), m_pFile(0), m_bOpened(false),
	  m_iNoLatchOperation(0), m_pLatchName(0)
{
	m_pFile = cDriver_.attachFile(cFileID_);
	m_iNoLatchOperation = m_pFile->getNoLatchOperation();
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::AutoLogicalFile -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const AutoLogicalFile& cOther_
//			コピー元
//
//	RETURN
//		なし
//
//	EXCEPTIONS

AutoLogicalFile::
AutoLogicalFile(const AutoLogicalFile& cOther_)
	: m_pDriver(cOther_.m_pDriver), m_pFile(cOther_.m_pFile),
	  m_bOpened(cOther_.m_bOpened),
	  m_iNoLatchOperation(cOther_.m_iNoLatchOperation),
	  m_pLatchName(cOther_.m_pLatchName)
{
	// コピーしたらコピー元は無効にする
	const_cast<AutoLogicalFile&>(cOther_).m_pDriver = 0;
	const_cast<AutoLogicalFile&>(cOther_).m_pFile = 0;
	const_cast<AutoLogicalFile&>(cOther_).m_bOpened = false;
	const_cast<AutoLogicalFile&>(cOther_).m_pLatchName = 0;
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::~AutoLogicalFile -- デストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

AutoLogicalFile::
~AutoLogicalFile()
{
	detach();
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::getSearchParameter --
//		検索のオープンパラメーターを得る
//
//	NOTES
//
//	ARGUMENTS
//		const TreeNodeInterface* pCond_
//			条件を表す木構造
//		OpenOption& cOption_
//			オープンオプション
//
//	RETURN
//		指定された条件が処理可能ならtrue、不可能ならfalse
//
//	EXCEPTIONS

bool
AutoLogicalFile::
getSearchParameter(const TreeNodeInterface*	pCondition_,
				   OpenOption&				cOpenOption_) const
{
	return m_pFile->getSearchParameter(pCondition_, cOpenOption_);
}

// FUNCTION public
//	LogicalFile::AutoLogicalFile::getProjectionParameter -- 
//
// NOTES
//
// ARGUMENTS
//	const TreeNodeInterface*	pNode_
//	OpenOption&				cOpenOption_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
AutoLogicalFile::
getProjectionParameter(const TreeNodeInterface*	pNode_,
					   OpenOption&				cOpenOption_) const
{
	return m_pFile->getProjectionParameter(pNode_, cOpenOption_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::getProjectionParameter --
//		プロジェクションのオープンパラメーターを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Common::IntegerArrayData& cProjection_
//			取得するフィールド位置を並べたもの
//		OpenOption& cOption_
//			オープンオプション
//
//	RETURN
//		指定されたプロジェクションが処理可能ならtrue、不可能ならfalse
//
//	EXCEPTIONS

bool
AutoLogicalFile::
getProjectionParameter(const Common::IntegerArrayData&	cProjection_,
					   OpenOption&						cOpenOption_) const
{
	return m_pFile->getProjectionParameter(cProjection_, cOpenOption_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::getUpdateParameter --
//		更新のオープンパラメーターを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Common::IntegerArrayData& cProjection_
//			更新するフィールド位置を並べたもの
//		OpenOption& cOption_
//			オープンオプション
//
//	RETURN
//		指定された更新が処理可能ならtrue、不可能ならfalse
//
//	EXCEPTIONS

bool
AutoLogicalFile::
getUpdateParameter(const Common::IntegerArrayData&	cUpdateFields_,
				   OpenOption&						cOpenOption_) const
{
	return m_pFile->getUpdateParameter(cUpdateFields_, cOpenOption_);
}

// FUNCTION public
//	LogicalFile::AutoLogicalFile::getSortParameter -- 
//
// NOTES
//
// ARGUMENTS
//	const TreeNodeInterface* 	pNode_
//	OpenOption&				cOpenOption_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
AutoLogicalFile::
getSortParameter(const TreeNodeInterface* 	pNode_,
				 OpenOption&				cOpenOption_) const
{
	return m_pFile->getSortParameter(pNode_, cOpenOption_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::getSortParameter --
//		ソートのオープンパラメーターを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Common::IntegerArrayData& cKeys_
//			ソートするフィールド位置を並べたもの
//		const Common::IntegerArrayData& cOrders_
//			昇順/降順を表すenum型をintにキャストしたものを並べたもの
//		OpenOption& cOption_
//			オープンオプション
//
//	RETURN
//		指定されたソート順での取得が可能ならtrue、不可能ならfalse
//
//	EXCEPTIONS

bool
AutoLogicalFile::
getSortParameter(const Common::IntegerArrayData&	cKeys_,
				 const Common::IntegerArrayData&	cOrders_,
				 OpenOption&						cOpenOption_) const
{
	return m_pFile->getSortParameter(cKeys_, cOrders_, cOpenOption_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::getLimitParameter --
//		Limitのオープンパラメーターを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Common::IntegerArrayData& cSpec_
//			LimitとOffset(あれば)を並べたもの
//		OpenOption& cOption_
//			オープンオプション
//
//	RETURN
//		指定されたLimitでの取得が可能ならtrue、不可能ならfalse
//
//	EXCEPTIONS

bool
AutoLogicalFile::
getLimitParameter(const Common::IntegerArrayData&	cSpec_,
				  OpenOption&						cOpenOption_) const
{
	return m_pFile->getLimitParameter(cSpec_, cOpenOption_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::open -- ファイルをオープンする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTransaction_
//			操作を行うトランザクション記述子
//		const OpenOption& cOption_
//			オープンオプション
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
open(Trans::Transaction&	cTrans_,
	 const OpenOption&		cOption_)
{
	; _SYDNEY_ASSERT(!m_bOpened);

	m_bIsUpdate = m_pLatchName != 0
		&& cOption_.getInteger(OpenOption::KeyNumber::OpenMode) == OpenOption::OpenMode::Update;

	_AutoLatch latch(m_pLatchName,
					 this,
					 File::Operation::Open,
					 isKeepLatch() ? File::Operation::Close : File::Operation::None);
	; _SYDNEY_ASSERT(isNeedLatch(File::Operation::Open) == false
					 || m_pLatchName == 0
					 || m_pLatchName->isLatched());
	m_pFile->open(cTrans_, cOption_);
	m_bOpened = true;
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::close -- ファイルをクローズする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
close()
{
	if (m_bOpened) {
		_AutoLatch latch(m_pLatchName,
						 this,
						 File::Operation::Close);
		; _SYDNEY_ASSERT(isNeedLatch(File::Operation::Close) == false
						 || m_pLatchName == 0
						 || m_pLatchName->isLatched());
		m_pFile->close();
		m_bIsUpdate = false;
		m_bOpened = false;
	}
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::create -- ファイルを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		ファイルID
//
//	EXCEPTIONS

const FileID&
AutoLogicalFile::
create(Trans::Transaction& cTrans_)
{
	_AutoLatch latch(m_pLatchName);
	; _SYDNEY_ASSERT(m_pLatchName == 0
					 || m_pLatchName->isLatched());
	return m_pFile->create(cTrans_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::destroy -- ファイルを消去する
//
//	NOTES
//		消去とdetachが同時に行われるのでこのメソッドが必要になる
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bForce_ = true
//			trueの場合チェックポイントを待たずに即座に破棄する
//			falseの場合システムパラメーターによる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
destroy(Trans::Transaction& cTrans_, bool bForce_)
{
	if (m_pDriver) {
		if (m_pFile) {
			close();
			// DatabaseIDが渡されない場合は常に即座に破棄する
			try {
				_AutoLatch latch(m_pLatchName);
				; _SYDNEY_ASSERT(m_pLatchName == 0
								 || m_pLatchName->isLatched());
				m_pFile->destroy(cTrans_);

			} catch (ModException& e) {
				switch (e.getErrorNumber()) {
				case ModOsErrorFileNotFound:
					// ファイルが見つからないエラーは無視する
					ModErrorHandle::reset();
					break;
				default:
					_SYDNEY_RETHROW;
				}
			}
			// ドライバーからデタッチする
			m_pDriver->detachFile(m_pFile);
			m_pFile = 0;
		}
		m_pDriver = 0;
	}
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::destroy -- ファイルを消去する
//
//	NOTES
//		消去とdetachが同時に行われるのでこのメソッドが必要になる
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::ObjectID::Value iDatabaseID_
//			破棄するファイルが属するデータベースID
//		bool bForce_ = true
//			trueの場合チェックポイントを待たずに即座に破棄する
//			falseの場合システムパラメーターによる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
destroy(Trans::Transaction& cTrans_, Schema::ObjectID::Value iDatabaseID_, bool bForce_)
{
	if (m_pDriver) {
		if (m_pFile) {
			close();
			if (!bForce_) {
				Checkpoint::FileDestroyer::enter(cTrans_, iDatabaseID_, *m_pDriver, *m_pFile);
			} else {
				try {
					_AutoLatch latch(m_pLatchName);
					; _SYDNEY_ASSERT(m_pLatchName == 0
									 || m_pLatchName->isLatched());
					m_pFile->destroy(cTrans_);

				} catch (ModException& e) {
					switch (e.getErrorNumber()) {
					case ModOsErrorFileNotFound:
						// ファイルが見つからないエラーは無視する
						ModErrorHandle::reset();
						break;
					default:
						_SYDNEY_RETHROW;
					}
				}
			}
			// ドライバーからデタッチする
			m_pDriver->detachFile(m_pFile);
			m_pFile = 0;
		}
		m_pDriver = 0;
	}
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::undoDestroy -- ファイルの消去を取り消す
//
//	NOTES
//		force = falseで実行されたdestroyを取り消す。破棄の予約からCheckpointが起きていないことが前提。
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
undoDestroy(Trans::Transaction& cTrans_)
{
	if (m_pDriver) {
		if (m_pFile) {
			close();
			Checkpoint::FileDestroyer::erase(cTrans_, *m_pDriver, *m_pFile);
			// ドライバーからデタッチする
			m_pDriver->detachFile(m_pFile);
			m_pFile = 0;
		}
		m_pDriver = 0;
	}
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::mount -- 論理ファイルをマウントする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTransaction_
//			トランザクション記述子
//
//	RETURN
//		const LogicalFile::FileID&
//			File 識別子
//
//	EXCEPTIONS
//
//
const LogicalFile::FileID&
AutoLogicalFile::
mount(Trans::Transaction& cTrans_)
{
	_AutoLatch latch(m_pLatchName);
	; _SYDNEY_ASSERT(m_pLatchName == 0
					 || m_pLatchName->isLatched());
	return m_pFile->mount(cTrans_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::unmount -- 論理ファイルをアンマウントする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTransaction_
//			トランザクション記述子
//
//	RETURN
//		const FileID&
//			ファイル識別子
//
//	EXCEPTIONS
//
//

const FileID&
AutoLogicalFile::
unmount(Trans::Transaction& cTrans_)
{
	_AutoLatch latch(m_pLatchName);
	; _SYDNEY_ASSERT(m_pLatchName == 0
					 || m_pLatchName->isLatched());
	return m_pFile->unmount(cTrans_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::flush -- 論理ファイルをフラッシュする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//

void
AutoLogicalFile::
flush(Trans::Transaction& cTrans_)
{
	_AutoLatch latch(m_pLatchName);
	; _SYDNEY_ASSERT(m_pLatchName == 0
					 || m_pLatchName->isLatched());
	m_pFile->flush(cTrans_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::startBackup -- 論理ファイルのバックアップを開始する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
startBackup(Trans::Transaction& cTrans_,
			bool bRestorable_)
{
	_AutoLatch latch(m_pLatchName);
	; _SYDNEY_ASSERT(m_pLatchName == 0
					 || m_pLatchName->isLatched());
	m_pFile->startBackup(cTrans_, bRestorable_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::endBackup -- 論理ファイルのバックアップを終了する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		取得したデータ
//
//	EXCEPTIONS

void
AutoLogicalFile::
endBackup(Trans::Transaction& cTrans_)
{
	_AutoLatch latch(m_pLatchName);
	; _SYDNEY_ASSERT(m_pLatchName == 0
					 || m_pLatchName->isLatched());
	m_pFile->endBackup(cTrans_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::recover -- 論理ファイルの障害から回復する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTransaction_
//			操作を行うトランザクション記述子
//		const Trans::TimeStamp& cPoint_
//			リカバーで回復する時点を表すタイムスタンプ
//
//	RETURN
//		取得したデータ
//
//	EXCEPTIONS

void
AutoLogicalFile::
recover(Trans::Transaction& cTrans_,
		const Trans::TimeStamp& cPoint_)
{
	_AutoLatch latch(m_pLatchName);
	; _SYDNEY_ASSERT(m_pLatchName == 0
					 || m_pLatchName->isLatched());
	m_pFile->recover(cTrans_, cPoint_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::restore --
//		ある時点に開始された読取専用トランザクションが
//		参照する版を最新版とする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTransaction_
//			操作を行うトランザクション記述子
//		const Trans::TimeStamp& cPoint_
//			リカバーで回復する時点を表すタイムスタンプ
//
//	RETURN
//		取得したデータ
//
//	EXCEPTIONS

void
AutoLogicalFile::
restore(Trans::Transaction& cTrans_,
		const Trans::TimeStamp& cPoint_)
{
	_AutoLatch latch(m_pLatchName);
	; _SYDNEY_ASSERT(m_pLatchName == 0
					 || m_pLatchName->isLatched());
	m_pFile->restore(cTrans_, cPoint_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::sync -- 不要な版を破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			不要な版を破棄する処理を行う
//			トランザクションのトランザクション記述子
//		bool&				incomplete
//			true
//				今回の同期処理でファイルを持つ
//				オブジェクトの一部に処理し残しがある
//			false
//				今回の同期処理でファイルを持つ
//				オブジェクトを完全に処理してきている
//
//				同期処理の結果、ファイルを処理し残したかを設定する
//		bool&				modified
//			true
//				今回の同期処理でファイルを持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の同期処理でファイルを持つ
//				オブジェクトはまだ更新されていない
//
//				同期処理の結果、ファイルが更新されたかを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::sync(Trans::Transaction& trans, bool& incomplete, bool& modified)
{
	_AutoLatch latch(m_pLatchName);
	; _SYDNEY_ASSERT(m_pLatchName == 0
					 || m_pLatchName->isLatched());
	m_pFile->sync(trans, incomplete, modified);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::getData -- データを取得する
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataArrayData* pData_
//			取得したデータを入れるDataArrayData
//
//	RETURN
//		データが取得できればtrue、尽きていたらfalse
//
//	EXCEPTIONS

bool
AutoLogicalFile::
getData(Common::DataArrayData* pData_)
{
#ifdef DEBUG
	int n = pData_->getCount();
	; _SYDNEY_ASSERT(n);
	for (int i = 0; i < n; ++i) {
		; _SYDNEY_ASSERT(pData_->getElement(i).get());
	}
#endif
	_AutoLatch latch(m_pLatchName,
					 this,
					 File::Operation::GetData);
	; _SYDNEY_ASSERT(isNeedLatch(File::Operation::GetData) == false
					 || m_pLatchName == 0
					 || m_pLatchName->isLatched());
	return m_pFile->get(pData_);
}

// FUNCTION public
//	LogicalFile::AutoLogicalFile::getProperty -- 
//
// NOTES
//
// ARGUMENTS
//	Common::DataArrayData* pKey_
//	Common::DataArrayData* pValue_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
AutoLogicalFile::
getProperty(Common::DataArrayData* pKey_,
			Common::DataArrayData* pValue_)
{
	_AutoLatch latch(m_pLatchName);
	; _SYDNEY_ASSERT(m_pLatchName == 0
					 || m_pLatchName->isLatched());
	m_pFile->getProperty(pKey_, pValue_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::getLocator -- Locatorを取得する
//
//	NOTES
//
//	ARGUMENTS
//		const Common::DataArrayData* pKey_
//
//	RETURN
//		取得したLocator
//
//	EXCEPTIONS

Locator*
AutoLogicalFile::
getLocator(const Common::DataArrayData* pKey_)
{
	_AutoLatch latch(m_pLatchName);
	; _SYDNEY_ASSERT(m_pLatchName == 0
					 || m_pLatchName->isLatched());
	return m_pFile->getLocator(pKey_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::compact -- 不要なデータを削除する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			不要なデータを削除する処理を行う
//			トランザクションのトランザクション記述子
//		bool&				incomplete
//			true
//				今回の削除処理でファイルを持つ
//				オブジェクトの一部に処理し残しがある
//			false
//				今回の削除処理でファイルを持つ
//				オブジェクトを完全に処理してきている
//
//				同期処理の結果、ファイルを処理し残したかを設定する
//		bool&				modified
//			true
//				今回の削除処理でファイルを持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の削除処理でファイルを持つ
//				オブジェクトはまだ更新されていない
//
//				削除処理の結果、ファイルが更新されたかを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::compact(Trans::Transaction& trans, bool& incomplete, bool& modified)
{
	_AutoLatch latch(m_pLatchName);
	; _SYDNEY_ASSERT(m_pLatchName == 0
					 || m_pLatchName->isLatched());
	m_pFile->compact(trans, incomplete, modified);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::insert -- データを挿入する
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataArrayData* pTuple_
//			挿入するデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
insert(Common::DataArrayData* pTuple_)
{
	_AutoLatch latch(m_pLatchName,
					 this,
					 File::Operation::Update);
	; _SYDNEY_ASSERT(isNeedLatch(File::Operation::Update) == false
					 || m_pLatchName == 0
					 || m_pLatchName->isLatched());
	m_pFile->insert(pTuple_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::update -- データを更新する
//
//	NOTES
//
//	ARGUMENTS
//		const Common::DataArrayData* pKey_
//			更新対象を特定するキー
//		Common::DataArrayData* pTuple_
//			更新するデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
update(const Common::DataArrayData* pKey_, Common::DataArrayData* pTuple_)
{
	_AutoLatch latch(m_pLatchName,
					 this,
					 File::Operation::Update);
	; _SYDNEY_ASSERT(isNeedLatch(File::Operation::Update) == false
					 || m_pLatchName == 0
					 || m_pLatchName->isLatched());
	m_pFile->update(pKey_, pTuple_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::expunge -- データを削除する
//
//	NOTES
//
//	ARGUMENTS
//		const Common::DataArrayData* pKey_
//			削除対象を特定するキー
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
expunge(const Common::DataArrayData* pKey_)
{
	_AutoLatch latch(m_pLatchName,
					 this,
					 File::Operation::Update);
	; _SYDNEY_ASSERT(isNeedLatch(File::Operation::Update) == false
					 || m_pLatchName == 0
					 || m_pLatchName->isLatched());
	m_pFile->expunge(pKey_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::undoExpunge -- undo expunge
//
//	NOTES
//
//	ARGUMENTS
//		const Common::DataArrayData* pKey_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
undoExpunge(const Common::DataArrayData* pKey_)
{
	_AutoLatch latch(m_pLatchName);
	; _SYDNEY_ASSERT(m_pLatchName == 0
					 || m_pLatchName->isLatched());
	m_pFile->undoExpunge(pKey_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::undoUpdate -- undo update
//
//	NOTES
//
//	ARGUMENTS
//		const Common::DataArrayData* pKey_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
undoUpdate(const Common::DataArrayData* pKey_)
{
	_AutoLatch latch(m_pLatchName);
	; _SYDNEY_ASSERT(m_pLatchName == 0
					 || m_pLatchName->isLatched());
	m_pFile->undoUpdate(pKey_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::fetch -- フェッチの検索条件を設定する
//
//	NOTES
//
//	ARGUMENTS
//		const Common::DataArrayData* pOption_
//			フェッチの検索条件
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
fetch(const Common::DataArrayData* pOption_)
{
	_AutoLatch latch(m_pLatchName,
					 this,
					 File::Operation::Fetch);
	; _SYDNEY_ASSERT(isNeedLatch(File::Operation::Fetch) == false
					 || m_pLatchName == 0
					 || m_pLatchName->isLatched());
	m_pFile->fetch(pOption_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::move -- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Common::StringArrayData& cArea_
//			移動先のパス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
move(Trans::Transaction&			cTrans_,
	 const Common::StringArrayData&	cArea_)
{
	_AutoLatch latch(m_pLatchName);
	; _SYDNEY_ASSERT(m_pLatchName == 0
					 || m_pLatchName->isLatched());
	m_pFile->move(cTrans_, cArea_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::verify -- ファイルを整合性検査する
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Verification::Progress& cProgress_
//			検査結果を格納する変数
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		unsigned int		eTreatment_
//			検査の結果見つかった障害に対する対応を表し、
//			Admin::Verification::Treatment::Value の論理和を指定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
verify(Trans::Transaction& cTrans_,
	   unsigned int uiTreatment_,
	   Admin::Verification::Progress& cProgress_)
{
	_AutoLatch latch(m_pLatchName);
	; _SYDNEY_ASSERT(m_pLatchName == 0
					 || m_pLatchName->isLatched());
	m_pFile->verify(cTrans_, uiTreatment_, cProgress_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::getSize -- ファイルサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		タプル数
//
//	EXCEPTIONS

ModInt64
AutoLogicalFile::
getSize(Trans::Transaction& cTrans_)
{
	_AutoLatch latch(m_pLatchName);
	; _SYDNEY_ASSERT(m_pLatchName == 0
					 || m_pLatchName->isLatched());
	return m_pFile->getSize(cTrans_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::getCount -- タプル数を得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		タプル数
//
//	EXCEPTIONS

ModInt64
AutoLogicalFile::
getCount(Trans::Transaction& cTrans_)
{
	// オープンされているときは使えない
	if (m_bOpened) {
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	OpenOption cOpenOption;

	// ReadモードでEstimate=trueにしてオープンする
	cOpenOption.setInteger(OpenOption::KeyNumber::OpenMode, OpenOption::OpenMode::Read);
	cOpenOption.setBoolean(OpenOption::KeyNumber::Estimate, true);

	ModInt64 iResult = 0;

	open(cTrans_, cOpenOption);
	{
		// スコープを抜けたら自動的にcloseする
		Common::AutoCaller0<AutoLogicalFile> autoCloser(this, &AutoLogicalFile::close);

		{
		_AutoLatch latch(m_pLatchName);
		; _SYDNEY_ASSERT(m_pLatchName == 0
						 || m_pLatchName->isLatched());
		iResult = m_pFile->getCount();
		}
		_SYDNEY_FAKE_ERROR("LogicalFile::AutoLogicalFile_GetCount_Opened",
						   Exception::Unexpected(moduleName, srcFile, __LINE__));
	}

	return iResult;
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::getCost -- コスト見積もりを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		double& dblOverhead_
//			オーバーヘッドコストを入れる返り値
//		double& dblProcessCost_
//			処理コストを入れる返り値
//		double* pdblCount_ = 0
//			If not null, returns estimated count
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
getCost(Trans::Transaction& cTrans_, OpenOption& cOpenOption_,
		double& dblOverhead_, double& dblProcessCost_,
		double* pdblCount_ /* = 0 */)
{
	// オープンされているときは使えない
	if (m_bOpened) {
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	bool bEstimateSet = false;
	if (pdblCount_ == 0
		|| (m_pFile->getCapability() & File::Capability::EstimateCount) == 0) {
		// Estimate=trueにしてオープンする
		cOpenOption_.setBoolean(OpenOption::KeyNumber::Estimate, true);
		bEstimateSet = true;
	}
	open(cTrans_, cOpenOption_);
	{
		// スコープを抜けたら自動的にcloseする
		Common::AutoCaller0<AutoLogicalFile> autoCloser(this, &AutoLogicalFile::close);

		{
		_AutoLatch latch(m_pLatchName,
						 this,
						 File::Operation::GetOverhead
						 | File::Operation::GetProcessCost);
		; _SYDNEY_ASSERT(isNeedLatch(File::Operation::GetOverhead
									 | File::Operation::GetProcessCost) == false
						 || m_pLatchName == 0
						 || m_pLatchName->isLatched());
		dblOverhead_ = m_pFile->getOverhead();
		dblProcessCost_ = m_pFile->getProcessCost();
		}
		if (pdblCount_) {
			_AutoLatch latch(m_pLatchName);
			; _SYDNEY_ASSERT(m_pLatchName == 0
							 || m_pLatchName->isLatched());
			*pdblCount_ = static_cast<double>(m_pFile->getCount());
		}
	}
	if (bEstimateSet) {
		// Estimate=falseに戻す
		cOpenOption_.setBoolean(OpenOption::KeyNumber::Estimate, false);
	}
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::mark -- ファイルをマークする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
mark()
{
	// in mark method, it is not needed to latch
	m_pFile->mark();
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::rewind -- ファイルを巻き戻す
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
rewind()
{
	// in rewind method, it is not needed to latch
	m_pFile->rewind();
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::reset -- カーソルをリセットする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
reset()
{
	_AutoLatch latch(m_pLatchName,
					 this,
					 File::Operation::Reset);
	; _SYDNEY_ASSERT(isNeedLatch(File::Operation::Reset) == false
					 ||m_pLatchName == 0
					 || m_pLatchName->isLatched());
	m_pFile->reset();
}

// FUNCTION public
//	LogicalFile::AutoLogicalFile::isNeedLatch -- 論理ファイルの操作にラッチが必要かを得る
//
// NOTES
//
// ARGUMENTS
//	File::Operation::Value iOperation_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
AutoLogicalFile::
isNeedLatch(File::Operation::Value iOperation_)
{
	// 渡されたオペレーションにNoLatchでないものがあれば必要
	return (m_iNoLatchOperation & iOperation_) != iOperation_;
}

// FUNCTION public
//	LogicalFile::AutoLogicalFile::isKeepLatch -- 
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
AutoLogicalFile::
isKeepLatch()
{
	// 更新操作以外でGetDataがNoLatchであればKeepの必要あり
	return !m_bIsUpdate && !isNeedLatch(File::Operation::GetData);
}

// FUNCTION public
//	LogicalFile::AutoLogicalFile::latch -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	AutoLogicalFile::AutoUnlatch
//
// EXCEPTIONS

AutoLogicalFile::AutoUnlatch
AutoLogicalFile::
latch()
{
	if (m_pLatchName) {
		m_pLatchName->latch();
	}
	return AutoUnlatch(this);
}

// FUNCTION public
//	LogicalFile::AutoLogicalFile::unlatch -- 
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
AutoLogicalFile::
unlatch()
{
	if (m_pLatchName) {
		m_pLatchName->unlatch();
	}
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::toString -- 文字列を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ファイルを表す文字列
//
//	EXCEPTIONS

ModUnicodeString
AutoLogicalFile::
toString() const
{
	return m_pFile->toString();
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::isOpened -- ファイルのオープン状態を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		trueの場合オープンされていることを示す
//		falseの場合クローズされていることを示す
//
//	EXCEPTIONS

bool
AutoLogicalFile::
isOpened() const
{
	return m_bOpened;
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::attach -- ファイルをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//		LogicalFile::FileDriver& cDriver_
//			ファイルドライバー
//		const LogicalFile::FileID& cFileID_
//			アタッチするファイルのファイルID
//
//	RETURN
//		アタッチしたファイル
//
//	EXCEPTIONS

LogicalFile::File*
AutoLogicalFile::
attach(LogicalFile::FileDriver& cDriver_,
	   const LogicalFile::FileID& cFileID_)
{
	// すでにアタッチされていたら一度デタッチする
	detach();

	m_pDriver = &cDriver_;
	m_pFile = cDriver_.attachFile(cFileID_);
	m_iNoLatchOperation = m_pFile->getNoLatchOperation();
	return m_pFile;
}

// FUNCTION public
//	LogicalFile::AutoLogicalFile::attach -- 
//
// NOTES
//
// ARGUMENTS
//	FileDriver& cDriver_
//	const FileID& cFileID_
//	const Lock::Name& cLockName_
//	Trans::Transaction& cTrans_
//	bool bNoLatch_
//	
// RETURN
//	File*
//
// EXCEPTIONS

File*
AutoLogicalFile::
attach(FileDriver& cDriver_, const FileID& cFileID_,
	   const Lock::Name& cLockName_,
	   Trans::Transaction& cTrans_,
	   bool bNoLatch_)
{
	if (!bNoLatch_
		&& !cTrans_.isNoLock()
		&& cTrans_.isNoVersion()) {
		m_pLatchName = new _LatchName(cLockName_, cTrans_);
	}
	return attach(cDriver_, cFileID_);
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::detach -- ファイルをデタッチする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AutoLogicalFile::
detach()
{
	if (m_pDriver) {
		if (m_pFile) {
			if (m_bOpened) {
				try {
					close();

				} catch (...) {
					// close中に例外が発生しても
					// detachFileは行う
					m_bOpened = false;
					m_pDriver->detachFile(m_pFile);
					m_pFile = 0;
					m_pDriver = 0;
					m_iNoLatchOperation = 0;
					delete m_pLatchName, m_pLatchName = 0;
					_SYDNEY_RETHROW;
				}
			}
			m_pDriver->detachFile(m_pFile);
			m_pFile = 0;
			m_iNoLatchOperation = 0;
			delete m_pLatchName, m_pLatchName = 0;
		}
		m_pDriver = 0;
	}
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::isAttached -- アタッチされているかを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		trueの場合アタッチされていることを示す
//		falseの場合アタッチされていないことを示す
//
//	EXCEPTIONS

bool
AutoLogicalFile::
isAttached() const
{
	return m_pFile != 0;
}

//	FUNCTION public
//	LogicalFile::AutoLogicalFile::isAccessible -- ファイルがあるか調べる
//
//	NOTES
//
//	ARGUMENTS
//		bool bForce_
//			物理ファイルの存在まで調べる
//	  		デフォルト: false
//	RETURN
//		なし
//
//	EXCEPTIONS

bool
AutoLogicalFile::
isAccessible(bool bForce_)
{
	return m_pFile->isAccessible(bForce_);
}

// FUNCTION public
//	LogicalFile::AutoLogicalFile::isAbleTo -- File driver supports some features
//
// NOTES
//
// ARGUMENTS
//	File::Capability::Value iCabalitily_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
AutoLogicalFile::
isAbleTo(File::Capability::Value iCapability_)
{
	return (m_pFile->getCapability() & iCapability_) == iCapability_;
}

////////////////////////////////////////////////
// LogicalFile::AutoLogicalFile::AutoUnlatch
////////////////////////////////////////////////

// FUNCTION public
//	LogicalFile::AutoLogicalFile::AutoUnlatch::latch -- explicit latch
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
AutoLogicalFile::AutoUnlatch::
latch()
{
	if (m_cAutoCaller.isOwner() == false) {
		AutoUnlatch cTmp = m_pFile->latch();
		*this = cTmp;
	}
}

// FUNCTION public
//	LogicalFile::AutoLogicalFile::AutoUnlatch::unlatch -- explicit unlatch
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
AutoLogicalFile::AutoUnlatch::
unlatch()
{
	m_cAutoCaller.free();
}

//
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
