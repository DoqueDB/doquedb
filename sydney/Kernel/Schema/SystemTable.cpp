// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SystemTable.cpp -- システム表関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2009, 2012, 2023 Ricoh Company, Ltd.
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

#include "Schema/Area.h"
#include "Schema/AreaMap.h"
#include "Schema/AreaContent.h"
#include "Schema/AreaContentMap.h"
#include "Schema/AutoLatch.h"
#include "Schema/Cascade.h"
#include "Schema/CascadeMap.h"
#include "Schema/Column.h"
#include "Schema/ColumnMap.h"
#include "Schema/Constraint.h"
#include "Schema/ConstraintMap.h"
#include "Schema/Database.h"
#include "Schema/DatabaseMap.h"
#include "Schema/Field.h"
#include "Schema/FieldMap.h"
#include "Schema/File.h"
#include "Schema/FileMap.h"
#include "Schema/Function.h"
#include "Schema/FunctionMap.h"
#include "Schema/Hold.h"
#include "Schema/Index.h"
#include "Schema/IndexMap.h"
#include "Schema/Key.h"
#include "Schema/KeyMap.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/Meta.h"
#include "Schema/Object.h"
#include "Schema/ObjectSnapshot.h"
#include "Schema/ObjectTemplate.h"
#include "Schema/Parameter.h"
#include "Schema/Partition.h"
#include "Schema/PartitionMap.h"
#include "Schema/PathParts.h"
#include "Schema/Privilege.h"
#include "Schema/PrivilegeMap.h"
#include "Schema/Recovery.h"
#include "Schema/Sequence.h"
#include "Schema/SystemFile.h"
#include "Schema/SystemTable.h"
#include "Schema/SystemTable_Database.h"
#include "Schema/SystemTable_Table.h"
#include "Schema/SystemTable_Area.h"
#include "Schema/SystemTable_AreaContent.h"
#include "Schema/SystemTable_Cascade.h"
#include "Schema/SystemTable_Column.h"
#include "Schema/SystemTable_Constraint.h"
#include "Schema/SystemTable_Index.h"
#include "Schema/SystemTable_Key.h"
#include "Schema/SystemTable_File.h"
#include "Schema/SystemTable_Field.h"
#include "Schema/SystemTable_Function.h"
#include "Schema/SystemTable_Partition.h"
#include "Schema/SystemTable_Privilege.h"
#include "Schema/Table.h"
#include "Schema/TableMap.h"
#include "Schema/TreeNode.h"
#include "Schema/Utility.h"
#ifdef DEBUG
#include "Schema/Debug.h"
#endif

#include "Checkpoint/Database.h"

#include "Common/Assert.h"
#include "Common/AutoCaller.h"
#include "Common/DataArrayData.h"
#include "Common/Hasher.h"
#include "Common/IntegerArrayData.h"
#include "Common/Message.h"
#include "Common/UnicodeString.h"

#include "Exception/MetaDatabaseCorrupted.h"

#include "LogicalFile/FileID.h"
#include "LogicalFile/ObjectID.h"

#include "FileCommon/FileOption.h"
#include "FileCommon/OpenOption.h"

#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"

#include "Trans/Transaction.h"

#include "ModAutoPointer.h"
#include "ModError.h"
#include "ModException.h"
#include "ModHashMap.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace {

////////////////////////////////////////
// エリアのパスに追加する文字列(索引) //
////////////////////////////////////////

const char* const _pszParentIDIndex	= "ParentID";
const char* const _pszIDIndex		= "ID";
const char* const _pszAreaIDIndex	= "AreaID";
const char* const _pszObjectIDIndex	= "ObjectID";

//////////////////////////////////////////
// システム表の永続化状態を管理する配列 //
//////////////////////////////////////////

Os::CriticalSection	_cCriticalSection;			// このモジュール内での
												// 排他制御を行うための
												// クリティカルセクション

typedef ModPair<int, ModUInt32> _SystemTableStatus;
typedef ModVector<_SystemTableStatus> _StatusTable;
typedef ModHashMap<Object::ID::Value, _StatusTable*, ModHasher<Object::ID::Value> > _StatusTableMap;
_StatusTableMap* _pStatusTables = 0;

// StatusTableMapのコンストラクターに与えるパラメーター
ModSize _statusTableMapSize = 3;
ModBoolean _statusTableMapEnableLink = ModFalse; // Iterationしない

namespace _Status
{
	// データベースIDに対応する永続化状態配列を得る
	_StatusTable* _getStatusTable(Schema::Object::ID::Value iID_);
	// データベースIDに対応する永続化状態配列を削除する
	void _eraseStatusTable(Schema::Object::ID::Value iID_);
	// 状態が削除されることを表すかを得る
	bool _isObjectToBeDeleted(Schema::Object::Status::Value eStatus_);
}

} // namespace

//////////////////////////
//	$$
//////////////////////////

//	FUNCTION local
//	$$::_Status::_getStatusTable -- データベースIDに対応する永続化状態配列を得る
//
//	NOTES

_StatusTable*
_Status::_getStatusTable(Schema::Object::ID::Value iID_)
{
	// 呼び出し側で排他制御するのでここではロックは不要

	if (!_pStatusTables)
		_pStatusTables = new _StatusTableMap(_statusTableMapSize, _statusTableMapEnableLink);
	else {
		_StatusTableMap::Iterator iterator = _pStatusTables->find(iID_);
		if (iterator != _pStatusTables->end())
			return _StatusTableMap::getValue(iterator);
	}

	// ここに来るのは対応する状態配列がまだないとき

	ModAutoPointer<_StatusTable> pNewTable =
		new _StatusTable(static_cast<ModSize>(Schema::Object::Category::ValueNum), _SystemTableStatus(0, 0));
	_pStatusTables->insert(iID_, pNewTable.release());

	return pNewTable.get();
}

//	FUNCTION local
//	$$::_Status::_eraseStatusTable -- データベースIDに対応する永続化状態配列を削除する
//
//	NOTES

void
_Status::_eraseStatusTable(Schema::Object::ID::Value iID_)
{
	// 呼び出し側で排他制御するのでここではロックは不要

	if (_pStatusTables) {
		_StatusTableMap::Iterator iterator = _pStatusTables->find(iID_);
		if (iterator != _pStatusTables->end()) {
			_StatusTable* p = _StatusTableMap::getValue(iterator);
			delete p;
			_pStatusTables->erase(iterator);
		}
	}
}

// 状態が削除されることを表すかを得る
inline
bool
_Status::_isObjectToBeDeleted(Schema::Object::Status::Value eStatus_)
{
	return (eStatus_ == Schema::Object::Status::Deleted
			|| eStatus_ == Schema::Object::Status::DeletedInRecovery);
}

//////////////////////////
//	Schema::SystemTable	//
//////////////////////////

//	FUNCTION public
//	Schema::SystemTable::initialize -- システム表に関する初期化
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
//		なし

void
SystemTable::
initialize()
{ }

//	FUNCTION public
//	Schema::SystemTable::terminate -- システム表に関する後処理
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
//		なし

void
SystemTable::
terminate()
{
	Os::AutoCriticalSection m(_cCriticalSection);

	if (_pStatusTables) {
		while (!_pStatusTables->isEmpty()) {
			_StatusTable* pEntry = _pStatusTables->getFront();
			delete pEntry;
			_pStatusTables->popFront();
		}
	}
	delete _pStatusTables, _pStatusTables = 0;
}

//	FUNCTION public
//	Schema::SystemTable::setStatus --
//		システム表の永続化に関する状態を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			対象のオブジェクトが属するデータベースID
//		Schema::Object::Category::Value eCategory_
//			対象のオブジェクトの種類
//		Schema::SystemTable::Status::Value eStatus_
//			設定する状態
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
SystemTable::
setStatus(Schema::Object::ID::Value iID_,
		  Schema::Object::Category::Value eCategory_,
		  SystemTable::Status::Value eStatus_)
{
	Os::AutoCriticalSection m(_cCriticalSection);

	_StatusTable* pEntry = _Status::_getStatusTable(iID_);
	; _SYDNEY_ASSERT(pEntry);

	_SystemTableStatus& cStatus = (*pEntry)[static_cast<ModSize>(eCategory_)];
	cStatus.first |= eStatus_;
	++cStatus.second;
}

