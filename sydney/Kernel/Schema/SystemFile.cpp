// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SystemFile.cpp -- システム表を構成するファイル関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Schema/AutoLatch.h"
#include "Schema/SystemFile.h"
#include "Schema/SystemTable.h"
#include "Schema/Database.h"
#include "Schema/FakeError.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/OpenOption.h"
#include "Schema/Parameter.h"
#include "Schema/PathParts.h"
#include "Schema/Table.h"
#include "Schema/TreeNode.h"
#include "Schema/Utility.h"
#include "Schema/Message_MetaIndexNotMatch.h"
#include "Schema/Message_MetaIndexTupleNotFound.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerArrayData.h"

#include "Exception/MetaDatabaseCorrupted.h"
#include "Exception/Unexpected.h"

#include "LogicalFile/FileID.h"
#include "LogicalFile/OpenOption.h"

#include "FileCommon/FileOption.h"
#include "FileCommon/OpenOption.h"

#include "Trans/Transaction.h"

#include "ModAutoPointer.h"
#include "ModError.h"
#include "ModException.h"
#include "ModHashMap.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

#define	_BEGIN_REORGANIZE_RECOVERY	\
								/* UNDO中の場合エラー処理をしない */ \
								if (!bUndo_ && Schema::Database::isAvailable(getDatabaseID())) { \
									try {
#define _END_REORGANIZE_RECOVERY \
									} catch (Exception::Object& e) { \
										SydErrorMessage << "Error recovery failed. FATAL. " << e << ModEndl; \
										/* データベースを使用不可能にする*/ \
										Schema::Database::setAvailability(getDatabaseID(), false); \
										/* エラー処理中に発生した例外は再送しない */ \
										/* thru. */ \
									} catch (...) { \
										SydErrorMessage << "Error recovery failed. FATAL." << ModEndl; \
										/* データベースを使用不可能にする*/ \
										Schema::Database::setAvailability(getDatabaseID(), false); \
										/* エラー処理中に発生した例外は再送しない */ \
										/* thru. */ \
									} \
								}

namespace {

	const ModSize _iMinimumIndexNumber = 2;		// 最低限作られる索引の数

	bool _isObjectToBeDeleted(Schema::Object::Status::Value eStatus_)
	{
		return (eStatus_ == Schema::Object::Status::Deleted
				|| eStatus_ == Schema::Object::Status::DeletedInRecovery);
	}
}

//////////////////////////////////////
//	Schema::SystemTable::SystemFile	//
//////////////////////////////////////

//	FUNCTION public
//	Schema::SystemTable::SystemFile::SystemFile -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Category::Value eCategory_
//			システム表に格納されるオブジェクトの種別
//		const Os::Path& cPath_
//			システムファイルを格納するパス名
//		SystemTable::Attribute::Value
//			システム表ファイルの属性
//
//	RETURN
//		なし
//
//	EXCEPTIONS