//	FUNCTION public
//	Schema::SystemTable::unsetStatus --
//		システム表の永続化に関する状態を解除する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			対象のオブジェクトが属するデータベースID
//		Schema::Object::Category::Value eCategory_
//			対象のオブジェクトの種類
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
SystemTable::
unsetStatus(Schema::Object::ID::Value iID_, Schema::Object::Category::Value eCategory_)
{
	Os::AutoCriticalSection m(_cCriticalSection);

	_StatusTable* pEntry = _Status::_getStatusTable(iID_);
	; _SYDNEY_ASSERT(pEntry);

	_SystemTableStatus& cStatus = (*pEntry)[static_cast<ModSize>(eCategory_)];
	; _SYDNEY_ASSERT(cStatus.second > 0);
	if (--cStatus.second == 0)
		cStatus.first = Status::Clean;
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::getStatus --
//		システム表の永続化に関する状態を得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			対象のオブジェクトが属するデータベースID
//		Schema::Object::Category::Value eCategory_
//			対象のオブジェクトの種類
//
//	RETURN
//		指定されたオブジェクトの永続化状態
//
//	EXCEPTIONS
//		なし

// static
SystemTable::Status::Value
SystemTable::
getStatus(Schema::Object::ID::Value iID_, Schema::Object::Category::Value eCategory_)
{
	Os::AutoCriticalSection m(_cCriticalSection);

	_StatusTable* pEntry = _Status::_getStatusTable(iID_);
	; _SYDNEY_ASSERT(pEntry);

	return static_cast<Status::Value>((*pEntry)[static_cast<ModSize>(eCategory_)].first);
}

//	FUNCTION public
//	Schema::SystemTable::eraseStatus --
//		システム表の永続化に関する状態を保持する構造を破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			対象のデータベースID
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
SystemTable::
eraseStatus(Schema::Object::ID::Value iID_)
{
	Os::AutoCriticalSection m(_cCriticalSection);

	_Status::_eraseStatusTable(iID_);
}

//	FUNCTION public
//	Schema::SystemTable::SystemFile::getSystemFile --
//		カテゴリーを指定してシステム表のオブジェクトを得る
//
//	NOTES
//		内部でnewしているので呼び出し側で解放する必要がある
//
//	ARGUMENTS
//		Schema::Object::Category::Value eCategory_
//			対象のオブジェクトの種類
//		Schema::Database* pDatabase_
//			対象のオブジェクトが属するデータベースオブジェクト
//
//	RETURN
//		指定されたカテゴリーのシステム表
//
//	EXCEPTIONS
//		なし

// static
SystemTable::SystemFile*
SystemTable::
getSystemFile(Schema::Object::Category::Value eCategory_,
			  Schema::Database* pDatabase_)
{
	; _SYDNEY_ASSERT(eCategory_ == Schema::Object::Category::Database
					 || pDatabase_);

	switch (eCategory_) {
	case Schema::Object::Category::Area:
	{
		return new Area(*pDatabase_);
	}
	case Schema::Object::Category::AreaContent:
	{
		return new AreaContent(*pDatabase_);
	}
	case Schema::Object::Category::Database:
	{
		return new Database();
	}
	case Schema::Object::Category::Table:
	{
		return new Table(*pDatabase_);
	}
	case Schema::Object::Category::Column:
	{
		return new Column(*pDatabase_);
	}
	case Schema::Object::Category::Constraint:
	{
		return new Constraint(*pDatabase_);
	}
	case Schema::Object::Category::Index:
	{
		return new Index(*pDatabase_);
	}
	case Schema::Object::Category::Key:
	{
		return new Key(*pDatabase_);
	}
	case Schema::Object::Category::File:
	{
		return new File(*pDatabase_);
	}
	case Schema::Object::Category::Field:
	{
		return new Field(*pDatabase_);
	}
	case Schema::Object::Category::Cascade:
	{
		return new Cascade(*pDatabase_);
	}
	case Schema::Object::Category::Partition:
	{
		return new Partition(*pDatabase_);
	}
	case Schema::Object::Category::Function:
	{
		return new Function(*pDatabase_);
	}
	case Schema::Object::Category::Privilege:
	{
		return new Privilege(*pDatabase_);
	}
	default:
		; _SYDNEY_ASSERT(false);
		break;
	}
	return 0;
}

//	FUNCTION
//	Schema::SystemTable::setAvailability --
//		メタデータベースが利用可能かを設定する
//
//	NOTES
//
//	ARGUMENTS
//		bool				v
//			true
//				利用可能にする
//			false
//				利用不可にする
//
//	RETURN
//		true
//			設定前は利用可能だった
//		false
//			設定前は利用不可だった
//
//	EXCEPTIONS

bool
SystemTable::
setAvailability(bool v)
{
	return Checkpoint::Database::setAvailability(ObjectID::SystemTable, v);
}

//	FUNCTION
//	Schema::SystemTable::isAvailable -- メタデータベースが利用可能か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			利用可能である
//		false
//			利用不可である
//
//	EXCEPTIONS

bool
SystemTable::
isAvailable()
{
	return Checkpoint::Database::isAvailable(ObjectID::SystemTable);
}

//////////////////////////////////
//	Schema::SystemTable::Base	//
//////////////////////////////////

//	TEMPLATE FUNCTION public
//	Schema::SystemTable::Base::Base --
//		システム表を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Os::Path& cPathBase_
//			システム表を格納するパスのベースになる文字列
//		const Os::Path& cPathPart_
//			ベースになる文字列に追加してシステム表のパス名になる文字列
//		SystemTable::Attribute::Value eAttr_
//			システム表の属性
//		Schema::Database* pDatabase_
//			オブジェクトが属するデータベース
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class _Object_, class _Pointer_, class _Parent_>
SystemTable::Base<_Object_, _Pointer_, _Parent_>::
Base(Schema::Object::Category::Value eCategory_,
	 const Os::Path& cPathBase_, const Os::Path& cPathPart_,
	 SystemTable::Attribute::Value eAttr_, Schema::Database* pDatabase_)
	: SystemFile(eCategory_, cPathBase_, cPathPart_, eAttr_)
	, m_pDatabase(pDatabase_)
{
}

//	TEMPLATE FUNCTION public
//	Schema::SystemTable::Base::load --
//		システム表から読み込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const _Parent_& cParent_
//			読み込むオブジェクトが属する親オブジェクト
//		Schema::SystemTable::IndexFile* pIndexFile_ = 0
//			0以外のとき、この索引を使って検索する(条件は親オブジェクトのID)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class _Object_, class _Pointer_, class _Parent_>
void
SystemTable::Base<_Object_, _Pointer_, _Parent_>::
load(Trans::Transaction& cTrans_, _Parent_& cParent_, IndexFile* pIndexFile_ /* = 0 */)
{
	// システム表を記録してある
	// ファイルがオープンされていなければ、オープンする

	if (pIndexFile_)
		open(cTrans_, pIndexFile_, cParent_.getID());
	else
		open(cTrans_, OpenMode::Read);

	// 正常終了でもエラーでもcloseする
	Common::AutoCaller1<SystemTable::Base<_Object_, _Pointer_, _Parent_>, Trans::Transaction&>
		closer(this, &SystemTable::Base<_Object_, _Pointer_, _Parent_>::close, cTrans_);

	// 読み出すのに使用するDataArrayDataを用意する
	Common::DataArrayData& cData = getRecord()->getTupleData();

	// データが尽きるまでスキーマ情報を読み出していく
	while (true) {
		_Pointer_ pObject;
		{
			// ラッチする（store側と合わせること）
			AutoLatch latch(cTrans_, *this);

			// 表から1件読み出す
			if (!getData(cTrans_, cData))
				break;

			// 読み出したデータから新しいインスタンスを得る
			pObject = _Object_::getNewInstance(cData);
			;_SYDNEY_ASSERT(pObject.get());

			// 読み出したばかりのデータは永続化されたのと同じ
			pObject->setStatus(Schema::Object::Status::Persistent);
			; _SYDNEY_ASSERT(pObject->getFileObjectID());

			if (pIndexFile_) {
				// 親IDを条件とした場合、対応が正しいかを読み込み時に検査する
				if (pObject->getParentID() != cParent_.getID()) {
					_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
				}
			}

			// データベースオブジェクトを設定する
			if (m_pDatabase)
				pObject->setDatabase(m_pDatabase);
		}

		// 読み込んだ後に行う処理を実行する
		// ★注意★
		// この処理はLatchの外で行わないとデッドロックの可能性がある
		// loadを呼ぶ側でロックしているので同じオブジェクトに対して
		// 同時に呼ばれることはないはず
		_Object_::doAfterLoad(pObject, cParent_, cTrans_);
	}

	// AutoCallerにより自動的にcloseされる
}

//	TEMPLATE FUNCTION public
//	Schema::SystemTable::Base::store --
//		システム表に書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		_Parent_& cParent_
//			親オブジェクト
//		const ObjectMap* pMap_
//			書き込むオブジェクトが含まれるマップ
//		bool bContinuously_ = false
//			trueのとき他のstoreから継続した実行である
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class _Object_, class _Pointer_, class _Parent_>
void
SystemTable::Base<_Object_, _Pointer_, _Parent_>::
store(Trans::Transaction& cTrans_, const _Parent_& cParent_,
	  const ObjectMap<_Object_, _Pointer_>* pMap_, bool bContinuously_ /* = false */)
{
	// マップに読み込まれていなければ処理の必要はない
	if (!pMap_)
		return;

	if (!bContinuously_)

		// ファイルがオープンされていなければ、オープンする
		// ★注意★
		// 親が永続なオブジェクトのときのみオープンすることができる

		if (cParent_.getScope() == Schema::Object::Scope::Permanent)
			open(cTrans_, OpenMode::Update);

	// 正常終了でもエラーでもcloseする
	Common::AutoCaller1<SystemTable::Base<_Object_, _Pointer_, _Parent_>, Trans::Transaction&>
		closer(this, &SystemTable::Base<_Object_, _Pointer_, _Parent_>::close, cTrans_);

	if (bContinuously_ || cParent_.getScope() != Schema::Object::Scope::Permanent)
		// ここでopenしていないのでAutoCallerをdisableする
		closer.release();

	// 指定された表に登録されている列を表すクラスごとに
	// その情報をファイルへ書き込んでいく

	// 親が削除されるときはこのオブジェクトをマップからeraseする必要はない
	bool bNeedToErase = (!_Status::_isObjectToBeDeleted(cParent_.getStatus()));

	typename ObjectMap<_Object_, _Pointer_>::ConstIterator iterator = pMap_->begin();
	const typename ObjectMap<_Object_, _Pointer_>::ConstIterator& end = pMap_->end();

	while (iterator != end) {
		_Pointer_ pObject = ObjectMap<_Object_, _Pointer_>::getValue(iterator++);
												// 削除される可能性があるので
												// ここで次へ進める
		if (pObject.get())
			store(cTrans_, pObject, true /* continuously */, bNeedToErase);
	}
}

//	TEMPLATE FUNCTION public
//	Schema::SystemTable::Base::store --
//		システム表に書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const _Pointer_& pObject_
//			書き込むオブジェクト
//		bool bContinuously_ = false
//			trueのとき他のstoreから継続した実行である
//		bool bNeedToErase_ = true
//			trueのときオブジェクトをマップから削除する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class _Object_, class _Pointer_, class _Parent_>
void
SystemTable::Base<_Object_, _Pointer_, _Parent_>::
store(Trans::Transaction& cTrans_, const _Pointer_& pObject_, bool bContinuously_ /* = false */, bool bNeedToErase_ /* = true */)
{
	// ラッチする（load 側と対応させること）
	AutoLatch latch(cTrans_, *this);

	// 永続化前に行う処理
	Schema::Object::Status::Value eStatus = pObject_->getStatus();
	_Object_::doBeforePersist(pObject_, eStatus, bNeedToErase_, cTrans_);

	if (pObject_->getScope() == Schema::Object::Scope::Permanent) {
		if ((!m_pDatabase
			 || !_Status::_isObjectToBeDeleted(m_pDatabase->getStatus()))
			&& eStatus != Schema::Object::Status::Persistent
			&& eStatus != Schema::Object::Status::CreateCanceled
			&& eStatus != Schema::Object::Status::ReallyDeleted) {

			// 永続的なオブジェクトかつ永続化されていないので、書き込む

			if (!bContinuously_)
				// ファイルがオープンされていなければ、オープンする
				open(cTrans_, OpenMode::Update);

			// 正常終了でもエラーでもcloseする
			Common::AutoCaller1<SystemTable::Base<_Object_, _Pointer_, _Parent_>, Trans::Transaction&>
				closer(this, &SystemTable::Base<_Object_, _Pointer_, _Parent_>::close, cTrans_);

			if (bContinuously_)
				// ここでopenしていないのでAutoCallerをdisableする
				closer.release();

			// オブジェクトをファイルに書き込む
			// Statusの更新も同時に行われる
			storeObject(*pObject_, eStatus, cTrans_);
		}
	} else {
		// 状態を「永続」にする
		pObject_->setStatus(Schema::Object::Status::Persistent);
	}

	// 永続化後に行う処理

	_Object_::doAfterPersist(pObject_, eStatus, bNeedToErase_, cTrans_);
}

template <class _Object_, class _Pointer_, class _Parent_>
void
SystemTable::Base<_Object_, _Pointer_, _Parent_>::
store(Trans::Transaction& cTrans_, Schema::Object::ID::Value iID_,
	  const ObjectMap<_Object_, _Pointer_>* pMap_,
	  bool bContinuously_ /* = false */,
	  bool bNeedToErase_ /* = true */)
{
	// マップに読み込まれているときのみ処理する
	if (pMap_) {
		// マップからポインターを得て処理する
		_Pointer_ pObject = pMap_->get(iID_);
		if (pObject.get())
			store(cTrans_, pObject, bContinuously_, bNeedToErase_);
	}
}

//	TEMPLATE FUNCTION protected
//	Schema::SystemTable::Base::setFieldInfo --
//		レコードファイルのFileIDにシステム表のフィールド情報を設定する
//
//	NOTES
//
//	ARGUMENTS
//		LogicalFile::FileID&	cFileID_
//			フィールド情報を設定するファイルID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// virtual
template <class _Object_, class _Pointer_, class _Parent_>
void
SystemTable::Base<_Object_, _Pointer_, _Parent_>::
setFieldInfo(LogicalFile::FileID& cFileID_) const
{
	_Object_().setFieldInfo(cFileID_);
}

//////////////////////////////////
//	Schema::SystemTable::Area	//
//////////////////////////////////

//	FUNCTION public
//	Schema::SystemTable::Area::Area --
//		「エリア」表を表すクラスのコンストラクター
//
//	NOTES
//		親オブジェクトから引く索引は使わない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

SystemTable::Area::
Area(Schema::Database& cDatabase_)
	: Base<Schema::Area, Schema::Area::Pointer, Schema::Database>(
			Schema::Object::Category::Area,
			cDatabase_.getPath(Schema::Database::Path::Category::System),
			Os::Path(PathParts::SystemTable::Schema)
						.addPart(PathParts::SystemTable::Area),
			(cDatabase_.isReadOnly() ? SystemTable::Attribute::ReadOnly : SystemTable::Attribute::Normal),
			&cDatabase_)
{ }

//	FUNCTION public
//	Schema::SystemTable::Area::load --
//		「エリア」表から、既存のエリアの情報をすべて読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Area::
load(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading area("
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << ")..."
		<< ModEndl;
#endif

	// データベースに登録されている
	// エリアを表すクラスをすべて破棄し、
	// 登録されていないことにする
	m_pDatabase->resetArea();

	// エラーが起きたらキャッシュを解放する
	Common::AutoCaller0<Schema::Database> clearer(m_pDatabase, &Schema::Database::clearArea);

	// デフォルト実装を使う
	Base<Schema::Area, Schema::Area::Pointer, Schema::Database>::load(cTrans_, *m_pDatabase);

	// AutoCallerをdisableする
	clearer.release();

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading area...("
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << ")done"
		<< ModEndl;
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Area::store --
//		「エリア」表へ、あるエリアの情報を書き込みする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Area::Pointer&	pArea_
//			このエリアの情報を「エリア」表へ書き込みする
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Area::
store(Trans::Transaction& cTrans_, const Schema::AreaPointer& pArea_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Area) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing area..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::Area, Schema::Area::Pointer, Schema::Database>::store(cTrans_, pArea_, continuously);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing area...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Area::store --
//		「エリア」表へ、あるエリアの情報を書き込みする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Area&	cArea_
//			このエリアの情報を「エリア」表へ書き込みする
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Area::
store(Trans::Transaction& cTrans_, const Schema::Area& cArea_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Area) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing area..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	; _SYDNEY_ASSERT(cArea_.getStatus() != Schema::Object::Status::Created
					 && cArea_.getStatus() != Schema::Object::Status::Mounted);
	Base<Schema::Area, Schema::Area::Pointer, Schema::Database>::store(cTrans_, cArea_.getID(),
																	   m_pDatabase->_areas, continuously);
#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing area...done"
			<< ModEndl;
	}
#endif
}

//////////////////////////////////////////
//	Schema::SystemTable::AreaContent	//
//////////////////////////////////////////

//	FUNCTION public
//	Schema::SystemTable::AreaContent::AreaContent --
//		「エリア格納関係」表を表すクラスのコンストラクター
//
//	NOTES
//		親オブジェクトから引く索引を使う
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

SystemTable::AreaContent::
AreaContent(Schema::Database& cDatabase_)
	: Base<Schema::AreaContent, Schema::AreaContent::Pointer, Schema::Area>(
			Schema::Object::Category::AreaContent,
			cDatabase_.getPath(Schema::Database::Path::Category::System),
			Os::Path(PathParts::SystemTable::Schema)
						.addPart(PathParts::SystemTable::AreaContent),
			(cDatabase_.isReadOnly() ? SystemTable::Attribute::ReadOnly : SystemTable::Attribute::Normal),
			&cDatabase_)
{
	// AreaContentに追加する索引は
	// ・AreaIDによるもの
	// ・ObjectIDによるもの
	// の2つである

#define _FieldType(_member_) \
	Meta::getFieldType(Schema::AreaContent().getMetaMemberType(_member_))

	addIndex(new IndexFile(_pszAreaIDIndex, IndexFile::Category::Btree,
						   Meta::AreaContent::AreaID, _FieldType(Meta::AreaContent::AreaID),
						   Meta::AreaContent::FileOID, _FieldType(Meta::AreaContent::FileOID)));
	addIndex(new IndexFile(_pszObjectIDIndex, IndexFile::Category::Btree,
						   Meta::AreaContent::ObjectID, _FieldType(Meta::AreaContent::ObjectID),
						   Meta::AreaContent::FileOID, _FieldType(Meta::AreaContent::FileOID)));
#undef _FieldType
}

//	FUNCTION public
//	Schema::SystemTable::AreaContent::load --
//		「エリア格納関係」表から、あるエリアを対象とするエリア格納関係の
//		情報をすべて読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Area& cArea_
//			このエリアを対象とする情報を読み出す
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::AreaContent::
load(Trans::Transaction& cTrans_, Schema::Area& cArea_, bool bRecovery_ /* = false */)
{
	// 「エリア格納関係」表を記録してある
	// ファイルがオープンされていなければ、オープンする
	// エリアIDによる索引を使う

	IndexFile* pIndex = getIndex(_pszAreaIDIndex);
	; _SYDNEY_ASSERT(pIndex);

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading content of area "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "." << cArea_.getName() << "..."
		<< ModEndl;
#endif

	// エリアが保持している格納関係を表すクラスをすべて破棄し、
	// 登録されていないことにする

	cArea_.resetContent();

	// エラーが起きたらキャッシュを解放する
	Common::AutoCaller0<Schema::Area> clearer(&cArea_, &Schema::Area::clearContent);

	// デフォルトの実装を使う
	Base<Schema::AreaContent, Schema::AreaContent::Pointer, Schema::Area>::load(cTrans_, cArea_, pIndex);

	// AutoCallerをdisableする
	clearer.release();

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading content of area "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "." << cArea_.getName() << "...done"
		<< ModEndl;
#endif
}

//	FUNCTION public
//	Schema::SystemTable::AreaContent::store --
//		「エリア格納関係」表へ、既存のエリア格納関係の情報をすべて書き込む
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

void
SystemTable::AreaContent::
store(Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::AreaContent) == Status::Clean)
		return;

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Storing area content..."
		<< ModEndl;
#endif

	// 「エリア格納関係」表を記録してある
	// ファイルがオープンされていなければ、オープンする

	open(cTrans_, OpenMode::Update);

	// 正常終了でもエラーでもcloseする
	Common::AutoCaller1<AreaContent, Trans::Transaction&> closer(this, &AreaContent::close, cTrans_);

	// システムに登録されているすべてのエリアを表すクラスについて、
	// それに関係する格納関係を表すクラスの情報をファイルへ書き込んでいく

	ModVector<Schema::Area*> vecArea = m_pDatabase->getArea(cTrans_);

	ModSize n = vecArea.getSize();
	for (ModSize i = 0; i < n; ++i)
		if (vecArea[i])
			store(cTrans_, *vecArea[i], true);

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Storing area content...done"
		<< ModEndl;
#endif
}

//	FUNCTION public
//	Schema::SystemTable::AreaContent::store --
//		「エリア格納関係」表へ、あるエリアを対象とする
//		エリア格納関係の情報をすべて書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Area&	cArea_
//			このエリアを対象とするエリア格納関係をシステム表に書き込む
//		bool			continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::AreaContent::
store(Trans::Transaction& cTrans_,
	  const Schema::Area& cArea_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::AreaContent) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing area content..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::AreaContent, Schema::AreaContent::Pointer, Schema::Area>::store(cTrans_, cArea_, cArea_._contents, continuously);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing area content...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::AreaContent::store --
//		「エリア格納関係」表へ、あるエリア格納関係の情報を書き込みする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::AreaContent::Pointer&	pContent_
//			このエリア格納関係の情報を「エリア格納関係」表へ書き込みする
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::AreaContent::
store(Trans::Transaction& cTrans_,
	  const Schema::AreaContent::Pointer& pContent_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::AreaContent) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing area content..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::AreaContent, Schema::AreaContent::Pointer, Schema::Area>::store(cTrans_, pContent_, continuously);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing area content...done"
			<< ModEndl;
	}
#endif
}

//////////////////////////////////////
//	Schema::SystemTable::Database	//
//////////////////////////////////////

//	FUNCTION public
//	Schema::SystemTable::Database::Database --
//		「データベース」表を表すクラスのコンストラクター
//
//	NOTES
//		親オブジェクトから引く索引は使わない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

SystemTable::Database::
Database()
	: Base<Schema::Database, Schema::Database::Pointer, Schema::ObjectSnapshot>(
			Schema::Object::Category::Database,
			Os::Path(Manager::Configuration::getSystemAreaPath()),
			Os::Path(PathParts::SystemTable::Schema)
						.addPart(PathParts::SystemTable::Database),
			SystemTable::Attribute::Normal,
			0)
{ }

//	FUNCTION public
//	Schema::SystemTable::Database::load --
//		「データベース」表から、既存のデータベースの情報をすべて読み出す
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

void
SystemTable::Database::
load(Trans::Transaction& cTrans_)
{
	using namespace Manager::RecoveryUtility;

	// loadを実行する場合、スナップショットが指定されていなければならない

	// 「データベース」表を記録してある
	// ファイルがオープンされていなければ、オープンする

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading database..."
		<< ModEndl;
#endif

	open(cTrans_, OpenMode::Read);

	// 正常終了でもエラーでもcloseする
	Common::AutoCaller1<Database, Trans::Transaction&> closer(this, &Database::close, cTrans_);

	// システムに登録されている
	// データベースを表すクラスをすべて破棄し、
	// 登録されていないことにする

	ObjectSnapshot* pSnapshot = Manager::ObjectSnapshot::get(cTrans_);
	; _SYDNEY_ASSERT(pSnapshot);

	pSnapshot->resetDatabase();

	// エラーが起きたらキャッシュを解放する
	Common::AutoCaller0<ObjectSnapshot> clearer(pSnapshot, &ObjectSnapshot::clearDatabase);

	// データが尽きるまでデータベースに関する情報を読み出していく
	Schema::Object::ID::Value iPrevID = Schema::Object::ID::Invalid;

	// 読み出すのに使用するDataArrayDataを用意する
	Common::DataArrayData& cData = getRecord()->getTupleData();

	while (true) {

		// ラッチする（store 側と対応させること）
		AutoLatch latch(cTrans_, *this);

		// 表から1件読み出す
		if (!getData(cTrans_, cData))
			break;

		// 読み出したデータから新しいインスタンスを得る
		Schema::Database::Pointer pDatabase =
			Schema::Database::getNewInstance(cData);
		;_SYDNEY_ASSERT(pDatabase.get());

		// 読み出したばかりのデータは永続化されたのと同じ
		pDatabase->setStatus(Schema::Object::Status::Persistent);
		; _SYDNEY_ASSERT(pDatabase->getFileObjectID());

		// データベース表のタプルに読み込みロックをかける
		Manager::SystemTable::hold(cTrans_,
								   Hold::Target::MetaTuple,
								   Lock::Name::Category::Tuple,
								   Hold::Operation::ReadOnly,
								   pDatabase->getID());

		if (iPrevID != Schema::Object::ID::Invalid) {
			// 前にロックしていたものを外す
			Manager::SystemTable::release(cTrans_,
										  Hold::Target::MetaTuple,
										  Lock::Name::Category::Tuple,
										  Hold::Operation::ReadOnly,
										  iPrevID);
		}
		iPrevID = pDatabase->getID();

		// データベースを設定する
		// データベースが属するデータベースは常にメタデータベース
		pDatabase->setDatabaseID(Schema::Object::ID::SystemTable);

		// 読み込み時に行う処理
		Schema::Database::doAfterLoad(pDatabase, *pSnapshot, cTrans_);
	}

	// AutoCallerをdisableする
	clearer.release();

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading database...done"
		<< ModEndl;
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Database::store --
//		「データベース」表へ、あるデータベースの情報を書き込みする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Database::Pointer&	pDatabase_
//			このデータベースの情報を「データベース」表へ書き込みする
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Database::
store(Trans::Transaction& cTrans_, const Schema::Database::Pointer& pDatabase_,
	  bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Database) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing database..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::Database, Schema::Database::Pointer, Schema::ObjectSnapshot>::store(cTrans_, pDatabase_, continuously);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing database...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Database::store --
//		「データベース」表へ、あるデータベースの情報を書き込みする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Database&	cDatabase_
//			このデータベースの情報を「データベース」表へ書き込みする
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	nRETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Database::
store(Trans::Transaction& cTrans_, const Schema::Database& cDatabase_,
	  bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Database) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing database..."
			<< ModEndl;
	}
#endif

	ObjectSnapshot* pSnapshot = Manager::ObjectSnapshot::get(cTrans_);
	; _SYDNEY_ASSERT(pSnapshot);

	// デフォルトの実装を使う
	; _SYDNEY_ASSERT(cDatabase_.getStatus() != Schema::Object::Status::Created
					 && cDatabase_.getStatus() != Schema::Object::Status::Mounted);
	Base<Schema::Database, Schema::Database::Pointer, Schema::ObjectSnapshot>::store(cTrans_, cDatabase_.getID(),
																					 pSnapshot->m_mapDatabases,
																					 continuously);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing database...done"
			<< ModEndl;
	}
#endif
}

//////////////////////////////////
//	Schema::SystemTable::Table	//
//////////////////////////////////