SystemTable::SystemFile::
SystemFile(Schema::Object::Category::Value eCategory_,
		   const Os::Path& cPathBase_,
		   const Os::Path& cPathPart_,
		   SystemTable::Attribute::Value eAttr_)
	: m_eCategory(eCategory_),
	  m_cPath(cPathBase_), m_cPathPart(cPathPart_), m_pRecord(0), m_pIndex(0),
	  m_eOpenedMode(OpenMode::Unknown), m_pOpenedIndex(0), m_eAttr(eAttr_)
{
	m_cPath.addPart(m_cPathPart);
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::~SystemFile -- デストラクター
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

SystemTable::SystemFile::
~SystemFile()
{
	destruct();
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::initialize --
//		システム表を構成するファイルを初期化する
//
//	NOTES
//
//	ARGUMENTS
//		bool bMounted_ = true
//			システム表がマウントされているかを示す
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
initialize(bool bMounted_)
{
	initializeRecord(getAreaPath(), bMounted_);

	if (m_pIndex) {
		ModSize n = m_pIndex->getSize();
		for (ModSize i = 0; i < n; i++)
			initializeIndex(getAreaPath(), (*m_pIndex)[i], bMounted_);
	}
}

//	FUNCTION protected
//	Schema::SystemTable::SystemFile::initializeRecord --
//		システム表を構成するファイルのうちレコードファイルを初期化する
//
//	NOTES
//
//	ARGUMENTS
//		const Os::Path& cPath_
//			システム表のパス名
//		bool bMounted_ = true
//			システム表がマウントされているかを示す
// 
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
initializeRecord(const Os::Path& cPath_, bool bMounted_)
{
	if (!m_pRecord)
		m_pRecord = new RecordFile();
	m_pRecord->initialize(*this, cPath_, m_eAttr, bMounted_);
}

//	FUNCTION protected
//	Schema::SystemTable::SystemFile::initializeIndex --
//		システム表を構成するファイルのうち索引ファイルを初期化する
//
//	NOTES
//
//	ARGUMENTS
//		const Os::Path& cPath_
//			システム表のパス名
//		Schema::SystemTable::IndexFile* pIndex_
//			初期化する索引ファイル
//		bool bMounted_ = true
//			システム表がマウントされているかを示す
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
initializeIndex(const Os::Path& cPath_, IndexFile* pIndex_, bool bMounted_)
{
	; _SYDNEY_ASSERT(pIndex_);
	pIndex_->initialize(*this, cPath_, m_eAttr, bMounted_);
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::terminate --
//		システム表を構成するファイルの後処理
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
SystemTable::SystemFile::
terminate()
{
	destruct();
}

//	FUNCTION private
//	Schema::SystemTable::SystemFile::destruct --
//		デストラクターの下位関数
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
SystemTable::SystemFile::
destruct()
{
	// ★注意★
	// ファイルの後処理はRecordFile、IndexFIleのデストラクターの中で
	// 実行される

	if (m_pRecord)
		delete m_pRecord, m_pRecord = 0;
	if (m_pIndex) {
		ModSize n = m_pIndex->getSize();
		for (ModSize i = 0; i < n; i++)
			delete (*m_pIndex)[i];
		delete m_pIndex, m_pIndex = 0;
	}
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::create -- システム表の保管場所を生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//		bool bAllowExistence_ = false
//			存在していてもエラーにしない
//			(redo用)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
create(Trans::Transaction& cTrans_, bool bAllowExistence_)
{
	// 論理ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(cTrans_, *this);

	// ファイルを格納するディレクトリーを作る
	// ★注意★
	// attachFileの前にディレクトリーが存在していなければならないので
	// initializeの前に生成する

	Utility::File::AutoRmDir record;
	record.setDir(Os::Path(getAreaPath()).addPart(PathParts::SystemTable::Record));
	SCHEMA_FAKE_ERROR("Schema::SystemFile", "Create", "Directory");

	// レコードファイルと索引ファイルを別々に初期化する
	initializeRecord(getAreaPath(), false /* not mounted */);
	; _SYDNEY_ASSERT(m_pRecord);

	if (bAllowExistence_ && m_pRecord->isAccessible()) {
		// 存在したら破棄する
		m_pRecord->drop(cTrans_);
		// もう一度初期化しなおす
		initializeRecord(getAreaPath(), false /* not mounted */);
	}

	// ファイルを生成する
	(void) m_pRecord->create(cTrans_);

	try {
		SCHEMA_FAKE_ERROR("Schema::SystemFile", "Create", "Record");

		if (m_pIndex) {
			// 索引ファイルも生成する
			ModSize n = m_pIndex->getSize();
			// エラー発生時に自動的にmkdirを取り消すためのクラス
			ModVector<Utility::File::AutoRmDir> vecAutoRmDir(n);
			ModSize iCreatedIndex = 0; // エラー処理に使う

			try {
				for (ModSize i = 0; i < n; ++i) {
					IndexFile* pIndex = (*m_pIndex)[i];

					// ファイルを格納するディレクトリーを作る
					vecAutoRmDir[i].setDir(Os::Path(
										 getAreaPath()).addPart(pIndex->getPathPart()));

					// 索引ファイルを初期化する
					initializeIndex(getAreaPath(), pIndex, false /* not mounted */);

					if (bAllowExistence_ && pIndex->isAccessible()) {
						// 存在したら破棄する
						pIndex->drop(cTrans_);
						// もう一度初期化しなおす
						initializeIndex(getAreaPath(), pIndex, false /* not mounted */);
					}
					// ファイルを生成する
					pIndex->create(cTrans_);
					// ファイル作成が成功した番号を覚える
					iCreatedIndex = i + 1;
				}
				SCHEMA_FAKE_ERROR("Schema::SystemFile", "Create", "Index");

			} catch (...) {
				// 作成してしまっている索引ファイルを削除する
				for (ModSize i = 0; i < iCreatedIndex; ++i) {
					(*m_pIndex)[i]->drop(cTrans_);
				}
				_SYDNEY_RETHROW;
			}

			// 成功したのでエラー処理用のクラスをdisableする
			for (ModSize i = 0; i < n; ++i) {
				vecAutoRmDir[i].disable();
			}
		}
	} catch (...) {
		// レコードファイルを削除する
		m_pRecord->drop(cTrans_);
		_SYDNEY_RETHROW;
	}

	// 成功したのでエラー処理用のクラスをdisableする
	record.disable();
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::drop -- システム表の保管場所を抹消する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::drop(Trans::Transaction& cTrans_)
{
	// 論理ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(cTrans_, *this);

	// 初期化する
	initialize();

	// ファイルを破棄する

	; _SYDNEY_ASSERT(m_pRecord);
	m_pRecord->drop(cTrans_);

	if (m_pIndex) {
		ModSize n = m_pIndex->getSize();
		// 索引ファイルも破棄する
		for (ModSize i = 0; i < n; i++)
			(*m_pIndex)[i]->drop(cTrans_);
	}

	// ファイルを格納するディレクトリーを削除する

	Utility::File::rmAll(getAreaPath());

	// 後処理までしてしまう
	terminate();
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::move -- システム表の保管場所を移動する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//		const Os::Path& cPrevPath_
//			移動前のパス名
//		const Os::Path& cNewPath_
//			移動先のパス名
//		bool bUndo_ = false
//			trueの場合エラー処理中なので重ねてのエラー処理はしない
//		bool bRecovery_ = false
//			trueなら変更前のパスにファイルがなくても変更後にあればOKとする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
move(Trans::Transaction& cTrans_,
	 const Os::Path& cPrevPath_, const Os::Path& cNewPath_,
	 bool bUndo_, bool bRecovery_)
{
	// 現在と新しいパス名を入れる
	Os::Path cPrevPath(cPrevPath_);
	Os::Path cNewPath(cNewPath_);
	// SystemFileのパス名はBase+PathPartで作成されるので付加する
	cPrevPath.addPart(m_cPathPart);
	cNewPath.addPart(m_cPathPart);

	enum {
		None,							// 初期値
		DirectoryCreated,				// 新ディレクトリー作成
		RecordMoved,					// レコードファイル移動完了
		IndexMoved,						// 索引ファイル移動
		DirectoryRemoved,				// 旧ディレクトリー破棄
		SetPath,						// パス設定
		ValueNum
	} eStatus = None;

	ModSize	iMovedIndex = 0;			// 索引の移動は移動した数で把握

	// 論理ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(cTrans_, *this);

	// 移動先のディレクトリーを設定する
	Utility::File::AutoRmDir cAutoRmDir;
	cAutoRmDir.setDir(cNewPath);

	try {
		eStatus = DirectoryCreated;
		SCHEMA_FAKE_ERROR("Schema::SystemFile", "Move", "DirectoryCreated");

		// 移動前のパスで初期化する
		initializeRecord(cPrevPath);

		// ファイルを移動する
		m_pRecord->move(cTrans_, cNewPath, bUndo_, bRecovery_), eStatus = RecordMoved;
		// 移動したら移動先のディレクトリーを自動で破棄できない
		cAutoRmDir.disable();
		SCHEMA_FAKE_ERROR("Schema::SystemFile", "Move", "Record");

		if (m_pIndex) {
			// 索引ファイルも移動する
			ModSize n = m_pIndex->getSize();
			for (ModSize i = 0; i < n; i++) {
				IndexFile* pIndex = (*m_pIndex)[i];

				// 移動前のパスで初期化する
				initializeIndex(cPrevPath, pIndex);
		
				// ファイルを移動する
				pIndex->move(cTrans_, cNewPath, bUndo_, bRecovery_), eStatus = IndexMoved;

				iMovedIndex = i + 1;
			}
			SCHEMA_FAKE_ERROR("Schema::SystemFile", "Move", "Index");
		}

		// 移動前のディレクトリーを破棄する
		Utility::File::rmAll(cPrevPath), eStatus = DirectoryRemoved;
		SCHEMA_FAKE_ERROR("Schema::SystemFile", "Move", "DirectoryRemoved");

		// 新しい格納場所をセットする
		m_cPath = cNewPath, eStatus = SetPath;
		SCHEMA_FAKE_ERROR("Schema::SystemFile", "Move", "SetPath");

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY;

		switch (eStatus) {
		case SetPath:
			m_cPath = cPrevPath;
		case DirectoryRemoved:
		case IndexMoved:
			{
				// 索引も移動されたので元に戻す
				for (ModSize iErr = 0; iErr < iMovedIndex; ++iErr) {
					// 移動を取り消す
					m_pIndex->at(iErr)->move(cTrans_, cPrevPath, true);
				}
			}
		case RecordMoved:
			{
				// 移動を取り消す
				m_pRecord->move(cTrans_, cPrevPath, true);
				// 移動したら再び作成したディレクトリーを破棄できる
				cAutoRmDir.enable();
			}
		case DirectoryCreated:
		case None:
		default:
			break;
		}

		_END_REORGANIZE_RECOVERY;

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::mount -- ファイルを mount する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//		bool bUndo_ = false
//			trueのときUNDO中なのでエラー処理を重ねてしない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::mount(Trans::Transaction& cTrans_, bool bUndo_ /* = false */)
{
	// 論理ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(cTrans_, *this);

	// ファイルを初期化する
	initialize(false /* not mounted */);
	; _SYDNEY_ASSERT(m_pRecord);

	// ファイルを mount する
	m_pRecord->mount(cTrans_);

	if (m_pIndex) {
		ModSize n = m_pIndex->getSize();
		ModSize last = 0;
		try {
			for (ModSize i = 0; i < n; ++i) {
				// ファイルを mount する
				(*m_pIndex)[i]->mount(cTrans_);
				last = i + 1;
			}
		} catch (...) {

			_BEGIN_REORGANIZE_RECOVERY;

			for (ModSize i = 0; i < last; ++i) {
				(*m_pIndex)[i]->unmount(cTrans_);
			}
			m_pRecord->unmount(cTrans_);

			_END_REORGANIZE_RECOVERY;			

			_SYDNEY_RETHROW;
		}
	}
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::unmount -- ファイルを unmount する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::unmount(Trans::Transaction& cTrans_, bool bUndo_ /* = false */)
{
	// 論理ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(cTrans_, *this);

	// ファイルを初期化する
	initialize();
	; _SYDNEY_ASSERT(m_pRecord);

	// ファイルを unmount する
	m_pRecord->unmount(cTrans_);

	if (m_pIndex) {
		ModSize n = m_pIndex->getSize();
		ModSize last = 0;
		try {
			for (ModSize i = 0; i < n; i++) {
				// ファイルを unmount する
				(*m_pIndex)[i]->unmount(cTrans_);
				last = i + 1;
			}
		} catch (...) {

			_BEGIN_REORGANIZE_RECOVERY;

			for (ModSize i = 0; i < last; ++i) {
				(*m_pIndex)[i]->mount(cTrans_);
			}
			m_pRecord->mount(cTrans_);

			_END_REORGANIZE_RECOVERY;			

			_SYDNEY_RETHROW;
		}
	}
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::flush -- ファイルを flush する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::flush(Trans::Transaction& cTrans_)
{
	// 論理ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(cTrans_, *this);

	// ファイルを初期化する
	initialize();
	; _SYDNEY_ASSERT(m_pRecord);

	// ファイルを flush する
	m_pRecord->flush(cTrans_);

	if (m_pIndex) {
		ModSize n = m_pIndex->getSize();
		// 索引ファイルも生成する
		for (ModSize i = 0; i < n; i++) {
			IndexFile* pIndex = (*m_pIndex)[i];

			// ファイルを unmounする
			pIndex->flush(cTrans_);
		}
	}
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::sync -- 不要な版を破棄する
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
SystemTable::SystemFile::sync(
	Trans::Transaction& trans, bool& incomplete, bool& modified)
{
	// 論理ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(trans, *this);

	initialize();

	; _SYDNEY_ASSERT(m_pRecord);
	m_pRecord->sync(trans, incomplete, modified);

	if (m_pIndex) {
		const ModSize n = m_pIndex->getSize();
		for (ModSize i = 0; i < n; ++i)
			(*m_pIndex)[i]->sync(trans, incomplete, modified);
	}
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::startBackup -- バックアップを開始する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//		bool bRestorable_ = true
//		bool bUndo_ = false
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
startBackup(Trans::Transaction& cTrans_, bool bRestorable_ /* = true */, bool bUndo_ /* = false */)
{
	// 論理ファイル操作の排他制御のためにラッチする
	//
	//【注意】	版を使用するトランザクションでも
	//			バックアップの開始時に更新操作を行う可能性があるので、
	//			とにかくラッチする

	AutoLatch	latch(cTrans_, *this, true);

	// ファイルを初期化する
	initialize();
	; _SYDNEY_ASSERT(m_pRecord);

	// ファイルを startBackup する
	m_pRecord->startBackup(cTrans_, bRestorable_);

	if (m_pIndex) {
		ModSize n = m_pIndex->getSize();
		ModSize last = 0;
		try {
			for (ModSize i = 0; i < n; i++) {
				(*m_pIndex)[i]->startBackup(cTrans_, bRestorable_);
				last = i + 1;
			}
		} catch (...) {

			_BEGIN_REORGANIZE_RECOVERY;

			for (ModSize i = 0; i < last; ++i) {
				(*m_pIndex)[i]->endBackup(cTrans_);
			}
			m_pRecord->endBackup(cTrans_);

			_END_REORGANIZE_RECOVERY;			

			_SYDNEY_RETHROW;
		}
	}
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::endBackup--
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::endBackup(Trans::Transaction& cTrans_)
{
	// 論理ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(cTrans_, *this);

	// ファイルを初期化する
	initialize();
	; _SYDNEY_ASSERT(m_pRecord);

	try {
		// ファイルを endBackup する
		m_pRecord->endBackup(cTrans_);

		if (m_pIndex) {
			ModSize n = m_pIndex->getSize();
			for (ModSize i = 0; i < n; i++)
				(*m_pIndex)[i]->endBackup(cTrans_);
		}
	} catch (...) {
		// endBackupに失敗したら利用不可にする
		Schema::Database::setAvailability(getDatabaseID(), false);
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::recover -- 障害から回復する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
recover(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_)
{
	// 論理ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(cTrans_, *this);

	// ファイルを初期化する
	initialize();
	; _SYDNEY_ASSERT(m_pRecord);

	// ファイルを recover する
	m_pRecord->recover(cTrans_, cPoint_);

	if (m_pIndex) {
		ModSize n = m_pIndex->getSize();
		// 索引ファイルも生成する
		for (ModSize i = 0; i < n; i++) {
			IndexFile* pIndex = (*m_pIndex)[i];

			// ファイルを recoverする
			pIndex->recover(cTrans_, cPoint_);
		}
	}
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::restore --
//		ある時点に開始された版管理するトランザクションが
//		参照する版を最新版とする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
restore(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_)
{
	// 論理ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(cTrans_, *this);

	// ファイルを初期化する
	initialize();
	; _SYDNEY_ASSERT(m_pRecord);

	// ファイルを restore する
	m_pRecord->restore(cTrans_, cPoint_);

	if (m_pIndex) {
		ModSize n = m_pIndex->getSize();
		// 索引ファイルも生成する
		for (ModSize i = 0; i < n; i++) {
			IndexFile* pIndex = (*m_pIndex)[i];

			// ファイルを restoreする
			pIndex->restore(cTrans_, cPoint_);
		}
	}
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::open --
//		システム表の保管場所であるファイルをオープンする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			オープンするトランザクションのトランザクションID
//		Schema::SystemTable::OpenMode::Value	mode
//			システム表の保管場所であるファイルを何のためにオープンするか
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
open(Trans::Transaction& cTrans_, OpenMode::Value mode)
{
	// 論理ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(cTrans_, *this);

	// まずレコードのみを初期化する
	initializeRecord(getAreaPath());
	; _SYDNEY_ASSERT(m_pRecord);

	// SystemFileがデストラクトされると自動的にcloseされるので
	// openの途中でエラーが発生してもエラー処理をしなくてもよい

	switch (mode) {
	case OpenMode::Read:
	{
		LogicalFile::OpenOption cOpenOption;	// レコードファイルの
												// オープンオプション

		// レコードファイルのスキャンアクセス

		if (!m_pRecord->getFile().getSearchParameter(0, cOpenOption)) {
			_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
		}

		// 使うのはレコードファイルのみ

		m_pRecord->open(cTrans_, cOpenOption);
		break;
	}

	case OpenMode::Update:
	{
		LogicalFile::OpenOption cOpenOption;	// レコードファイルの
												// オープンオプション

		// 更新操作はすべて同じオープンモードである
		// ★注意★
		// 修正で対象のフィールドを指定することはしない
		// すなわちupdateに対しては変更がないフィールドも入力される

		OpenOption::setOpenMode(cOpenOption,
								_SYDNEY_SCHEMA_PARAMETER_KEY(
										FileCommon::OpenOption::OpenMode::Key),
								_SYDNEY_SCHEMA_PARAMETER_VALUE(
										FileCommon::OpenOption::OpenMode::Update));

		// レコードファイルをオープンする

		m_pRecord->open(cTrans_, cOpenOption);

		if (!m_pIndex) break;

		ModSize n = m_pIndex->getSize();
		for (ModSize i = 0; i < n; i++) {
			IndexFile* pIndex = (*m_pIndex)[i];

			// 索引ファイルを初期化する
			initializeIndex(getAreaPath(), pIndex);

			LogicalFile::OpenOption cBOption;	// 索引ファイルの
												// オープンオプション

			OpenOption::setOpenMode(cBOption,
									_SYDNEY_SCHEMA_PARAMETER_KEY(
										FileCommon::OpenOption::OpenMode::Key),
									_SYDNEY_SCHEMA_PARAMETER_VALUE(
										FileCommon::OpenOption::OpenMode::Update));

			// 更新用ファイルをオープンする

			pIndex->open(cTrans_, cBOption);
		}

		break;
	}

	default:
		; _SYDNEY_ASSERT(false);
	}
	m_eOpenedMode = mode;
	m_pOpenedIndex = 0;
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::open --
//		システム表の保管場所であるファイルをオープンする
//
//	NOTES
//		親オブジェクトのIDを指定して取得する
//		索引があるシステム表でなければならない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			オープンするトランザクションのトランザクションID
//		Schema::SystemTable::IndexFile* pIndex_
//			検索に使う索引ファイル
//		Schema::Object::ID::Value	iID_
//			データを取得する条件となるオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
open(Trans::Transaction& cTrans_,
	 IndexFile* pIndex_, Schema::Object::ID::Value iID_)
{
	; _SYDNEY_ASSERT(pIndex_);

	// 論理ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(cTrans_, *this);

	// レコードファイルと検索に使用する索引ファイルを初期化する
	initializeRecord(getAreaPath());
	initializeIndex(getAreaPath(), pIndex_);
	; _SYDNEY_ASSERT(m_pRecord);

	LogicalFile::OpenOption cOpenOption;	// レコードファイルの
											// オープンオプション
	LogicalFile::OpenOption cIndexOption;	// B+木ファイルの
											// オープンオプション

	// B+木ファイルには「キー=iID_」という条件を渡す

	TreeNode::Field cIDField(pIndex_->getKeyPosition());
	TreeNode::Value cIDValue(iID_);
	TreeNode::Equals cEquals(cIDField, cIDValue);
												// B+木ファイルに与える条件

	// レコードファイルには「OIDでFetchする」という条件を渡す

	TreeNode::List cFetchFields;
	cFetchFields.addNode(new TreeNode::Field(0));// OIDは常にレコードファイルの
												 // 0番目のフィールドである
	TreeNode::Fetch cFetch(cFetchFields);		 // レコードファイルに与える条件

	// 索引ファイルからはバリューだけを取得する
	TreeNode::Field cValueField(pIndex_->getValuePosition());

	// B+木ファイルに対して条件と取得フィールドを設定
	// レコードファイルに対して条件を設定

	if (!pIndex_->getFile().getSearchParameter(&cEquals, cIndexOption)
		|| !pIndex_->getFile().getProjectionParameter(&cValueField, cIndexOption)
		|| !m_pRecord->getFile().getSearchParameter(&cFetch, cOpenOption)) {
		_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
	}

	// SystemFileがデストラクトされると自動的にcloseされるので
	// openの途中でエラーが発生してもエラー処理をしなくてもよい

	// レコードファイルをオープンする

	m_pRecord->open(cTrans_, cOpenOption);

	// 索引ファイルをオープンする
	pIndex_->open(cTrans_, cIndexOption);

	m_eOpenedMode = OpenMode::Search;
	m_pOpenedIndex = const_cast<IndexFile*>(pIndex_);
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::close --
//		システム表の保管場所であるファイルをクローズする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			ファイルをクローズするトランザクションのトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::close(Trans::Transaction& trans)
{
	if (m_eOpenedMode != OpenMode::Unknown) {

		// 論理ファイル操作の排他制御のためにラッチする

		AutoLatch	latch(trans, *this);

		// 初期化されているはず
		; _SYDNEY_ASSERT(m_pRecord);

		m_pRecord->close();

		if (m_pIndex) {
			ModSize n = m_pIndex->getSize();
			for (ModSize i = 0; i < n; i++)
				(*m_pIndex)[i]->close();
		}

		// オープンされているモードを初期状態にする

		m_eOpenedMode = OpenMode::Unknown;
	}
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::getData -- システム表のタプルを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			タプルを得るトランザクションのトランザクション記述子
//		Common::DataArrayData& cData_
//			ファイルから取得したデータを格納するDataArrayData
//
//	RETURN
//		trueの場合データが取得できた
//		falseの場合データが尽きた
//
//	EXCEPTIONS

bool
SystemTable::SystemFile::getData(Trans::Transaction& trans, Common::DataArrayData& cData_)
{
	// 論理ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(trans, *this);

	// 初期化されているはず
	; _SYDNEY_ASSERT(m_pRecord);

	bool bResult = false;

	switch (m_eOpenedMode) {
	case OpenMode::Read:
	{
		; _SYDNEY_ASSERT(!m_pOpenedIndex);

		// スキャンアクセスは単純にレコードファイルから1件得ればよい

		bResult = m_pRecord->getFile().getData(&cData_);
#ifdef DEBUG
		if (bResult)
			SydSchemaDebugMessage << "SystemFile::getData(Record) = " << cData_.toString() << ModEndl;
#endif
		break;
	}
	case OpenMode::Search:
	{
		; _SYDNEY_ASSERT(m_pOpenedIndex);
		// オープンされた索引のバリュータイプはOIDであるはず
		; _SYDNEY_ASSERT(m_pOpenedIndex->getValueType() == LogicalFile::ObjectID().getType());

		// 索引を使った検索の場合、
		// 索引ファイルから読んでレコードファイルをFetchする

		Common::DataArrayData& cKey = m_pOpenedIndex->getTupleData();
		if (!m_pOpenedIndex->getFile().getData(&cKey))
			break;
#ifdef DEBUG
		SydSchemaDebugMessage << "SystemFile::getData(Index) = " << cKey.toString() << ModEndl;
#endif

		// IndexからはレコードファイルのOIDだけが取得されているはずであるから
		// そのままFetchの引数にしてよい

		m_pRecord->getFile().fetch(&cKey);
		bResult = m_pRecord->getFile().getData(&cData_);
#ifdef DEBUG
		if (bResult)
			SydSchemaDebugMessage << "SystemFile::getData(Record) = " << cData_.toString() << ModEndl;
#endif

		// ★注意★
		// システム表を変更するような操作は1スレッドでしか動かないので
		// ここで得られないことはないはず
		// NotAvailableのときはその限りではない

		; _SYDNEY_ASSERT(bResult || !Schema::Database::isAvailable(getDatabaseID()));
		break;
	}
	default:
		; _SYDNEY_ASSERT(false);
		break;
	}

	return bResult;
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::loadID -- 索引ファイルからIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			IDを得るトランザクションのトランザクション記述子
//
//	RETURN
//		Schema::Object::ID::Value
//			ID
//
//	EXCEPTIONS

Object::ID::Value
SystemTable::SystemFile::loadID(Trans::Transaction& trans)
{
	// 論理ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(trans, *this);

	Schema::Object::ID::Value result = Schema::Object::ID::Invalid;

	// 索引と条件が指定されたオープンの後にしか呼ばれないはず
	; _SYDNEY_ASSERT(m_eOpenedMode == OpenMode::Search);	
	; _SYDNEY_ASSERT(m_pOpenedIndex);

	// オープンされた索引のバリュータイプはIDであるはず
	; _SYDNEY_ASSERT(m_pOpenedIndex->getValueType() == Schema::Object::ID().getType());

	// 索引ファイルから読みこむ
	Common::DataArrayData& cData = m_pOpenedIndex->getTupleData();
	if (m_pOpenedIndex->getFile().getData(&cData)) {
#ifdef DEBUG
		SydSchemaDebugMessage << "SystemFile::getData(ID) = " << cData.toString() << ModEndl;
#endif

		// 0番目にIDが入っているはずである
		; _SYDNEY_ASSERT(cData.getElement(0)->getType() == Schema::Object::ID().getType());
#ifdef DEBUG
		bool bRet =
#endif
		Schema::Object::unpack(cData.getElement(0).get(), result);
#ifdef DEBUG
		; _SYDNEY_ASSERT(bRet);
#endif
	}
	return result;
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::insert -- システム表にタプルを挿入する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			タプルを挿入するトランザクションのトランザクション記述子
//		Common::DataArrayData* pTuple_
//			挿入するシステム表のタプルデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
insert(Trans::Transaction& trans, Common::DataArrayData* pTuple_)
{
	// 論理ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(trans, *this);

	; _SYDNEY_ASSERT(m_eOpenedMode == OpenMode::Update);

	// 初期化されているはず
	; _SYDNEY_ASSERT(m_pRecord);

	// ★注意★
	// IDの一意性チェックは行わない
	// ログは再構成要求のSQL文に対してとられているのでここでは不要

	if (pTuple_) {

		// レコードファイルに挿入する
		m_pRecord->getFile().insert(pTuple_);
						// pTuple_の0番目要素に新たに割り当てられたOIDが入る
#ifdef DEBUG
		SydSchemaDebugMessage << "SystemFile::insert(Record) = " << pTuple_->toString() << ModEndl;
#endif

		if (m_pIndex) {
			ModSize iInsertedIndex = 0;
			try {
				SCHEMA_FAKE_ERROR("Schema::SystemFile", "Insert", "Record");
				ModSize n = m_pIndex->getSize();
				for (ModSize i = 0; i < n; i++) {
					IndexFile* pIndex = (*m_pIndex)[i];

					// 索引ファイルに挿入する
					insertIndex(trans, pTuple_, pIndex);

					iInsertedIndex = i + 1;
					SCHEMA_FAKE_ERROR("Schema::SystemFile", "Insert", "Index");
				}

			} catch (...) {

				// 索引ファイルの挿入を取り消す
				for (ModSize i = 0; i < iInsertedIndex; ++i) {
					IndexFile* pIndex = (*m_pIndex)[i];

					expungeIndex(trans, pTuple_, pIndex, true /* undo */);
				}

				// レコードファイルの挿入を取り消す
				Common::Data::Pointer pOID = pTuple_->getElement(0);
				ModAutoPointer<Common::DataArrayData> pRecordKey = new Common::DataArrayData;
				pRecordKey->reserve(1);
				pRecordKey->pushBack(pOID);
				m_pRecord->getFile().expunge(pRecordKey);
#ifdef DEBUG
				SydSchemaDebugMessage << "SystemFile::expunge(Record) = " << pRecordKey->toString() << ModEndl;
#endif

				_SYDNEY_RETHROW;
			}
		}
	}
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::insertIndex -- システム表の索引ファイルに挿入する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			挿入するトランザクションのトランザクション記述子
//		Common::DataArrayData* pTuple_
//			挿入するシステム表のタプルデータ
//		Schema::SystemTable::IndexFile* pIndex_
//			挿入する索引ファイル
//		bool bUndo_ = false
//			trueのときエラー処理中なので重ねてエラー処理しない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
insertIndex(Trans::Transaction& trans,
			Common::DataArrayData* pTuple_,
			IndexFile* pIndex_,
			bool bUndo_ /* = false */)
{
	ModAutoPointer<Common::DataArrayData> pIndexTuple =
		pIndex_->makeInsertTuple(pTuple_);

	// 索引ファイルに挿入する
	pIndex_->getFile().insert(pIndexTuple);
#ifdef DEBUG
	SydSchemaDebugMessage << "SystemFile::insert(Index) = " << pIndexTuple->toString() << ModEndl;
#endif
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::update -- システム表のタプルを修正する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			タプルを修正するトランザクションのトランザクション記述子
//		Common::Data* pKey_
//			修正するタプルを指すキー(OID)
//		Common::DataArrayData* pTuple_
//			修正するシステム表のタプルデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
update(Trans::Transaction& trans,
	   const Common::DataArrayData* pKey_, Common::DataArrayData* pTuple_)
{
	// 論理ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(trans, *this);

	; _SYDNEY_ASSERT(m_eOpenedMode == OpenMode::Update);

	// 初期化されているはず
	; _SYDNEY_ASSERT(m_pRecord);

	// ★注意★
	// ログは再構成要求のSQL文に対してとられているのでここでは不要

	if (pKey_ && pTuple_) {

		// レコードファイルを修正する

		m_pRecord->getFile().update(pKey_, pTuple_);
#ifdef DEBUG
		SydSchemaDebugMessage << "SystemFile::update(Record) = key:" << pKey_->toString() << " value: " << pTuple_->toString() << ModEndl;
#endif

		// 索引の条件になるフィールドは修正されないものだけである
	}
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::expunge -- システム表のタプルを削除する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			タプルを削除するトランザクションのトランザクション記述子
//		const Common::DataArrayData* pKey_
//			削除するタプルを指すキー(OID)
//		Common::DataArrayData* pTuple_
//			削除するシステム表のタプルデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
expunge(Trans::Transaction& trans, const Common::DataArrayData* pKey_,
		Common::DataArrayData* pTuple_)
{
	// 論理ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(trans, *this);

	; _SYDNEY_ASSERT(m_eOpenedMode == OpenMode::Update);

	// 初期化されているはず
	; _SYDNEY_ASSERT(m_pRecord);

	// ★注意★
	// ログは再構成要求のSQL文に対してとられているのでここでは不要

	if (pKey_) {

		ModSize iExpungedIndex = 0;
		Common::Data::Pointer pData;
		try {

			// 索引があるならそれを削除する
			if (m_pIndex) {

				ModSize n = m_pIndex->getSize();
				for (ModSize i = 0; i < n; i++) {
					IndexFile* pIndex = (*m_pIndex)[i];

					expungeIndex(trans, pTuple_, pIndex);

					iExpungedIndex = i + 1;
					SCHEMA_FAKE_ERROR("Schema::SystemFile", "Expunge", "Index");
				}
			}

			SCHEMA_FAKE_ERROR("Schema::SystemFile", "Expunge", "Record");

			// レコードファイルから削除する
			m_pRecord->getFile().expunge(pKey_);
#ifdef DEBUG
			SydSchemaDebugMessage << "SystemFile::expunge(Record) = " << pKey_->toString() << ModEndl;
#endif

		} catch (...) {
			// レコードファイルの削除以降に例外を投げるような処理はないので
			// ここに来るときはレコードファイルの削除は成功していないものとしてよい
			// したがって索引ファイルの削除を取り消すことで全体の処理を取り消すことができる
			if (m_pIndex) {
				for (ModSize i = 0; i < iExpungedIndex; ++i) {
					IndexFile* pIndex = (*m_pIndex)[i];

					insertIndex(trans, pTuple_, pIndex, true /* undo */);
				}
			}

			_SYDNEY_RETHROW;
		}
	}
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::expungeIndex -- システム表の索引ファイルから削除する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			削除するトランザクションのトランザクション記述子
//		Common::DataArrayData* pTuple_
//			削除するシステム表のタプルデータ
//		Schema::SystemTable::IndexFile* pIndex_
//			削除する索引ファイル
//		bool bUndo_ = false
//			trueのときエラー処理中なので重ねてエラー処理しない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
expungeIndex(Trans::Transaction& trans,
			 Common::DataArrayData* pTuple_,
			 IndexFile* pIndex_,
			 bool bUndo_ /* = false */)
{
	ModAutoPointer<Common::DataArrayData> pIndexTuple =
		pIndex_->makeExpungeKey(pTuple_);

	// 索引ファイルから削除する
	pIndex_->getFile().expunge(pIndexTuple);
#ifdef DEBUG
	SydSchemaDebugMessage << "SystemFile::expunge(Index) = " << pIndexTuple->toString() << ModEndl;
#endif
}

//
//	FUNCTION public
//	Schema::SystemTable::SystemFile::storeObject --
//		スキーマオブジェクトをファイルに格納する
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Object& cObject_
//			ファイルに格納するスキーマオブジェクト
//		Schema::Object::Status::Value eStatus_
//			オブジェクトの永続化状態
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
storeObject(const Schema::Object& cObject_,
			Schema::Object::Status::Value eStatus_,
			Trans::Transaction& cTrans_)
{
	// ファイルに書き込む
	// Statusの値によって処理が異なる

	// オブジェクトの内容をDataArrayDataにする

	ModAutoPointer<Common::DataArrayData> pData = cObject_.pack();

	switch (eStatus_) {
	case Schema::Object::Status::Created:
	case Schema::Object::Status::Mounted:
	case Schema::Object::Status::DeleteCanceled:
	{
		insert(cTrans_, pData.get());
		// ファイル上のオブジェクトIDを設定する
		Common::Data* pElement = pData->getElement(0).get();
		; _SYDNEY_ASSERT(pElement
						 && pElement->getType() == LogicalFile::ObjectID().getType());

		LogicalFile::ObjectID* pOID =
			_SYDNEY_DYNAMIC_CAST(LogicalFile::ObjectID*, pElement);
		; _SYDNEY_ASSERT(pOID);
		const_cast<Schema::Object&>(cObject_).setFileObjectID(*pOID);
		break;
	}
	case Schema::Object::Status::Changed:
	case Schema::Object::Status::Deleted:
	case Schema::Object::Status::DeletedInRecovery:
	{
		// 修正や削除の対象を特定するキーとしてファイル上のオブジェクトIDを得る

		const Common::Data* pOID = cObject_.getFileObjectID();

		Common::DataArrayData cKey;
		cKey.setElement(0, Common::Data::Pointer(pOID));

		switch (eStatus_) {
		case Schema::Object::Status::Changed:
			update(cTrans_, &cKey, pData.get());	break;
		case Schema::Object::Status::Deleted:
		case Schema::Object::Status::DeletedInRecovery:
			expunge(cTrans_, &cKey, pData.get());	break;
		}
		break;
	}
	default:
		; _SYDNEY_ASSERT(false);
		break;
	}

	// 書き込みが成功したデータは永続化された状態にする
	// 削除をReallyDeletedにするのは呼び出し側でキャッシュから削除した後に行う

	cObject_.setStatus(Schema::Object::Status::Persistent);
	; _SYDNEY_ASSERT(cObject_.getFileObjectID());
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::addIndex --
//		システム表の検索に使う索引ファイルを追加する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::SystemTable::IndexFile* pIndex_
//			追加する索引ファイル
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
addIndex(IndexFile* pIndex_)
{
	if (!m_pIndex) {
		m_pIndex = new ModVector<IndexFile*>();
		m_pIndex->reserve(_iMinimumIndexNumber);	// 最低でも2つは必要
	}
	m_pIndex->pushBack(pIndex_);
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::getIndex --
//		システム表の検索に使う索引ファイルのうち指定した名前をもつものを得る
//
//	NOTES
//
//	ARGUMENTS
//		const char* pszName_
//			この名前をもつ索引ファイルを得る
//
//	RETURN
//		Schema::SystemTable::IndexFile*
//			指定された名前をもつ索引ファイル
//
//	EXCEPTIONS

SystemTable::IndexFile*
SystemTable::SystemFile::
getIndex(const char* pszName_)
{
	if (m_pIndex) {
		ModSize n = m_pIndex->getSize();
		for (ModSize i = 0; i < n; i++)
			if ((*m_pIndex)[i]->getName() == pszName_)
				return (*m_pIndex)[i];
	}

	return 0;
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::verify --
//		システム表の整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Verification::Progress& cResult_
//			検査結果を格納する変数
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		unsigned int		eTreatment_
//			検査の結果見つかった障害に対する対応を表し、
//			Admin::Verification::Treatment::Value の論理和を指定する
//		Schema::Object::ID::Value& iMaxID_
//			検査中に取得したスキーマオブジェクトIDの最大値を記録する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
verify(Admin::Verification::Progress& cResult_,
	   Trans::Transaction& cTrans_,
	   Admin::Verification::Treatment::Value eTreatment_,
	   Schema::Object::ID::Value& iMaxID_)
{
	const ModUnicodeString cstrPath;	// スキーマでProgressに入れるパスは空文字列

	bool bCascade = (eTreatment_ & Admin::Verification::Treatment::Cascade);
	bool bCorrect = (eTreatment_ & Admin::Verification::Treatment::Correct);
	bool bContinue = (eTreatment_ & Admin::Verification::Treatment::Continue);

	// 呼び出し側で検査の経過が良好であることを保証する必要がある
	; _SYDNEY_ASSERT(cResult_.isGood());

	// 論理ファイル操作の排他制御のためにラッチする

	AutoLatch	latch(cTrans_, *this);

	// ファイルを初期化する
	initialize();
	; _SYDNEY_ASSERT(m_pRecord);

	// 索引の数を調べておく
	ModSize n = m_pIndex ? m_pIndex->getSize() : 0;

	if (bCascade) {
		// 論理ファイルの整合性検査を実行する
		m_pRecord->getFile().verify(cTrans_, eTreatment_, cResult_);
		if (!cResult_.isGood()) {
			return;
		}

		// 中断のポーリング
		Manager::checkCanceled(cTrans_);

		// 索引ファイルの整合性検査を実行する
		for (ModSize i = 0; i < n; i++) {
			IndexFile* pIndex = (*m_pIndex)[i];

			pIndex->getFile().verify(cTrans_, eTreatment_, cResult_);
			if (!cResult_.isGood()) {
				return;
			}

			// 中断のポーリング
			Manager::checkCanceled(cTrans_);
		}
	}

	//-----------------------
	// レコードファイルと索引ファイルの
	// 整合性を調べる
	//-----------------------

	// レコードに格納されている件数を調べる
	ModInt64 iRecordCount = m_pRecord->getFile().getCount(cTrans_);

	if (m_pIndex) {
		// 索引ファイルの件数が一致しているか調べる
		for (ModSize i = 0; i < n; i++) {
			IndexFile* pIndex = (*m_pIndex)[i];

			if (pIndex->getFile().getCount(cTrans_) != iRecordCount) {
				// 件数が食い違っている
				// ★注意★
				// 索引ファイルを作り直すことで修復可能であるが、
				// 整合性検査中に他のクライアントがアクセスする可能性があるので
				// ファイル全体に影響する修復はできない

				_SYDNEY_VERIFY_INCONSISTENT(cResult_, cstrPath,
											Message::MetaIndexNotMatch(ModUnicodeString(pIndex->getName())));

				// Continue指定がない限りここで終える
				if (!bContinue) {
					return;
				}
			}
		}
	}

	// レコードはScanモードでオープンする
	LogicalFile::OpenOption cRecordOpenOption;
	if (!m_pRecord->getFile().getSearchParameter(0, cRecordOpenOption)) {
		// レコードがScanモードでオープンできないはずがない
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	m_pRecord->getFile().open(cTrans_, cRecordOpenOption);

	// 索引はFetchモードでオープンする
	ModSize i = 0;
	for (; i < n; i++) {
		IndexFile* pIndex = (*m_pIndex)[i];

		LogicalFile::OpenOption cIndexOpenOption;

		// Fetchの条件を作る
		if (pIndex->getCategory() == IndexFile::Category::Btree) {
			// B+木はキーとバリュー
			TreeNode::Field cKey(pIndex->getKeyPosition());
			TreeNode::Field cValue(pIndex->getValuePosition());
			TreeNode::Pair cPair(cKey, cValue);
			TreeNode::Fetch cFetch(cPair);

			if (!pIndex->getFile().getSearchParameter(&cFetch, cIndexOpenOption)) {
				// 失敗することはありえない
				_SYDNEY_THROW0(Exception::Unexpected);
			}

		} else {
			// Vectorはキー
			TreeNode::List cFetchFields;
			cFetchFields.addNode(new TreeNode::Field(pIndex->getKeyPosition()));
			TreeNode::Fetch cFetch(cFetchFields);

			if (!pIndex->getFile().getSearchParameter(&cFetch, cIndexOpenOption)) {
				// 失敗することはありえない
				_SYDNEY_THROW0(Exception::Unexpected);
			}
		}

		// Projectionでキーとバリューだけを得る
		TreeNode::List cFields;
		cFields.addNode(new TreeNode::Field(pIndex->getKeyPosition()));
		cFields.addNode(new TreeNode::Field(pIndex->getValuePosition()));

		if (!pIndex->getFile().getProjectionParameter(&cFields, cIndexOpenOption)) {
			// 失敗することはありえない
			_SYDNEY_THROW0(Exception::Unexpected);
		}

		pIndex->getFile().open(cTrans_, cIndexOpenOption);
	}

	// レコードを尽きるまで読む
	Common::DataArrayData& cData = m_pRecord->getTupleData();
	while (m_pRecord->getFile().getData(&cData)) {

		// IDの値を得る
		// ★注意★
		// すべてのスキーマオブジェクトでIDの位置は固定(i=1)

		Common::Data* pElement = cData.getElement(1).get();
		if (!pElement
			|| pElement->getType() != Common::DataType::UnsignedInteger) {
			_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
		}
		Common::UnsignedIntegerData* pID =
			_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData*, pElement);
		; _SYDNEY_ASSERT(pID);
		if (iMaxID_ == Schema::Object::ID::Invalid
			|| iMaxID_ < pID->getValue()) {
			iMaxID_ = pID->getValue();
		}

		// 索引ごとにキーになっている値に対応するデータがあるか調べる
		for (i = 0; i < n; i++) {
			IndexFile* pIndex = (*m_pIndex)[i];
			Common::Data::Pointer pKey = cData.getElement(pIndex->getRecordKeyPosition());
			Common::Data::Pointer pValue = cData.getElement(pIndex->getRecordValuePosition());

			Common::DataArrayData cFetchKey;
			if (pIndex->getCategory() == IndexFile::Category::Btree) {
				// B+木はキーとバリュー
				cFetchKey.reserve(2);
				cFetchKey.setElement(0, pKey);
				cFetchKey.setElement(1, pValue);

			} else {
				// ベクターはキー
				cFetchKey.setElement(0, pKey);
			}
			pIndex->getFile().fetch(&cFetchKey);

			bool bFound = false;
			Common::DataArrayData& cIndexData = pIndex->getTupleData(true /* include key */);
			; _SYDNEY_ASSERT(cIndexData.getCount() == 2);

			while (pIndex->getFile().getData(&cIndexData)) {
				if (cIndexData.getElement(0)->equals(pKey.get())
					&& cIndexData.getElement(1)->equals(pValue.get())) {
					bFound = true;
					break;
				}
			}
			if (!bFound) {
				// 合致しないデータがあった

				_SYDNEY_VERIFY_INCONSISTENT(cResult_, cstrPath,
											Message::MetaIndexTupleNotFound(ModUnicodeString(pIndex->getName()),
																			pKey->getString()));

				if (!bContinue) {
					// Continue指定でなければここで終わる
					goto End;
				}
			}
		}

		// 中断のポーリング
		Manager::checkCanceled(cTrans_);
	}

  End:
	m_pRecord->getFile().close();
	for (i = 0; i < n; i++) {
		IndexFile* pIndex = (*m_pIndex)[i];
		pIndex->getFile().close();
	}
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::setFileID --
//		システム表のレコードファイルに対応するFileIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		FileID
//
//	EXCEPTIONS

const LogicalFile::FileID&
SystemTable::SystemFile::
getFileID()
{
	initializeRecord(getAreaPath());

	return m_pRecord->getFileID();
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::getAreaPath --
//		システム表の保管場所であるファイルのパス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイルのパス名
//
//	EXCEPTIONS
//		なし

const Os::Path&
SystemTable::SystemFile::
getAreaPath() const
{
	return m_cPath;
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::getRecord --
//		システム表の保管場所であるファイルのうちレコードファイルを表す
//		クラスのオブジェクトを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイルのオブジェクト
//
//	EXCEPTIONS
//		なし

SystemTable::RecordFile*
SystemTable::SystemFile::
getRecord()
{
	return m_pRecord;
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::getCategory -- システム表種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ファイル種別
//
//	EXCEPTIONS
//		なし

Schema::Object::Category::Value
SystemTable::SystemFile::
getCategory() const
{
	return m_eCategory;
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::propagateDatabaseAttribute
//		-- データベースの属性変更に対応する操作を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
SystemTable::SystemFile::
propagateDatabaseAttribute(Trans::Transaction& cTrans_,
						   const Schema::Database::Attribute& cAttribute_)
{
	// 現在はデータベースの読み書き属性が変更された場合にのみ呼ばれる
	// データベースに属するシステム表は論理ファイルが保持している内部構造を
	// 破棄するために一度unmountする
	unmount(cTrans_);

	// 厳密にはm_eAttrの内容も変更するべきだがこの関数が呼ばれるのは
	// auto変数のSystemFileからなので変更しても使用されないので変更しない
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::getTableID --
//		システム表に対応する表IDを得る
//
//	NOTES
//		select文で使用できるSystem_TableなどのIDと一致する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		システム表に対応する表ID
//
//	EXCEPTIONS

Schema::Object::ID::Value
SystemTable::SystemFile::
getTableID() const
{
	return Schema::Table::getSystemTableID(getCategory());
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::getID --
//		システム表に対応するファイルオブジェクトIDを得る
//
//	NOTES
//		select文で使用できるSystem_Tableなどを構成するファイルのIDは
//		計算で得られないため表IDと同じ値にする
//		ロック名に用いられるのでファイル間で異なっていれば問題ないはず
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		システム表に対応するファイルオブジェクトID
//
//	EXCEPTIONS

Schema::Object::ID::Value
SystemTable::SystemFile::
getID() const
{
	return getTableID();
}

//	FUNCTION protected
//	Schema::SystemTable::SystemFile::setLockName --
//		システムファイルのFileIDにシステム表のロック名情報を設定する
//
//	NOTES
//
//	ARGUMENTS
//		LogicalFile::FileID&	cFileID_
//			ロック名情報を設定するファイルID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::SystemFile::
setLockName(LogicalFile::FileID& cFileID_) const
{
	// ファイルのロック名取得に関するオプション
	cFileID_.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
						FileCommon::FileOption::DatabaseID::Key),
						getDatabaseID());
	cFileID_.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
						FileCommon::FileOption::TableID::Key),
						getTableID());
	cFileID_.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
						FileCommon::FileOption::FileObjectID::Key),
					    getID());
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