//	FUNCTION public
//	Schema::SystemTable::Table::Table --
//		「表」表のうち、あるデータベースに関する部分を表すクラスの
//		コンストラクター
//
//	NOTES
//		親オブジェクトから引く索引は使わない
//
//	ARGUMENTS
//		Schema::Database&	database
//			このデータベースについての部分「表」表である
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

SystemTable::Table::
Table(Schema::Database& database)
	: Base<Schema::Table, Schema::Table::Pointer, Schema::Database>(
			Schema::Object::Category::Table,
			database.getPath(Schema::Database::Path::Category::System),
			Os::Path(PathParts::SystemTable::Schema)
						.addPart(PathParts::SystemTable::Table),
			(database.isReadOnly() ? SystemTable::Attribute::ReadOnly : SystemTable::Attribute::Normal),
			&database)
{ }

//	FUNCTION public
//	Schema::SystemTable::Table::load --
//		「表」表から、既存の表の情報をすべて読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Table::
load(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading table of database "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "..."
		<< ModEndl;
#endif

	// システムに登録されている
	// 表を表すクラスをすべて破棄し、
	// 登録されていないことにする

	m_pDatabase->resetTable(*m_pDatabase);

	// エラーが起きたらキャッシュを解放する
	Common::AutoCaller0<Schema::Database> clearer(m_pDatabase, &Schema::Database::clearTable);

	// デフォルトの実装を使う
	Base<Schema::Table, Schema::Table::Pointer, Schema::Database>::load(cTrans_, *m_pDatabase);

	// AutoCallerをdisableする
	clearer.release();

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading table of database "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "...done"
		<< ModEndl;
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Table::store --
//		「表」表へ、ある表の情報を書き込みする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Table::Pointer&		pTable_
//			この表の情報を「表」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//		bool				bNeedToErase_ = true
//			true
//				削除のときに登録からの抹消も行う
//			false または指定されないとき
//				削除のときに登録からの抹消は行わない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Table::
store(Trans::Transaction& cTrans_,
	  const Schema::Table::Pointer& pTable_, bool continuously, bool bNeedToErase_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Table) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing table..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::Table, Schema::Table::Pointer, Schema::Database>::store(cTrans_, pTable_, continuously, bNeedToErase_);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing table...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Table::store --
//		「表」表へ、ある表の情報を書き込みする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Table&		cTable_
//			この表の情報を「表」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//		bool				bNeedToErase_ = true
//			true
//				削除のときに登録からの抹消も行う
//			false または指定されないとき
//				削除のときに登録からの抹消は行わない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Table::
store(Trans::Transaction& cTrans_,
	  const Schema::Table& cTable_, bool continuously, bool bNeedToErase_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Table) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing table..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	; _SYDNEY_ASSERT(cTable_.getStatus() != Schema::Object::Status::Created
					 && cTable_.getStatus() != Schema::Object::Status::Mounted);
	Base<Schema::Table, Schema::Table::Pointer, Schema::Database>::store(cTrans_, cTable_.getID(), m_pDatabase->_tables,
																		 continuously, bNeedToErase_);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing table...done"
			<< ModEndl;
	}
#endif
}

//////////////////////////////////
//	Schema::SystemTable::Column	//
//////////////////////////////////

//	FUNCTION public
//	Schema::SystemTable::Column::Column --
//		「列」表のうち、あるデータベースに関する部分を表すクラスの
//		コンストラクター
//
//	NOTES
//		親オブジェクトから引く索引を使う
//
//	ARGUMENTS
//		Schema::Database&	database
//			このデータベースについての部分「列」表である
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

SystemTable::Column::
Column(Schema::Database& database)
	: Base<Schema::Column, Schema::Column::Pointer, Schema::Table>(
			Schema::Object::Category::Column,
			database.getPath(Schema::Database::Path::Category::System),
			Os::Path(PathParts::SystemTable::Schema)
						.addPart(PathParts::SystemTable::Column),
			(database.isReadOnly() ? SystemTable::Attribute::ReadOnly : SystemTable::Attribute::Normal),
			&database)
{
	// ・ID->ParentID
	// ・ParentID->OID

#define _FieldType(_member_) \
	Meta::getFieldType(Schema::Column().getMetaMemberType(_member_))

	addIndex(new IndexFile(_pszIDIndex, IndexFile::Category::Vector,
						   Meta::Column::ID, _FieldType(Meta::Column::ID),
						   Meta::Column::ParentID, _FieldType(Meta::Column::ParentID)));
	addIndex(new IndexFile(_pszParentIDIndex, IndexFile::Category::Btree,
						   Meta::Column::ParentID, _FieldType(Meta::Column::ParentID),
						   Meta::Column::FileOID, _FieldType(Meta::Column::FileOID)));
#undef _FieldType
}

//	FUNCTION public
//	Schema::SystemTable::Column::load --
//		「列」表から、既存の列の情報をすべて読み出す
//
//	NOTES
//		先に「表」表から、情報を読み出す列が存在する
//		表の情報を Schema::SystemTable::Table::load により
//		読み出している必要がある
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Table& cTable_
//			読み出す列が属する表
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Column::
load(Trans::Transaction& cTrans_, Schema::Table& cTable_, bool bRecovery_ /* = false */)
{
	// あるデータベースに関する「列」表を記録してある
	// ファイルがオープンされていなければ、オープンする

	IndexFile* pIndex = getIndex(_pszParentIDIndex);
	; _SYDNEY_ASSERT(pIndex);

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading column of table "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "." << cTable_.getName() << "..."
		<< ModEndl;
#endif

	// この表に登録されている
	// 列を表すクラスをすべて破棄し、
	// 登録されていないことにする

	cTable_.resetColumn(*m_pDatabase);

	// エラーが起きたらキャッシュを解放する
	Common::AutoCaller1<Schema::Table, Trans::Transaction&> clearer(&cTable_, &Schema::Table::clearColumn, cTrans_);

	// デフォルトの実装を使う
	Base<Schema::Column, Schema::Column::Pointer, Schema::Table>::load(cTrans_, cTable_, pIndex);

	// AutoCallerをdisableする
	clearer.release();

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading column of table "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "." << cTable_.getName() << "...done"
		<< ModEndl;
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Column::load --
//		「列」表から、指定されたオブジェクトIDを持つ列の情報を読み出す
//
//	NOTES
//		同じ表に属するすべての列の情報が同時に読み出される
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Object::ID::Value iID_
//			読み出す列のオブジェクトID
//
//	RETURN
//		0以外
//			指定されたオブジェクトIDを持つ列を表すクラスの先頭アドレス
//		0
//			指定されたオブジェクトIDを持つ列は存在しない
//
//	EXCEPTIONS

Schema::Column*
SystemTable::Column::
load(Trans::Transaction& cTrans_, Schema::Object::ID::Value iID_)
{
	// IDをキーに親IDを取得するためのインデックスファイルを使う
	IndexFile* pIndex = getIndex(_pszIDIndex);
	; _SYDNEY_ASSERT(pIndex);

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading column of ID=" << iID_ << " in database "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "..."
		<< ModEndl;
#endif

	Schema::Object::ID::Value id;
	{
		open(cTrans_, pIndex, iID_);

		// 正常終了でもエラーでもcloseする
		Common::AutoCaller1<Column, Trans::Transaction&> closer(this, &Column::close, cTrans_);

		// IDを得る
		id = loadID(cTrans_);

		// 一度ファイルを閉じる
	}
	
#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading column of ID=" << iID_ << " in database "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "...done"
		<< ModEndl;
#endif

	// 親オブジェクトを取得して改めてオブジェクトを取得する
	if (id != Schema::Object::ID::Invalid) {
		if (Schema::Table* pTable = Schema::Table::get(id, m_pDatabase, cTrans_, true /* internal */))
			return pTable->getColumnByID(iID_, cTrans_);
	}

	return 0;
}

//	FUNCTION public
//	Schema::SystemTable::Column::store --
//		「列」表へ、ある表の既存の列の情報をすべて書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Table&	cTable_
//			この表のすべての列の情報を「列」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Column::
store(Trans::Transaction& cTrans_, Schema::Table& cTable_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Column) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing column..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::Column, Schema::Column::Pointer, Schema::Table>::store(cTrans_, cTable_, cTable_._columns, continuously);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing column...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Column::store --
//		「列」表へ、ある索引がつく既存の列の情報をすべて書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Index&	cIndex_
//			この索引がつくすべての列の情報を「列」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Column::
store(Trans::Transaction& cTrans_, Schema::Index& cIndex_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Column) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing column..."
			<< ModEndl;
	}
#endif

	if (!continuously)

		// あるデータベースに関する「列」表を記録してある
		// ファイルがオープンされていなければ、オープンする
		// ★注意★
		// 永続なオブジェクトのときのみオープンすることができる

		if (cIndex_.getScope() == Schema::Object::Scope::Permanent)
			open(cTrans_, OpenMode::Update);

	// 正常終了でもエラーでもcloseする
	Common::AutoCaller1<Column, Trans::Transaction&> closer(this, &Column::close, cTrans_);

	if (continuously || cIndex_.getScope() != Schema::Object::Scope::Permanent)
		// ここでopenしていないのでdisableする
		closer.release();

	Schema::Table* pTable = cIndex_.getTable(cTrans_);
	; _SYDNEY_ASSERT(pTable);

	// ROWIDの列を書き込む
	Schema::Column* rowid = pTable->getTupleID(cTrans_);
	; _SYDNEY_ASSERT(rowid);

	Base<Schema::Column, Schema::Column::Pointer, Schema::Table>::store(cTrans_, rowid->getID(),
																		pTable->_columns, true /* continuously */);

	// 指定された索引に登録されている列を表すクラスごとに
	// その情報をファイルへ書き込んでいく

	ModVector<Schema::Column*> columns = cIndex_.getColumn(cTrans_);

	ModSize n = columns.getSize();
	for (ModSize i = 0; i < n; ++i)
		if (columns[i]) {
			Base<Schema::Column, Schema::Column::Pointer, Schema::Table>::store(cTrans_, columns[i]->getID(),
																		pTable->_columns, true /* continuously */);
		}

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing column...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Column::store --
//		「列」表へ、ある列の情報を書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Column::Pointer&		pColumn_
//			この列の情報を「列」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//		bool				bNeedToErase_ = true
//			true
//				削除のときに登録からの抹消も行う
//			false または指定されないとき
//				削除のときに登録からの抹消は行わない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Column::
store(Trans::Transaction& cTrans_,
	  const Schema::Column::Pointer& pColumn_, bool continuously, bool bNeedToErase_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Column) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing column..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::Column, Schema::Column::Pointer, Schema::Table>::store(cTrans_, pColumn_, continuously, bNeedToErase_);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing column...done"
			<< ModEndl;
	}
#endif
}

//////////////////////////////////////
//	Schema::SystemTable::Constraint	//
//////////////////////////////////////

//	FUNCTION public
//	Schema::SystemTable::Constraint::Constraint --
//		「制約」表のうち、あるデータベースに関する部分を表すクラスの
//		コンストラクター
//
//	NOTES
//		親オブジェクトから引く索引を使う
//
//	ARGUMENTS
//		Schema::Database&	database
//			このデータベースについての部分「制約」表である
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

SystemTable::Constraint::
Constraint(Schema::Database& database)
	: Base<Schema::Constraint, Schema::Constraint::Pointer, Schema::Table>(
			Schema::Object::Category::Constraint,
			database.getPath(Schema::Database::Path::Category::System),
			Os::Path(PathParts::SystemTable::Schema)
						.addPart(PathParts::SystemTable::Constraint),
			(database.isReadOnly() ? SystemTable::Attribute::ReadOnly : SystemTable::Attribute::Normal),
			&database)
{
	// ・ID->ParentID
	// ・ParentID->OID

#define _FieldType(_member_) \
	Meta::getFieldType(Schema::Constraint().getMetaMemberType(_member_))

	addIndex(new IndexFile(_pszIDIndex, IndexFile::Category::Vector,
						   Meta::Constraint::ID, _FieldType(Meta::Constraint::ID),
						   Meta::Constraint::ParentID, _FieldType(Meta::Constraint::ParentID)));
	addIndex(new IndexFile(_pszParentIDIndex, IndexFile::Category::Btree,
						   Meta::Constraint::ParentID, _FieldType(Meta::Constraint::ParentID),
						   Meta::Constraint::FileOID, _FieldType(Meta::Constraint::FileOID)));
#undef _FieldType
}

//	FUNCTION public
//	Schema::SystemTable::Constraint::load --
//		「制約」表から、既存の制約の情報をすべて読み出す
//
//	NOTES
//		先に「制約」表から、情報を読み出す制約が存在する
//		表の情報を Schema::SystemTable::Table::load により
//		読み出している必要がある
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Table& cTable_
//			読み出す制約が属する表
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Constraint::
load(Trans::Transaction& cTrans_, Schema::Table& cTable_, bool bRecovery_ /* = false */)
{
	// あるデータベースに関する「制約」表を記録してある
	// ファイルがオープンされていなければ、オープンする

	IndexFile* pIndex = getIndex(_pszParentIDIndex);
	; _SYDNEY_ASSERT(pIndex);

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading constraint of table "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "." << cTable_.getName() << "..."
		<< ModEndl;
#endif

	// このデータベースに登録されている
	// 制約を表すクラスをすべて破棄し、
	// 登録されていないことにする

	cTable_.resetConstraint(*m_pDatabase);

	// エラーが起きたらキャッシュを解放する
	Common::AutoCaller1<Schema::Table, Trans::Transaction&> clearer(&cTable_, &Schema::Table::clearConstraint, cTrans_);

	// デフォルトの実装を使う
	Base<Schema::Constraint, Schema::Constraint::Pointer, Schema::Table>::load(cTrans_, cTable_, pIndex);

	// AutoCallerをdisableする
	clearer.release();

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading constraint of table "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "." << cTable_.getName() << "...done"
		<< ModEndl;
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Constraint::load --
//		「制約」表から、指定されたオブジェクトIDを持つ制約の情報を読み出す
//
//	NOTES
//		同じ表に属するすべての制約の情報が同時に読み出される
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Object::ID::Value iID_
//			読み出す制約のオブジェクトID
//
//	RETURN
//		0以外
//			指定されたオブジェクトIDを持つ制約を表すクラスの先頭アドレス
//		0
//			指定されたオブジェクトIDを持つ制約は存在しない
//
//	EXCEPTIONS

Schema::Constraint*
SystemTable::Constraint::
load(Trans::Transaction& cTrans_, Schema::Object::ID::Value iID_)
{
	// IDをキーに親IDを取得するためのインデックスファイルを使う
	IndexFile* pIndex = getIndex(_pszIDIndex);
	; _SYDNEY_ASSERT(pIndex);

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading constraint of ID=" << iID_ << " in database "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "..."
		<< ModEndl;
#endif

	Schema::Object::ID::Value id;
	{
		open(cTrans_, pIndex, iID_);

		// 正常終了でもエラーでもcloseする
		Common::AutoCaller1<Constraint, Trans::Transaction&> closer(this, &Constraint::close, cTrans_);

		// IDを得る
		id = loadID(cTrans_);
		
		// 一度ファイルを閉じる
	}

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading constraint of ID=" << iID_ << " in database "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "...done"
		<< ModEndl;
#endif

	// 親オブジェクトを取得して改めてオブジェクトを取得する
	if (id != Schema::Object::ID::Invalid) {
		if (Schema::Table* pTable = Schema::Table::get(id, m_pDatabase, cTrans_, true /* internal */))
			return pTable->getConstraint(iID_, cTrans_);
	}

	return 0;
}

//	FUNCTION public
//	Schema::SystemTable::Constraint::store --
//		「制約」表へ、ある表の既存の制約の情報をすべて書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Table&		table
//			この表のすべての制約の情報を「制約」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Constraint::
store(Trans::Transaction& cTrans_, Schema::Table& cTable_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Constraint) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing constraint..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::Constraint, Schema::Constraint::Pointer, Schema::Table>::store(cTrans_, cTable_, cTable_._constraints, continuously);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing constraint...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Constraint::store --
//		「制約」表へ、ある制約の情報を書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Constraint::Pointer&	pConstraint_
//			この制約の情報を「制約」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//		bool				bNeedToErase_ = true
//			true
//				削除のときに登録からの抹消も行う
//			false または指定されないとき
//				削除のときに登録からの抹消は行わない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Constraint::
store(Trans::Transaction& cTrans_,
	  const Schema::Constraint::Pointer& pConstraint_, bool continuously, bool bNeedToErase_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Constraint) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing constraint..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::Constraint, Schema::Constraint::Pointer, Schema::Table>::store(cTrans_, pConstraint_, continuously, bNeedToErase_);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing constraint...done"
			<< ModEndl;
	}
#endif
}

//////////////////////////////////
//	Schema::SystemTable::Index	//
//////////////////////////////////

//	FUNCTION public
//	Schema::SystemTable::Index::Index --
//		「索引」表のうち、あるデータベースに関する部分を表すクラスの
//		コンストラクター
//
//	NOTES
//		親オブジェクトから引く索引を使う
//
//	ARGUMENTS
//		Schema::Database&	database
//			このデータベースについての部分「索引」表である
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

SystemTable::Index::
Index(Schema::Database& database)
	: Base<Schema::Index, Schema::Index::Pointer, Schema::Table>(
			Schema::Object::Category::Index,
			database.getPath(Schema::Database::Path::Category::System),
			Os::Path(PathParts::SystemTable::Schema)
						.addPart(PathParts::SystemTable::Index),
			(database.isReadOnly() ? SystemTable::Attribute::ReadOnly : SystemTable::Attribute::Normal),
			&database)
{
	// ・ID->ParentID
	// ・ParentID->OID

#define _FieldType(_member_) \
	Meta::getFieldType(Schema::Index().getMetaMemberType(_member_))

	addIndex(new IndexFile(_pszIDIndex, IndexFile::Category::Vector,
						   Meta::Index::ID, _FieldType(Meta::Index::ID),
						   Meta::Index::ParentID, _FieldType(Meta::Index::ParentID)));
	addIndex(new IndexFile(_pszParentIDIndex, IndexFile::Category::Btree,
						   Meta::Index::ParentID, _FieldType(Meta::Index::ParentID),
						   Meta::Index::FileOID, _FieldType(Meta::Index::FileOID)));
#undef _FieldType
}

//	FUNCTION public
//	Schema::SystemTable::Index::load --
//		「索引」表から、既存の索引の情報をすべて読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Table& cTable_
//			読み出す索引が属する表
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Index::
load(Trans::Transaction& cTrans_, Schema::Table& cTable_, bool bRecovery_ /* = false */)
{
	// あるデータベースに関する「索引」表を記録してある
	// ファイルがオープンされていなければ、オープンする

	IndexFile* pIndex = getIndex(_pszParentIDIndex);
	; _SYDNEY_ASSERT(pIndex);

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading index of table "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "." << cTable_.getName() << "..."
		<< ModEndl;
#endif

	// この表に登録されている
	// 索引を表すクラスをすべて破棄し、
	// 登録されていないことにする

	cTable_.resetIndex(*m_pDatabase);

	// エラーが起きたらキャッシュを解放する
	Common::AutoCaller1<Schema::Table, Trans::Transaction&> clearer(&cTable_, &Schema::Table::clearIndex, cTrans_);

	// デフォルトの実装を使う
	Base<Schema::Index, Schema::Index::Pointer, Schema::Table>::load(cTrans_, cTable_, pIndex);

	// AutoCallerをdisableする
	clearer.release();

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading index of table "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "." << cTable_.getName() << "...done"
		<< ModEndl;
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Index::load --
//		「索引」表から、指定されたオブジェクトIDを持つ索引の情報を読み出す
//
//	NOTES
//		同じ表に属するすべての索引の情報が同時に読み出される
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Object::ID::Value iID_
//			読み出す索引のオブジェクトID
//
//	RETURN
//		0以外
//			指定されたオブジェクトIDを持つ索引を表すクラスの先頭アドレス
//		0
//			指定されたオブジェクトIDを持つ索引は存在しない
//
//	EXCEPTIONS

Schema::Index*
SystemTable::Index::
load(Trans::Transaction& cTrans_, Schema::Object::ID::Value iID_)
{
	// IDをキーに親IDを取得するためのインデックスファイルを使う
	IndexFile* pIndex = getIndex(_pszIDIndex);
	; _SYDNEY_ASSERT(pIndex);

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading index of ID=" << iID_ << " in database "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "..."
		<< ModEndl;
#endif

	Schema::Object::ID::Value id;
	{
		open(cTrans_, pIndex, iID_);

		// 正常終了でもエラーでもcloseする
		Common::AutoCaller1<Index, Trans::Transaction&> closer(this, &Index::close, cTrans_);

		// IDを得る
		id = loadID(cTrans_);

		// 一度ファイルを閉じる
	}

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading index of ID=" << iID_ << " in database "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "...done"
		<< ModEndl;
#endif

	// 親オブジェクトを取得して改めてオブジェクトを取得する
	if (id != Schema::Object::ID::Invalid) {
		if (Schema::Table* pTable = Schema::Table::get(id, m_pDatabase, cTrans_, true /* internal */))
			return pTable->getIndex(iID_, cTrans_);
	}

	return 0;
}

//	FUNCTION public
//	Schema::SystemTable::Index::store --
//		「索引」表へ、ある表の既存の索引の情報をすべて書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Table&		cTable_
//			この表のすべての索引の情報を「索引」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Index::
store(Trans::Transaction& cTrans_, Schema::Table& cTable_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Index) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing index..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::Index, Schema::Index::Pointer, Schema::Table>::store(cTrans_, cTable_, cTable_._indices, continuously);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing index...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Index::store --
//		「索引」表へ、ある索引の情報を書き込みする
//
//	NOTES
//		データベース単位または表単位での永続化からの呼び出しに使用する
//		新規作成された索引の永続化にも使用される
//		すでに存在する索引の永続化はPointerで指定できないことがあるので
//		Index&が引数のstoreを使う
//
//	ARGUMENTS
//		const Schema::Index::Pointer&		pIndex_
//			この索引の情報を「索引」表へ書き込む
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//		bool				bNeedToErase_ = true
//			true
//				削除のときに登録からの抹消も行う
//			false または指定されないとき
//				削除のときに登録からの抹消は行わない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Index::
store(Trans::Transaction& cTrans_,
	  const Schema::Index::Pointer& pIndex_, bool continuously, bool bNeedToErase_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Index) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing index..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::Index, Schema::Index::Pointer, Schema::Table>::store(cTrans_, pIndex_, continuously, bNeedToErase_);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing index...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Index::store --
//		「索引」表へ、ある索引の情報を書き込みする
//
//	NOTES
//		データベース単位または表単位での永続化からの呼び出しに使用する
//		新規作成された索引の永続化にも使用される
//		すでに存在する索引の永続化はPointerで指定できないことがあるので
//		Index&が引数のstoreを使う
//
//	ARGUMENTS
//		const Schema::Index&		cIndex_
//			この索引の情報を「索引」表へ書き込む
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//		bool				bNeedToErase_ = true
//			true
//				削除のときに登録からの抹消も行う
//			false または指定されないとき
//				削除のときに登録からの抹消は行わない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Index::
store(Trans::Transaction& cTrans_,
	  const Schema::Index& cIndex_, bool continuously, bool bNeedToErase_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Index) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing index..."
			<< ModEndl;
	}
#endif

	Schema::Table* pTable = cIndex_.getTable(cTrans_);

	// デフォルトの実装を使う
	; _SYDNEY_ASSERT(cIndex_.getStatus() != Schema::Object::Status::Created
					 && cIndex_.getStatus() != Schema::Object::Status::Mounted);
	Base<Schema::Index, Schema::Index::Pointer, Schema::Table>::store(cTrans_, cIndex_.getID(),
																	  pTable->_indices, continuously, bNeedToErase_);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing index...done"
			<< ModEndl;
	}
#endif
}

//////////////////////////////////
//	Schema::SystemTable::Key	//
//////////////////////////////////

//	FUNCTION public
//	Schema::SystemTable::Key::Key --
//		「キー」表のうち、あるデータベースに関する部分を表すクラスの
//		コンストラクター
//
//	NOTES
//		親オブジェクトから引く索引を使う
//
//	ARGUMENTS
//		Schema::Database&	database
//			このデータベースについての部分「キー」表である
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

SystemTable::Key::
Key(Schema::Database& database)
	: Base<Schema::Key, Schema::Key::Pointer, Schema::Index>(
			Schema::Object::Category::Key,
			database.getPath(Schema::Database::Path::Category::System),
			Os::Path(PathParts::SystemTable::Schema)
						.addPart(PathParts::SystemTable::Key),
			(database.isReadOnly() ? SystemTable::Attribute::ReadOnly : SystemTable::Attribute::Normal),
			&database)
{
	// ・ID->ParentID
	// ・ParentID->OID

#define _FieldType(_member_) \
	Meta::getFieldType(Schema::Key().getMetaMemberType(_member_))

	addIndex(new IndexFile(_pszIDIndex, IndexFile::Category::Vector,
						   Meta::Key::ID, _FieldType(Meta::Key::ID),
						   Meta::Key::ParentID, _FieldType(Meta::Key::ParentID)));
	addIndex(new IndexFile(_pszParentIDIndex, IndexFile::Category::Btree,
						   Meta::Key::ParentID, _FieldType(Meta::Key::ParentID),
						   Meta::Key::FileOID, _FieldType(Meta::Key::FileOID)));
#undef _FieldType
}

//	FUNCTION public
//	Schema::SystemTable::Key::load --
//		「キー」表から、既存のキーの情報をすべて読み出す
//
//	NOTES
//		先に「キー」表から、情報を読み出すキーが存在する
//		索引の情報を Schema::SystemTable::Index::load により
//		読み出している必要がある
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Index& cIndex_
//			読み出すキーが属する索引
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Key::
load(Trans::Transaction& cTrans_, Schema::Index& cIndex_, bool bRecovery_ /* = false */)
{
	// ある索引に関する「キー」表を記録してある
	// ファイルがオープンされていなければ、オープンする

	IndexFile* pIndex = getIndex(_pszParentIDIndex);
	; _SYDNEY_ASSERT(pIndex);

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading key of index "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "." << cIndex_.getName() << "..."
		<< ModEndl;
#endif

	// このデータベースに登録されている
	// キーを表すクラスをすべて破棄し、
	// 登録されていないことにする

	cIndex_.resetKey(*m_pDatabase);

	// エラーが起きたらキャッシュを解放する
	Common::AutoCaller0<Schema::Index> clearer(&cIndex_, &Schema::Index::clearKey);

	// デフォルトの実装を使う
	Base<Schema::Key, Schema::Key::Pointer, Schema::Index>::load(cTrans_, cIndex_, pIndex);

	// AutoCallerをdisableする
	clearer.release();

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading key of index "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "." << cIndex_.getName() << "...done"
		<< ModEndl;
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Key::load --
//		「キー」表から、指定されたオブジェクトIDを持つキーの情報を読み出す
//
//	NOTES
//		同じ索引に属するすべてのキーの情報が同時に読み出される
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Object::ID::Value iID_
//			読み出すキーのオブジェクトID
//
//	RETURN
//		0以外
//			指定されたオブジェクトIDを持つキーを表すクラスの先頭アドレス
//		0
//			指定されたオブジェクトIDを持つキーは存在しない
//
//	EXCEPTIONS

Schema::Key*
SystemTable::Key::
load(Trans::Transaction& cTrans_, Schema::Object::ID::Value iID_)
{
	// IDをキーに親IDを取得するためのインデックスファイルを使う
	IndexFile* pIndex = getIndex(_pszIDIndex);
	; _SYDNEY_ASSERT(pIndex);

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading key of ID=" << iID_ << " in database "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "..."
		<< ModEndl;
#endif

	Schema::Object::ID::Value id;
	{
		open(cTrans_, pIndex, iID_);

		// 正常終了でもエラーでもcloseする
		Common::AutoCaller1<Key, Trans::Transaction&> closer(this, &Key::close, cTrans_);

		// IDを得る
		id = loadID(cTrans_);

		// 一度ファイルを閉じる
	}

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading key of ID=" << iID_ << " in database "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "...done"
		<< ModEndl;
#endif

	// 親オブジェクトを取得して改めてオブジェクトを取得する
	if (id != Schema::Object::ID::Invalid) {
		if (Schema::Index* pIndex = Schema::Index::get(id, m_pDatabase, cTrans_))
			return pIndex->getKey(iID_, cTrans_);
	}

	return 0;
}

//	FUNCTION public
//	Schema::SystemTable::Key::store --
//		「キー」表へ、ある表につく索引の既存のキーの情報をすべて書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Table&		cTable_
//			この索引のすべてのキーの情報を「キー」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Key::
store(Trans::Transaction& cTrans_, Schema::Table& cTable_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Key) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing key..."
			<< ModEndl;
	}
#endif

	// 読み込んだ索引がひとつもなければ処理の必要はない

	if (!cTable_._indices)
		return;

	if (!continuously)

		// あるデータベースに関する「キー」表を記録してある
		// ファイルがオープンされていなければ、オープンする
		// ★注意★
		// 永続なオブジェクトのときのみオープンすることができる

		if (cTable_.getScope() == Schema::Object::Scope::Permanent)
			open(cTrans_, OpenMode::Update);

	// 正常終了でもエラーでもcloseする
	Common::AutoCaller1<Key, Trans::Transaction&> closer(this, &Key::close, cTrans_);

	if (continuously || cTable_.getScope() != Schema::Object::Scope::Permanent)
		// ここでopenしていないのでdisableする
		closer.release();

	// この表につく索引を表すクラスごとに処理していく

	Schema::IndexMap::Iterator iterator = cTable_._indices->begin();
	const Schema::IndexMap::Iterator& end = cTable_._indices->end();

	for (; iterator != end; ++iterator) {
		const Schema::Index::Pointer& index = Schema::IndexMap::getValue(iterator);
		if (index.get())
			store(cTrans_, *index, true);
	}

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing key...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Key::store --
//		「キー」表へ、ある索引の既存のキーの情報をすべて書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Index&		cIndex_
//			この索引のすべてのキーの情報を「キー」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Key::
store(Trans::Transaction& cTrans_, Schema::Index& cIndex_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Key) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing key..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::Key, Schema::Key::Pointer, Schema::Index>::store(cTrans_, cIndex_, cIndex_._keys, continuously);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing key...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Key::store --
//		「キー」表へ、あるキーの情報を書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Key&		cKey_
//			このキーの情報を「キー」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//		bool				bNeedToErase_ = true
//			true
//				削除のときに登録からの抹消も行う
//			false または指定されないとき
//				削除のときに登録からの抹消は行わない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Key::
store(Trans::Transaction& cTrans_,
	  const Schema::Key::Pointer& pKey_, bool continuously, bool bNeedToErase_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Key) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing key..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::Key, Schema::Key::Pointer, Schema::Index>::store(cTrans_, pKey_, continuously, bNeedToErase_);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing key...done"
			<< ModEndl;
	}
#endif
}

//////////////////////////////////
//	Schema::SystemTable::File	//
//////////////////////////////////

//	FUNCTION public
//	Schema::SystemTable::File::File --
//		「ファイル」表のうち、あるデータベースに関する部分を表すクラスの
//		コンストラクター
//
//	NOTES
//		親オブジェクトから引く索引を使う
//
//	ARGUMENTS
//		Schema::Database&	database
//			このデータベースについての部分「ファイル」表である
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

SystemTable::File::
File(Schema::Database& database)
	: Base<Schema::File, Schema::File::Pointer, Schema::Table>(
			Schema::Object::Category::File,
			database.getPath(Schema::Database::Path::Category::System),
			Os::Path(PathParts::SystemTable::Schema)
						.addPart(PathParts::SystemTable::File),
			(database.isReadOnly() ? SystemTable::Attribute::ReadOnly : SystemTable::Attribute::Normal),
			&database)
{
	// ・ID->ParentID
	// ・ParentID->OID

#define _FieldType(_member_) \
	Meta::getFieldType(Schema::File().getMetaMemberType(_member_))

	addIndex(new IndexFile(_pszIDIndex, IndexFile::Category::Vector,
						   Meta::File::ID, _FieldType(Meta::File::ID),
						   Meta::File::ParentID, _FieldType(Meta::File::ParentID)));
	addIndex(new IndexFile(_pszParentIDIndex, IndexFile::Category::Btree,
						   Meta::File::ParentID, _FieldType(Meta::File::ParentID),
						   Meta::File::FileOID, _FieldType(Meta::File::FileOID)));
#undef _FieldType
}

//	FUNCTION public
//	Schema::SystemTable::File::load --
//		「ファイル」表から、既存のファイルの情報をすべて読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Table& cTable_
//			読み込むファイルが属する表
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::File::
load(Trans::Transaction& cTrans_, Schema::Table& cTable_, bool bRecovery_ /* = false */)
{
	// あるデータベースに関する「ファイル」表を記録してある
	// ファイルがオープンされていなければ、オープンする

	IndexFile* pIndex = getIndex(_pszParentIDIndex);
	; _SYDNEY_ASSERT(pIndex);

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading file of table "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "." << cTable_.getName() << "..."
		<< ModEndl;
#endif

	// この表に登録されている
	// ファイルを表すクラスをすべて破棄し、
	// 登録されていないことにする

	cTable_.resetFile(*m_pDatabase);

	// エラーが起きたらキャッシュを解放する
	Common::AutoCaller1<Schema::Table, Trans::Transaction&> clearer(&cTable_, &Schema::Table::clearFile, cTrans_);

	// デフォルトの実装を使う
	Base<Schema::File, Schema::File::Pointer, Schema::Table>::load(cTrans_, cTable_, pIndex);

	// AutoCallerをdisableする
	clearer.release();

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading file of table "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "." << cTable_.getName() << "...done"
		<< ModEndl;
#endif
}

//	FUNCTION public
//	Schema::SystemTable::File::load --
//		「ファイル」表から、指定されたオブジェクトIDを持つファイルの情報を読み出す
//
//	NOTES
//		同じ表に属するすべてのファイルの情報が同時に読み出される
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			読み出すファイルのオブジェクトID
//
//	RETURN
//		0以外
//			指定されたオブジェクトIDを持つファイルを表すクラスの先頭アドレス
//		0
//			指定されたオブジェクトIDを持つファイルは存在しない
//
//	EXCEPTIONS

Schema::File*
SystemTable::File::
load(Trans::Transaction& cTrans_, Schema::Object::ID::Value iID_)
{
	// IDをキーに親IDを取得するためのインデックスファイルを使う
	IndexFile* pIndex = getIndex(_pszIDIndex);
	; _SYDNEY_ASSERT(pIndex);

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading file of ID=" << iID_ << " in database "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "..."
		<< ModEndl;
#endif

	Schema::Object::ID::Value id;
	{
		open(cTrans_, pIndex, iID_);

		// 正常終了でもエラーでもcloseする
		Common::AutoCaller1<File, Trans::Transaction&> closer(this, &File::close, cTrans_);

		// IDを得る
		id = loadID(cTrans_);

		// ファイルを閉じる
	}

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading file of ID=" << iID_ << " in database "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "...done"
		<< ModEndl;
#endif

	// 親オブジェクトを取得して改めてオブジェクトを取得する
	if (id != Schema::Object::ID::Invalid) {
		if (Schema::Table* pTable = Schema::Table::get(id, m_pDatabase, cTrans_, true /* internal */))
			return pTable->getFile(iID_, cTrans_);
	}

	return 0;
}

//	FUNCTION public
//	Schema::SystemTable::File::store --
//		「ファイル」表へ、既存のファイルの情報をすべて書き込む
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

void
SystemTable::File::
store(Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::File) == Status::Clean)
		return;

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Storing file..."
		<< ModEndl;
#endif

	// 読み込んだ表がひとつもなければ処理の必要はない

	if (!m_pDatabase->_tables)
		return;

	// あるデータベースに関する「ファイル」表を記録してある
	// ファイルがオープンされていなければ、オープンする

	open(cTrans_, OpenMode::Update);

	// 正常終了でもエラーでもcloseする
	Common::AutoCaller1<File, Trans::Transaction&> closer(this, &File::close, cTrans_);

	// このデータベースに登録されている
	// 表を表すクラスごとに処理していく

	Schema::TableMap::Iterator iterator = m_pDatabase->_tables->begin();
	const Schema::TableMap::Iterator& end = m_pDatabase->_tables->end();

	for (;iterator != end; ++iterator) {
		const Schema::Table::Pointer& table = Schema::TableMap::getValue(iterator);
		if (table.get())
			store(cTrans_, *table, true);
	}

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Storing file...done"
		<< ModEndl;
#endif
}

//	FUNCTION public
//	Schema::SystemTable::File::store --
//		「ファイル」表へ、ある表を構成するファイルの情報をすべて書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Table&		table
//			この表を構成するすべてのファイルの情報を「ファイル」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::File::
store(Trans::Transaction& cTrans_, Schema::Table& cTable_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::File) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing file..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::File, Schema::File::Pointer, Schema::Table>::store(cTrans_, cTable_, cTable_._files, continuously);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing file...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::File::store --
//		「ファイル」表へ、ある索引を構成するファイルの情報をすべて書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Index&		index
//			この索引を構成するすべてのファイルの情報を「ファイル」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::File::
store(Trans::Transaction& cTrans_, Schema::Index& index, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::File) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing file..."
			<< ModEndl;
	}
#endif

	// ★注意★
	// 索引を構成するファイルは1つと決まっている

	const Schema::File::Pointer& pFile = index.getFile(cTrans_);
	if (pFile.get()) {
		bool bNeedToErase = _Status::_isObjectToBeDeleted(pFile->getStatus());

		store(cTrans_, pFile, continuously, bNeedToErase);
	}

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing file...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::File::store --
//		「ファイル」表へ、ある制約に対応するファイルの情報をすべて書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Constraint&		cConstraint_
//			この制約に対応するすべてのファイルの情報を「ファイル」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::File::
store(Trans::Transaction& cTrans_, Schema::Constraint& cConstraint_, bool continuously)
{
	Schema::Index* pIndex = cConstraint_.getIndex(cTrans_);
	store(cTrans_, *pIndex, continuously);
}

//	FUNCTION public
//	Schema::SystemTable::File::store --
//		「ファイル」表へ、あるファイルの情報を書き込みする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::File::Pointer&		pFile_
//			このファイルの情報を「ファイル」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//		bool				bNeedToErase_ = true
//			true
//				削除のときに登録からの抹消も行う
//			false または指定されないとき
//				削除のときに登録からの抹消は行わない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::File::
store(Trans::Transaction& cTrans_,
	  const Schema::File::Pointer& pFile_, bool continuously, bool bNeedToErase_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::File) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing file..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::File, Schema::File::Pointer, Schema::Table>::store(cTrans_, pFile_, continuously, bNeedToErase_);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing file...done"
			<< ModEndl;
	}
#endif
}

//////////////////////////////////
//	Schema::SystemTable::Field	//
//////////////////////////////////

//	FUNCTION public
//	Schema::SystemTable::Field::Field --
//		「フィールド」表のうち、あるデータベースに関する部分を表すクラスの
//		コンストラクター
//
//	NOTES
//		親オブジェクトから引く索引を使う
//
//	ARGUMENTS
//		Schema::Database&	database
//			このデータベースについての部分「フィールド」表である
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

SystemTable::Field::
Field(Schema::Database& database)
	: Base<Schema::Field, Schema::Field::Pointer, Schema::File>(
			Schema::Object::Category::Field,
			database.getPath(Schema::Database::Path::Category::System),
			Os::Path(PathParts::SystemTable::Schema)
						.addPart(PathParts::SystemTable::Field),
			(database.isReadOnly() ? SystemTable::Attribute::ReadOnly : SystemTable::Attribute::Normal),
			&database)
{
	// ・ID->ParentID
	// ・ParentID->FileOID

#define _FieldType(_member_) \
	Meta::getFieldType(Schema::Field().getMetaMemberType(_member_))

	addIndex(new IndexFile(_pszIDIndex, IndexFile::Category::Vector,
						   Meta::Field::ID, _FieldType(Meta::Field::ID),
						   Meta::Field::ParentID, _FieldType(Meta::Field::ParentID)));
	addIndex(new IndexFile(_pszParentIDIndex, IndexFile::Category::Btree,
						   Meta::Field::ParentID, _FieldType(Meta::Field::ParentID),
						   Meta::Field::FileOID, _FieldType(Meta::Field::FileOID)));
#undef _FieldType
}

//	FUNCTION public
//	Schema::SystemTable::Field::load --
//		「フィールド」表から、既存のフィールドの情報をすべて読み出す
//
//	NOTES
//		先に「フィールド」表から、情報を読み出すフィールドが存在する
//		ファイルの情報を Schema::SystemTable::File::load により
//		読み出している必要がある
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::File& cFile_
//			読み出すフィールドが属するファイル
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Field::
load(Trans::Transaction& cTrans_, Schema::File& cFile_, bool bRecovery_ /* = false */)
{
	// あるデータベースに関する「フィールド」表を記録してある
	// ファイルがオープンされていなければ、オープンする

	IndexFile* pIndex = getIndex(_pszParentIDIndex);
	; _SYDNEY_ASSERT(pIndex);

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading field of file "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "." << cFile_.getTable(cTrans_)->getName() << "." << cFile_.getName() << "..."
		<< ModEndl;
#endif

	// このデータベースに登録されている
	// フィールドを表すクラスをすべて破棄し、
	// 登録されていないことにする

	cFile_.resetField(*m_pDatabase);

	// エラーが起きたらキャッシュを解放する
	Common::AutoCaller1<Schema::File, Trans::Transaction&> clearer(&cFile_, &Schema::File::clearField, cTrans_);

	// デフォルトの実装を使う
	Base<Schema::Field, Schema::Field::Pointer, Schema::File>::load(cTrans_, cFile_, pIndex);

	// AutoCallerをdisableする
	clearer.release();

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading field of file "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "." << cFile_.getTable(cTrans_)->getName() << "." << cFile_.getName() << "...done"
		<< ModEndl;
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Field::load --
//		「フィールド」表から、指定されたオブジェクトIDを持つ
//		フィールドの情報を読み出す
//
//	NOTES
//		同じファイルに属するすべてのフィールドの情報が同時に読み出される
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Object::ID::Value iID_
//			読み出すフィールドのオブジェクトID
//
//	RETURN
//		0以外
//			指定されたオブジェクトIDを持つフィールドを表すクラスの先頭アドレス
//		0
//			指定されたオブジェクトIDを持つフィールドは存在しない
//
//	EXCEPTIONS

Schema::Field*
SystemTable::Field::
load(Trans::Transaction& cTrans_, Schema::Object::ID::Value iID_)
{
	// IDをキーに親IDを取得するためのインデックスファイルを使う
	IndexFile* pIndex = getIndex(_pszIDIndex);
	; _SYDNEY_ASSERT(pIndex);

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading field of ID=" << iID_ << " in database "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "..."
		<< ModEndl;
#endif

	Schema::Object::ID::Value id;
	{
		open(cTrans_, pIndex, iID_);

		// 正常終了でもエラーでもcloseする
		Common::AutoCaller1<Field, Trans::Transaction&> closer(this, &Field::close, cTrans_);

		// IDを得る
		id = loadID(cTrans_);

		// 一度ファイルを閉じる
	}

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading field of ID=" << iID_ << " in database "
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << "...done"
		<< ModEndl;
#endif

	// 親オブジェクトを取得して改めてオブジェクトを取得する
	if (id != Schema::Object::ID::Invalid) {
		if (Schema::File* pFile = Schema::File::get(id, m_pDatabase, cTrans_))
			return pFile->getFieldByID(iID_, cTrans_);
	}

	return 0;
}

//	FUNCTION public
//	Schema::SystemTable::Field::store --
//		「フィールド」表へ、ある表を構成するフィールドの情報をすべて書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Table&		table
//			この表を構成するファイルに存在する
//			フィールドの情報を「フィールド」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Field::
store(Trans::Transaction& cTrans_, Schema::Table& table, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Field) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing field..."
			<< ModEndl;
	}
#endif

	// 読み込んだファイルがひとつもなければ処理の必要はない

	if (!table._files)
		return;

	if (!continuously)

		// あるデータベースに関する「フィールド」表を記録してある
		// ファイルがオープンされていなければ、オープンする
		// ★注意★
		// 永続なオブジェクトのときのみオープンすることができる

		if (table.getScope() == Schema::Object::Scope::Permanent)
			open(cTrans_, OpenMode::Update);

	// 正常終了でもエラーでもcloseする
	Common::AutoCaller1<Field, Trans::Transaction&> closer(this, &Field::close, cTrans_);

	if (continuously || table.getScope() != Schema::Object::Scope::Permanent)
		// ここでopenしていないのでdisableする
		closer.release();

	// この表を構成するファイルを表すクラスごとに
	// 存在するフィールドの情報をファイルへ書き込んでいく

	Schema::FileMap::Iterator iterator = table._files->begin();
	const Schema::FileMap::Iterator& end = table._files->end();

	for (;iterator != end; ++iterator) {
		const Schema::File::Pointer& file = Schema::FileMap::getValue(iterator);
		if (file.get())
			store(cTrans_, *file, true);
	}

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing field...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Field::store --
//		「フィールド」表へ、ある索引を構成するフィールドの情報をすべて書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Index&		index
//			この索引を構成するファイルに存在する
//			フィールドの情報を「フィールド」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Field::
store(Trans::Transaction& cTrans_, Schema::Index& index, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Field) == Status::Clean)
		return;

	// ★注意★
	// 索引に関するファイルは1つに決まっている

	const Schema::File::Pointer& pFile = index.getFile(cTrans_);

	if (pFile.get())
		store(cTrans_, *pFile, continuously);
}

//	FUNCTION public
//	Schema::SystemTable::Field::store --
//		「フィールド」表へ、ある制約に対応するファイルのフィールドの情報をすべて書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Constraint&		cConstraint_
//			この制約に対応するファイルに存在する
//			フィールドの情報を「フィールド」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Field::
store(Trans::Transaction& cTrans_, Schema::Constraint& cConstraint_, bool continuously)
{
	Schema::Index* pIndex = cConstraint_.getIndex(cTrans_);
	store(cTrans_, *pIndex, continuously);
}

//	FUNCTION public
//	Schema::SystemTable::Field::store --
//		「フィールド」表へ、あるファイルの既存のフィールドの情報を
//		すべて書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::File&		file
//			このファイルのすべてのフィールドの情報を「フィールド」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Field::
store(Trans::Transaction& cTrans_, Schema::File& cFile_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Field) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing field..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::Field, Schema::Field::Pointer, Schema::File>::store(cTrans_, cFile_, cFile_._fields, continuously);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing field...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Field::store --
//		「フィールド」表へ、あるフィールドの情報を書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Field::Pointer&		pField_
//			このフィールドの情報を「フィールド」表へ書き込む
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//		bool				bNeedToErase_ = true
//			true
//				削除のときに登録からの抹消も行う
//			false または指定されないとき
//				削除のときに登録からの抹消は行わない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Field::
store(Trans::Transaction& cTrans_,
	  const Schema::Field::Pointer& pField_, bool continuously, bool bNeedToErase_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Field) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing field..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::Field, Schema::Field::Pointer, Schema::File>::store(cTrans_, pField_, continuously, bNeedToErase_);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing field...done"
			<< ModEndl;
	}
#endif
}

//////////////////////////////////////
//	Schema::SystemTable::Cascade	//
//////////////////////////////////////

//	FUNCTION public
//	Schema::SystemTable::Cascade::Cascade --
//		「子サーバー」表を表すクラスのコンストラクター
//
//	NOTES
//		親オブジェクトから引く索引は使わない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

SystemTable::Cascade::
Cascade(Schema::Database& cDatabase_)
	: Base<Schema::Cascade, Schema::Cascade::Pointer, Schema::Database>(
			Schema::Object::Category::Cascade,
			cDatabase_.getPath(Schema::Database::Path::Category::System),
			Os::Path(PathParts::SystemTable::Schema)
						.addPart(PathParts::SystemTable::Cascade),
			(cDatabase_.isReadOnly() ? SystemTable::Attribute::ReadOnly : SystemTable::Attribute::Normal),
			&cDatabase_)
{ }

//	FUNCTION public
//	Schema::SystemTable::Cascade::load --
//		「子サーバー」表から、既存の子サーバーの情報をすべて読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Cascade::
load(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading cascade("
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << ")..."
		<< ModEndl;
#endif

	// データベースに登録されている
	// 子サーバーを表すクラスをすべて破棄し、
	// 登録されていないことにする
	m_pDatabase->resetCascade();

	// エラーが起きたらキャッシュを解放する
	Common::AutoCaller0<Schema::Database> clearer(m_pDatabase, &Schema::Database::clearCascade);

	// デフォルト実装を使う
	Base<Schema::Cascade, Schema::Cascade::Pointer, Schema::Database>::load(cTrans_, *m_pDatabase);

	// AutoCallerをdisableする
	clearer.release();

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading cascade...("
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << ")done"
		<< ModEndl;
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Cascade::store --
//		「子サーバー」表へ、ある子サーバーの情報を書き込みする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Cascade::Pointer&	pCascade_
//			この子サーバーの情報を「子サーバー」表へ書き込みする
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Cascade::
store(Trans::Transaction& cTrans_, const Schema::CascadePointer& pCascade_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Cascade) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing cascade..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::Cascade, Schema::Cascade::Pointer, Schema::Database>::store(cTrans_, pCascade_, continuously);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing cascade...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Cascade::store --
//		「子サーバー」表へ、ある子サーバーの情報を書き込みする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Cascade&	cCascade_
//			この子サーバーの情報を「子サーバー」表へ書き込みする
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Cascade::
store(Trans::Transaction& cTrans_, const Schema::Cascade& cCascade_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Cascade) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing cascade..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	; _SYDNEY_ASSERT(cCascade_.getStatus() != Schema::Object::Status::Created
					 && cCascade_.getStatus() != Schema::Object::Status::Mounted);
	Base<Schema::Cascade, Schema::Cascade::Pointer, Schema::Database>::store(cTrans_, cCascade_.getID(),
																			 m_pDatabase->_cascades,
																			 continuously);
#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing cascade...done"
			<< ModEndl;
	}
#endif
}

//////////////////////////////////////
//	Schema::SystemTable::Partition	//
//////////////////////////////////////

//	FUNCTION public
//	Schema::SystemTable::Partition::Partition --
//		「ルール」表を表すクラスのコンストラクター
//
//	NOTES
//		親オブジェクトから引く索引は使わない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

SystemTable::Partition::
Partition(Schema::Database& cDatabase_)
	: Base<Schema::Partition, Schema::Partition::Pointer, Schema::Database>(
			Schema::Object::Category::Partition,
			cDatabase_.getPath(Schema::Database::Path::Category::System),
			Os::Path(PathParts::SystemTable::Schema)
						.addPart(PathParts::SystemTable::Partition),
			(cDatabase_.isReadOnly() ? SystemTable::Attribute::ReadOnly : SystemTable::Attribute::Normal),
			&cDatabase_)
{ }

//	FUNCTION public
//	Schema::SystemTable::Partition::load --
//		「ルール」表から、既存のルールの情報をすべて読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Partition::
load(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading partition("
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << ")..."
		<< ModEndl;
#endif

	// データベースに登録されている
	// ルールを表すクラスをすべて破棄し、
	// 登録されていないことにする
	m_pDatabase->resetPartition();

	// エラーが起きたらキャッシュを解放する
	Common::AutoCaller0<Schema::Database> clearer(m_pDatabase, &Schema::Database::clearPartition);

	// デフォルト実装を使う
	Base<Schema::Partition, Schema::Partition::Pointer, Schema::Database>::load(cTrans_, *m_pDatabase);

	// AutoCallerをdisableする
	clearer.release();

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading partition...("
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << ")done"
		<< ModEndl;
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Partition::store --
//		「ルール」表へ、あるルールの情報を書き込みする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Partition::Pointer&	pPartition_
//			このルールの情報を「ルール」表へ書き込みする
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Partition::
store(Trans::Transaction& cTrans_, const Schema::PartitionPointer& pPartition_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Partition) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing partition..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::Partition, Schema::Partition::Pointer, Schema::Database>::store(cTrans_, pPartition_, continuously);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing partition...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Partition::store --
//		「ルール」表へ、あるルールの情報を書き込みする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Partition&	cPartition_
//			このルールの情報を「ルール」表へ書き込みする
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Partition::
store(Trans::Transaction& cTrans_, const Schema::Partition& cPartition_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Partition) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing partition..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	; _SYDNEY_ASSERT(cPartition_.getStatus() != Schema::Object::Status::Created
					 && cPartition_.getStatus() != Schema::Object::Status::Mounted);
	Base<Schema::Partition, Schema::Partition::Pointer, Schema::Database>::store(cTrans_, cPartition_.getID(),
																			 m_pDatabase->_partitions,
																			 continuously);
#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing partition...done"
			<< ModEndl;
	}
#endif
}

//////////////////////////////////////
//	Schema::SystemTable::Function	//
//////////////////////////////////////

//	FUNCTION public
//	Schema::SystemTable::Function::Function --
//		「関数」表を表すクラスのコンストラクター
//
//	NOTES
//		親オブジェクトから引く索引は使わない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

SystemTable::Function::
Function(Schema::Database& cDatabase_)
	: Base<Schema::Function, Schema::Function::Pointer, Schema::Database>(
			Schema::Object::Category::Function,
			cDatabase_.getPath(Schema::Database::Path::Category::System),
			Os::Path(PathParts::SystemTable::Schema)
						.addPart(PathParts::SystemTable::Function),
			(cDatabase_.isReadOnly() ? SystemTable::Attribute::ReadOnly : SystemTable::Attribute::Normal),
			&cDatabase_)
{ }

//	FUNCTION public
//	Schema::SystemTable::Function::load --
//		「関数」表から、既存の関数の情報をすべて読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Function::
load(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading function("
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << ")..."
		<< ModEndl;
#endif

	// データベースに登録されている
	// 関数を表すクラスをすべて破棄し、
	// 登録されていないことにする
	m_pDatabase->resetFunction();

	// エラーが起きたらキャッシュを解放する
	Common::AutoCaller0<Schema::Database> clearer(m_pDatabase, &Schema::Database::clearFunction);

	// デフォルト実装を使う
	Base<Schema::Function, Schema::Function::Pointer, Schema::Database>::load(cTrans_, *m_pDatabase);

	// AutoCallerをdisableする
	clearer.release();

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading function...("
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << ")done"
		<< ModEndl;
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Function::store --
//		「関数」表へ、ある関数の情報を書き込みする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Function::Pointer&	pFunction_
//			この関数の情報を「関数」表へ書き込みする
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Function::
store(Trans::Transaction& cTrans_, const Schema::FunctionPointer& pFunction_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Function) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing function..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	Base<Schema::Function, Schema::Function::Pointer, Schema::Database>::store(cTrans_, pFunction_, continuously);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing function...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Function::store --
//		「関数」表へ、ある関数の情報を書き込みする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Function&	cFunction_
//			この関数の情報を「関数」表へ書き込みする
//		bool				continuously = false
//			true
//				この呼び出しは連続した一連の呼び出しの一部である
//			false または指定されないとき
//				この呼び出しは連続した一連の呼び出しの一部でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemTable::Function::
store(Trans::Transaction& cTrans_, const Schema::Function& cFunction_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Function) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing function..."
			<< ModEndl;
	}
#endif

	// デフォルトの実装を使う
	; _SYDNEY_ASSERT(cFunction_.getStatus() != Schema::Object::Status::Created
					 && cFunction_.getStatus() != Schema::Object::Status::Mounted);
	Base<Schema::Function, Schema::Function::Pointer, Schema::Database>::store(cTrans_, cFunction_.getID(),
																			   m_pDatabase->_functions,
																			   continuously);
#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing function...done"
			<< ModEndl;
	}
#endif
}

//////////////////////////////////////
//	Schema::SystemTable::Privilege	//
//////////////////////////////////////

//	FUNCTION public
//	Schema::SystemTable::Privilege::Privilege --
//		constructor
//
//	NOTES
//
//	ARGUMENTS
//		Nothing
//
//	RETURN
//		Nothing
//
//	EXCEPTIONS

SystemTable::Privilege::
Privilege(Schema::Database& cDatabase_)
	: Super(Schema::Object::Category::Privilege,
			cDatabase_.getPath(Schema::Database::Path::Category::System),
			Os::Path(PathParts::SystemTable::Schema)
						.addPart(PathParts::SystemTable::Privilege),
			(cDatabase_.isReadOnly() ? SystemTable::Attribute::ReadOnly : SystemTable::Attribute::Normal),
			&cDatabase_)
{ }

//	FUNCTION public
//	Schema::SystemTable::Privilege::load --
//		load all the privilege object from system table
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//		bool bRecovery_ = false
//
//	RETURN
//		Nothing
//
//	EXCEPTIONS

void
SystemTable::Privilege::
load(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading privilege("
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << ")..."
		<< ModEndl;
#endif

	// Reset privilege hash map in the database object
	m_pDatabase->resetPrivilege();

	// Auto object to clear cache
	Common::AutoCaller0<Schema::Database> clearer(m_pDatabase, &Schema::Database::clearPrivilege);

	// Use default implementation
	Super::load(cTrans_, *m_pDatabase);

	// Disable the auto object
	clearer.release();

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportSystemTable)
		<< "Loading privilege...("
		<< "[" << m_pDatabase->getSnapshotID() << "]"
		<< m_pDatabase->getName() << ")done"
		<< ModEndl;
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Privilege::store --
//		Store one privilege object to system table
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//		const Schema::Privilege::Pointer&	pPrivilege_
//		bool				continuously = false
//			true
//				This method is called in a series of storing
//			false or ignored
//				This methos is called solely
//
//	RETURN
//		Nothing
//
//	EXCEPTIONS

void
SystemTable::Privilege::
store(Trans::Transaction& cTrans_, const Schema::PrivilegePointer& pPrivilege_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Privilege) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing privilege..."
			<< ModEndl;
	}
#endif

	// Use default implementation
	Super::store(cTrans_, pPrivilege_, continuously);

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing privilege...done"
			<< ModEndl;
	}
#endif
}

//	FUNCTION public
//	Schema::SystemTable::Privilege::store --
//		Store one privilege object to system table (modifications)
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//		Schema::Privilege&	cPrivilege_
//		bool				continuously = false
//			true
//				This method is called in a series of storing
//			false or ignored
//				This methos is called solely
//
//	RETURN
//		Nothing
//
//	EXCEPTIONS

void
SystemTable::Privilege::
store(Trans::Transaction& cTrans_, const Schema::Privilege& cPrivilege_, bool continuously)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (getStatus(getDatabaseID(), Schema::Object::Category::Privilege) == Status::Clean)
		return;

#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing privilege..."
			<< ModEndl;
	}
#endif

	// Use default implementation
	; _SYDNEY_ASSERT(cPrivilege_.getStatus() != Schema::Object::Status::Created
					 && cPrivilege_.getStatus() != Schema::Object::Status::Mounted);
	Super::store(cTrans_, cPrivilege_.getID(), m_pDatabase->_privileges, continuously);
#ifdef DEBUG
	if (!continuously) {
		SydSchemaParameterMessage(Message::ReportSystemTable)
			<< "Storing privilege...done"
			<< ModEndl;
	}
#endif
}

//
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2009, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
