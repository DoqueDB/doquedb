// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectSnapshot.cpp -- オブジェクトスナップショット関連の関数定義
// 
// Copyright (c) 2000,2001,2002, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
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

#include "Schema/ObjectSnapshot.h"
#include "Schema/Area.h"
#include "Schema/AutoRWLock.h"
#include "Schema/Database.h"
#include "Schema/DatabaseMap.h"
#include "Schema/Manager.h"
#include "Schema/NameParts.h"
#include "Schema/Object.h"
#include "Schema/Recovery.h"
#include "Schema/SystemDatabase.h"
#include "Schema/SystemTable_Database.h"

#include "Common/Assert.h"

#include "Os/AutoCriticalSection.h"

#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace {

Os::CriticalSection _latch;

// OpenMapのコンストラクターに与えるパラメーター
const ModSize _openMapSize = 23;
const ModBoolean _openMapEnableLink = ModFalse; // Iterationしない

// IDとSnapshotの関係を保持するマップ
typedef
ModHashMap<ObjectSnapshot::ID, ObjectSnapshot*, ModHasher<ObjectSnapshot::ID> > _SnapshotMap;
_SnapshotMap* _snapshots = 0;

// SnapshotMapのコンストラクターに与えるパラメーター
const ModSize _snapshotMapSize = 3;
const ModBoolean _snapshotMapEnableLink = ModFalse; // Iterationしない

namespace _ID {
	Os::CriticalSection _latch;
	ObjectSnapshot::ID _last = 0;

	// IDを割り当てる
	ObjectSnapshot::ID _assign();
}

namespace _Cache {
	Os::CriticalSection _latch;

	//	STRUCT local
	//	_Cache::_ListElement -- キャッシュのクリアを行うリストのエントリー
	//
	//	NOTES

	struct _ListElement
	{
		ObjectSnapshot::ID	m_iSnapshotID;
		Object::ID::Value	m_iDatabaseID;
		_ListElement*		m_pPrev;
		_ListElement*		m_pNext;

	public:
		_ListElement(ObjectSnapshot::ID id_, Object::ID::Value dbid_)
			: m_iSnapshotID(id_), m_iDatabaseID(dbid_), m_pPrev(0), m_pNext(0)
		{ }
	};

	// キャッシュのクリア候補のリスト
	_ListElement* _listHead = 0;
	_ListElement* _listTail = 0;

	// リスト操作
	void erase(ObjectSnapshot::ID iSnapshotID_, Object::ID::Value iDatabaseID_);
	void erase(_ListElement*& p_);
	void insert(_ListElement* p_, bool bHead_);
}

namespace _Session {

	Os::CriticalSection _latch;

	// セッション->データベースIDのマップ
	typedef ModHashMap<SessionID, Object::ID::Value, ModHasher<SessionID> > _SessionMap;
	const ModSize _sessionMapSize = 23;
	const ModBoolean _sessionMapEnableLink = ModFalse;
	_SessionMap* _sessions = 0;

	// 初期化
	void _initialize();
	// 使用開始セッションを登録する
	void _reserveID(SessionID sessionID);
	// IDを使用しているセッションの登録を消去する
	void _eraseID(SessionID sessionID);
	// セッションが使用するデータベースIDを記録する
	bool _setDatabaseID(SessionID sessionID, Object::ID::Value id);
	// セッションが使用するデータベースIDを取得する
	Object::ID::Value _getDatabaseID(SessionID sessionID);

} // namespace _Session

namespace _Omit
{
	// Undo中に外部に見せるときに除外されるべきデータベースを判定する
	bool _invalid(Database* pDatabase_);

} // namespace _Omit

namespace _Find
{
	// 名前で探すための判定関数
	bool _byName(Database* pDatabase_, const Schema::Object::Name& cName_);
}

}

//////////////////////////////////////////////////////////////////////
// _ID
//////////////////////////////////////////////////////////////////////

// IDを割り当てる
ObjectSnapshot::ID
_ID::_assign()
{
	Os::AutoCriticalSection l(_latch);
	return ++_last;
}

//////////////////////////////////////////////////////////////////////
// _Cache
//////////////////////////////////////////////////////////////////////

// キャッシュクリア候補リストから消去する
void
_Cache::erase(ObjectSnapshot::ID iSnapshotID_, Object::ID::Value iDatabaseID_)
{
	Os::AutoCriticalSection l(_latch);
	_ListElement* p = _listHead;
	for (; p; p = p->m_pNext) {
		if (p->m_iDatabaseID == iDatabaseID_ && p->m_iSnapshotID == iSnapshotID_) {
			if (p == _listHead) _listHead = _listHead->m_pNext;
			if (p == _listTail) _listTail = _listTail->m_pPrev;
			erase(p);
			break;
		}
	}
}

// キャッシュクリア候補リストから消去する
void
_Cache::erase(_ListElement*& p_)
{
	Os::AutoCriticalSection l(_latch);
	if (p_->m_pNext) p_->m_pNext->m_pPrev = p_->m_pPrev;
	if (p_->m_pPrev) p_->m_pPrev->m_pNext = p_->m_pNext;
	delete p_, p_ = 0;
}

// キャッシュクリア候補をリストに挿入する
void
_Cache::insert(_ListElement* p_, bool bHead_)
{
	Os::AutoCriticalSection l(_latch);

	if (bHead_) {
		// 先頭に入れる
		if (_listHead) _listHead->m_pPrev = p_;
		p_->m_pPrev = 0;
		p_->m_pNext = _listHead;
		_listHead = p_;
		if (!_listTail) _listTail = p_;
	} else {
		// 最後尾に入れる
		if (_listTail) _listTail->m_pNext = p_;
		p_->m_pPrev = _listTail;
		p_->m_pNext = 0;
		_listTail = p_;
		if (!_listHead) _listHead = p_;
	}
}

//////////////////////////////////////////////////////////////////////
// _Session
//////////////////////////////////////////////////////////////////////

//	FUNCTION local
//	$$::_Session::_initialize --
//		_Session内の関数のための初期化を行う
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

inline
void
_Session::_initialize()
{
	// 呼び出し側が排他している
	if (!_sessions) {
		_sessions = new _SessionMap(_sessionMapSize, _sessionMapEnableLink);
	}
}

//	FUNCTION local
//	$$::_Session::_reserveID --
//		使用開始セッションを登録する
//
//	NOTES
//		セッションが開始されてから初めてデータベースオブジェクトの取得が要求されるまでの間保持される
//
//	ARGUMENTS
//		Schema::SessionID sessionID
//			使用を開始するセッションのID
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
_Session::_reserveID(SessionID sessionID)
{
	Os::AutoCriticalSection l(_latch);
	_initialize();
	_sessions->insert(sessionID, Object::ID::Invalid, ModTrue /* no check */);
}

//	FUNCTION local
//	$$::_Session::_eraseID --
//		IDを使用しているセッションの登録を消去する
//
//	NOTES
//		セッションが終了するときに使用しているデータベースのIDに関する登録を消去する
//
//	ARGUMENTS
//		Schema::SessionID sessionID
//			登録を消去するセッションのID
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
_Session::_eraseID(SessionID sessionID)
{
	if (sessionID != Server::IllegalSessionID) {

		Os::AutoCriticalSection l(_latch);
		_initialize();
		_sessions->erase(sessionID);
	}
}

//	FUNCTION local
//	$$::_Session::_setDatabaseID --
//		セッションが使用するデータベースIDを登録する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::SessionID sessionID
//			登録するセッションのID
//		Schema::Object::ID::Value id
//			登録するデータベースID
//
//	RETURN
//		true .. 初めて登録された
//		false.. すでに登録されていた
//
//	EXCEPTIONS
//		なし

bool
_Session::_setDatabaseID(SessionID sessionID, Object::ID::Value id)
{
	if (sessionID != Server::IllegalSessionID) {
		Os::AutoCriticalSection l(_latch);

		_initialize();

		_SessionMap::Iterator iterator = _sessions->find(sessionID);

		if (iterator != _sessions->end()) {
			Object::ID::Value& rid = _SessionMap::getValue(iterator);
			if (rid == Object::ID::Invalid) {
				// 初めて設定された
				rid = id;
				return true;
			}
		}
	}
	return false;
}

//	FUNCTION local
//	$$::_Session::_getDatabaseID --
//		セッションが使用するデータベースIDを取得する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::SessionID sessionID
//			登録したセッションのID
//
//	RETURN
//		登録されているデータベースID
//
//	EXCEPTIONS
//		なし

Object::ID::Value
_Session::_getDatabaseID(SessionID sessionID)
{
	if (sessionID != Server::IllegalSessionID) {
		Os::AutoCriticalSection l(_latch);

		_initialize();

		_SessionMap::Iterator iterator = _sessions->find(sessionID);

		if (iterator != _sessions->end())
			return _SessionMap::getValue(iterator);
	}
	return Object::ID::Invalid;
}

//////////////////////////////////////////////////////////////////////
// _Omit
//////////////////////////////////////////////////////////////////////

//	FUNCTION local
//	$$::_Omit::_invalid --
//		データベース一覧から除外するデータベースを判定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database* pDatabase_
//			判定対象のデータベース
//
//	RETURN
//		true .. 除外する
//		false.. 除外しない
//
//	EXCEPTIONS

inline
bool
_Omit::_invalid(Database* pDatabase_)
{
	return !Manager::RecoveryUtility::Undo::isValidDatabase(pDatabase_->getID());
}

//////////////////////////////////////////////////////////////////////
// _Find
//////////////////////////////////////////////////////////////////////

//	FUNCTION local
//	$$::_Find::_byName --
//		名前で検索するための条件式
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database* pDatabase_
//			判定対象のデータベース
//		const Schema::Object::Name& cName_
//			探す名称
//
//	RETURN
//		true .. 一致した
//		false.. 一致しない
//
//	EXCEPTIONS

inline
bool
_Find::_byName(Database* pDatabase_, const Schema::Object::Name& cName_)
{
	return (cName_ == pDatabase_->getName() && !_Omit::_invalid(pDatabase_));
}

//////////////////////////////////////////////////////////////////////
// ObjectSnapshot
//////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	Schema::ObjectSnapshot::initialize -- 初期化処理
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

//static
void
ObjectSnapshot::
initialize()
{
	if (!_snapshots) {
		Os::AutoCriticalSection l(_latch);

		if (!_snapshots) {
			_snapshots = new _SnapshotMap(_snapshotMapSize, _snapshotMapEnableLink);
		}
	}
}

//	FUNCTION public
//	Schema::ObjectSnapshot::termintae -- 終了処理
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

//static
void
ObjectSnapshot::
terminate()
{
	Os::AutoCriticalSection l(_latch);

	delete _snapshots, _snapshots = 0;
}

//	FUNCTION public
//	Schema::ObjectSnapshot::create --
//		オブジェクトスナップショットを新規に作る
//
//	NOTES
//
//	ARGUMENTS
//		bool bIsNoVersion_ = false
//			trueのとき版管理をしない(最新のシステム表であり再構成の対象になる)
//
//	RETURN
//		作成したスナップショットオブジェクト
//
//	EXCEPTIONS

//static
ObjectSnapshot*
ObjectSnapshot::
create(bool bIsNoVersion_ /* = false */)
{
	ObjectSnapshot* result = new ObjectSnapshot(bIsNoVersion_);

	Os::AutoCriticalSection l(_latch);
	; _SYDNEY_ASSERT(_snapshots);
	_snapshots->insert(result->getID(), result);

	return result;
}

//	FUNCTION public
//	Schema::ObjectSnapshot::get --
//		オブジェクトスナップショットを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ObjectSnapshot::ID id_
//			得るスナップショットのID
//
//	RETURN
//		取得したスナップショットオブジェクト
//
//	EXCEPTIONS

//static
ObjectSnapshot*
ObjectSnapshot::
get(ID id_)
{
	Os::AutoCriticalSection l(_latch);
	; _SYDNEY_ASSERT(_snapshots);
	_SnapshotMap::Iterator iterator = _snapshots->find(id_);
	return iterator != _snapshots->end() ? _SnapshotMap::getValue(iterator) : 0;
}

//	FUNCTION public
//	Schema::ObjectSnapshot::erase --
//		オブジェクトスナップショットを消去する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ObjectSnapshot::ID id_
//			消すスナップショットのID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

//static
void
ObjectSnapshot::
erase(ID id_)
{
	Os::AutoCriticalSection l(_latch);
	; _SYDNEY_ASSERT(_snapshots);
	_snapshots->erase(id_);
}

//	FUNCTION public
//	Schema::ObjectSnapshot::reset --
//		オブジェクトスナップショットが保持するすべてのオブジェクトを破棄する
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
ObjectSnapshot::
reset()
{
	AutoRWLock l(m_cRWLock, Os::RWLock::Mode::Write);

	if (m_mapDatabases)
		resetDatabase();
	m_mapOpened.clear();
}

//	FUNCTION public
//	Schema::ObjectSnapshot::clear --
//		オブジェクトスナップショットが保持するすべてのオブジェクトを破棄し、
//		管理用のベクターも破棄する
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
ObjectSnapshot::
clear()
{
	AutoRWLock l(m_cRWLock, Os::RWLock::Mode::Write);

	clearDatabase();
	m_mapOpened.clear();
}

//	FUNCTION public
//	Schema::ObjectSnapshot::reserveDatabase --
//		セッションがデータベースを使用することを予約する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::SessionID iSessionID_
//			セッションID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
ObjectSnapshot::
reserveDatabase(SessionID iSessionID_)
{
	// セッションIDを記録する
	_Session::_reserveID(iSessionID_);
}

//	FUNCTION public
//	Schema::ObjectSnapshot::eraseReservation --
//		セッションが終了時にデータベースの使用予約を解除する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::SessionID iSessionID_
//			セッションID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
ObjectSnapshot::
eraseReservation(SessionID iSessionID_)
{
	// セッションの登録を解除する
	_Session::_eraseID(iSessionID_);
}

//	FUNCTION public
//	Schema::ObjectSnapshot::releaseDatabase --
//		セッションが終了時にデータベースの使用を終了する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::SessionID iSessionID_
//			セッションID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ObjectSnapshot::
releaseDatabase(SessionID iSessionID_)
{
	// 余計にopenされていたデータベースがあったらcloseする
	AutoRWLock l(m_cRWLock);
	if (isDatabaseOpened(iSessionID_)) {
		// IDを得る
		Schema::Object::ID::Value iDatabaseID = _Session::_getDatabaseID(iSessionID_);

		// IDに対応するデータベースオブジェクトがあればcloseする
		if (m_mapDatabases) {

			Database::Pointer pDatabase = m_mapDatabases->get(iDatabaseID);
			if (pDatabase.get())
				pDatabase->close();
		}

		// オープンされているデータベースの登録を除く
		l.convert(Os::RWLock::Mode::Write);
		m_mapOpened.erase(iSessionID_);
	}
}

////////////////////////////////////////
// 以下はObjectTreeに対応するメソッド //
////////////////////////////////////////

//////////////
// Database //
//////////////

//
//	FUNCTION public
//		Schema::ObjectSnapshot::loadDatabase --
//			データベース表を読み込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		const Schema::DatabaseMap&
//			読み込んだデータベース表
//
//	EXCEPTIONS
//		???
//

const DatabaseMap&
ObjectSnapshot::
loadDatabase(Trans::Transaction& cTrans_)
{
	// 念のために初期化しておく
	initialize();

	AutoRWLock l(m_cRWLock);

	if (!m_mapDatabases) {

		l.convert(Os::RWLock::Mode::Write);
		if (!m_mapDatabases) {
			SystemTable::Database().load(cTrans_);

			if (m_bIsNoVersion)
				// メタデータベースが利用可能であることを設定する
				SystemTable::setAvailability(true);
		}
	}
	return *m_mapDatabases;
}

//
//	FUNCTION public
//		Schema::ObjectSnapshot::getDatabase --
//			スナップショットに登録されているすべてのデータベースを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		const ModVector<Database*>&
//			スナップショットに登録されているすべてのデータベース
//
//	EXCEPTIONS
//		???
//

const ModVector<Database*>&
ObjectSnapshot::getDatabase(Trans::Transaction& cTrans_)
{
	// 念のために初期化しておく
	initialize();

	return loadDatabase(cTrans_).getView(m_cRWLock, BoolFunction0<Database>(_Omit::_invalid));
}

//
//	FUNCTION public
//		Schema::ObjectSnapshot::getDatabase --
//			スナップショットに登録されているもののうち
//			指定したIDを持つデータベースを得る
//
//	NOTES
//		一時データベースの可能性も調べるために
//		更新トランザクションにおいてもこのメソッドは呼ばれる
//
//	ARGUMENTS
//		Schema::Object::ID::Value databaseID
//			取得するデータベースのID
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		Database*
//			指定したIDを持つデータベース
//		0
//			指定したIDを持つデータベースは存在しない
//
//	EXCEPTIONS
//		???
//

Database*
ObjectSnapshot::
getDatabase(Schema::Object::ID::Value databaseID, Trans::Transaction& cTrans_)
{
	if (databaseID == Schema::Object::ID::Invalid)
		return 0;

	// メタデータベースのIDならメタデータベースを返す
	if (databaseID == Schema::Object::ID::SystemTable) {
		return SystemDatabase::getInstance(cTrans_);
	}

	// 外部に見せてはいけないデータベースなら存在しないものとして返す
	if (!Manager::RecoveryUtility::Undo::isValidDatabase(databaseID)) {
		return 0;
	}

	// 念のために初期化しておく
	initialize();

	// 「データベース」表を読み込んでいなければ読み込む
	loadDatabase(cTrans_);
	; _SYDNEY_ASSERT(m_mapDatabases);

	AutoRWLock l(m_cRWLock);
	Database* result = m_mapDatabases->get(databaseID).get();

	// セッションに対応するデータベースとして登録する
	if (result
		&& cTrans_.getSessionID() != Server::IllegalSessionID
		&& (_Session::_setDatabaseID(cTrans_.getSessionID(), databaseID)
			|| !isDatabaseOpened(cTrans_.getSessionID()))) {

		// 初めて登録された場合、余計にopenしておく
		// --> releaseDatabaseでcloseされる
		result->open();

		// 余計にopenしたことを覚えておく
		l.convert(Os::RWLock::Mode::Write);

		m_mapOpened.insert(cTrans_.getSessionID(), true, ModTrue /* no check */);
	}

	return result;
}

//
//	FUNCTION public
//		Schema::ObjectSnapshot::getDatabase --
//			スナップショットに登録されているもののうち
//			指定した名前を持つデータベースIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Object::Name& databaseName
//			取得するデータベースの名前
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		ObjectID::Invalid 以外
//			指定した名前を持つデータベースID
//
//		ObjectID::Invalid
//			指定した名前を持つデータベースは存在しない
//
//	EXCEPTIONS
//		???

ObjectID::Value
ObjectSnapshot::
getDatabaseID(const Schema::Object::Name& databaseName,
			  Trans::Transaction& cTrans_)
{
	// メタデータベースならメタデータベースのIDを返す
	if (databaseName == Schema::Object::Name(NameParts::Database::System)) {
		return Schema::Object::ID::SystemTable;
	}

	// 念のために初期化しておく
	initialize();

	// 「データベース」表を読み込んでいなければ読み込む
	loadDatabase(cTrans_);
	; _SYDNEY_ASSERT(m_mapDatabases);

	AutoRWLock l(m_cRWLock);

	Database* pDatabase = static_cast<const DatabaseMap*>(m_mapDatabases)
		->find(BoolFunction1<Database, const Schema::Object::Name&>(_Find::_byName, databaseName));
	return (pDatabase) ? pDatabase->getID() : ObjectID::Invalid;
}

//
//	FUNCTION public
//		Schema::ObjectSnapshot::addDatabase --
//			スナップショットにデータベースを追加する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database& database
//			追加するデータベース
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		Schema::Database&
//			追加したデータベース
//
//	EXCEPTIONS
//		???
//

Database&
ObjectSnapshot::
addDatabase(const Database::Pointer& database, Trans::Transaction& cTrans_)
{
	// 念のために初期化しておく
	initialize();

	// データベース表を読み込む
	loadDatabase(cTrans_);

	AutoRWLock l(m_cRWLock, Os::RWLock::Mode::Write);

	m_mapDatabases->insert(database);

	Manager::ObjectTree::Database::incrementCacheSize();

	// データベースオブジェクトを保持するスナップショットとして
	// 自身を登録する
	database->setSnapshot(this);

/************ すべての表をPreloadするならこれとDatabase.cppのものを生かす
	if (Manager::Configuration::isSchemaPreloaded())

		// すべてのデータベースのスキーマ情報を
		// ここで読み込む
		// 表を読み込めば表以下のスキーマ情報も読み込まれる
		(void) database.loadTable(cTrans_);
*/

	return *database;
}

//
//	FUNCTION public
//		Schema::ObjectSnapshot::eraseDatabase --
//			スナップショットからデータベースを抹消する
//
//	NOTES
//
//	ARGUMENTS
//		const Database* database
//			登録を抹消するデータベース
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???
//

void
ObjectSnapshot::
eraseDatabase(const Database* database)
{
	// 版管理をしているものは再構成時に丸ごと破棄されるので
	// 再構成用のものについてのみ行えばよい

	if (m_bIsNoVersion) {

		// 念のために初期化しておく
		initialize();

		eraseDatabase(database->getID());
	}
}

// FUNCTION public
//	Schema::ObjectSnapshot::eraseDatabase -- 登録の抹消
//
// NOTES
//
// ARGUMENTS
//	Schema::Object::ID::Value iDatabaseID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ObjectSnapshot::
eraseDatabase(Schema::Object::ID::Value iDatabaseID_, SessionID iSessionID_)
{
	AutoRWLock l(m_cRWLock, Os::RWLock::Mode::Write);

	if (m_mapDatabases) {
		Manager::ObjectTree::Database::decrementCacheSize();
		if (_Session::_getDatabaseID(iSessionID_) == iDatabaseID_
			&& isDatabaseOpened(iSessionID_)) {
			// セッションのデータベースが消されるデータベース自身のとき、ここでcloseする
			Database::Pointer pDatabase = m_mapDatabases->get(iDatabaseID_);
			if (pDatabase.get())
				pDatabase->close();
		}
		m_mapDatabases->erase(iDatabaseID_);
	}
}

//
//	FUNCTION public
//		Schema::ObjectSnapshot::resetDatabase --
//			スナップショットからすべてのデータベースを抹消する
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
//		???
//

void
ObjectSnapshot::
resetDatabase()
{
	// 念のために初期化しておく
	initialize();

	AutoRWLock l(m_cRWLock, Os::RWLock::Mode::Write);

	if (!m_mapDatabases) {
		m_mapDatabases = new DatabaseMap;
	} else {
		Manager::ObjectTree::Database::decrementCacheSize(m_mapDatabases->getSize());
		// キャッシュをクリアすれば勝手にdeleteされる
		m_mapDatabases->reset();
	}
}

//
//	FUNCTION public
//		Schema::ObjectSnapshot::clearDatabase --
//			スナップショットからすべてのデータベースを抹消し、
//			管理用のベクターを破棄する
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
//		???
//

void
ObjectSnapshot::
clearDatabase()
{
	// 念のために初期化しておく
	initialize();

	AutoRWLock l(m_cRWLock, Os::RWLock::Mode::Write);

	if (m_mapDatabases) {
		resetDatabase();
		delete m_mapDatabases, m_mapDatabases = 0;
	}
}

// ベクターに保持されているキャッシュをクリアする
void
ObjectSnapshot::
clearDatabaseView()
{
	// 念のために初期化しておく
	initialize();

	AutoRWLock l(m_cRWLock, Os::RWLock::Mode::Write);

	if (m_mapDatabases) {
		m_mapDatabases->clearView();
	}
}

// FUNCTION public
//	Schema::ObjectSnapshot::eraseTable -- 表の削除
//
// NOTES
//
// ARGUMENTS
//	Schema::Object::ID::Value iDatabaseID_
//	Schema::Object::ID::Value iTableID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ObjectSnapshot::
eraseTable(Schema::Object::ID::Value iDatabaseID_,
		   Schema::Object::ID::Value iTableID_)
{
	AutoRWLock l(m_cRWLock, Os::RWLock::Mode::Write);

	if (m_mapDatabases) {
		Database* pDatabase = m_mapDatabases->get(iDatabaseID_).get();
		if (pDatabase) {
			pDatabase->eraseTable(iTableID_);
		}
	}
}

// FUNCTION public
//	Schema::ObjectSnapshot::eraseIndex -- 索引の削除
//
// NOTES
//
// ARGUMENTS
//	Schema::Object::ID::Value iDatabaseID_
//	Schema::Object::ID::Value iTableID_
//	Schema::Object::ID::Value iIndexID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ObjectSnapshot::
eraseIndex(Schema::Object::ID::Value iDatabaseID_,
		   Schema::Object::ID::Value iTableID_,
		   Schema::Object::ID::Value iIndexID_)
{
	AutoRWLock l(m_cRWLock, Os::RWLock::Mode::Write);

	if (m_mapDatabases) {
		Database* pDatabase = m_mapDatabases->get(iDatabaseID_).get();
		if (pDatabase) {
			pDatabase->eraseIndex(iTableID_, iIndexID_);
		}
	}
}

//
//	FUNCTION public
//		Schema::ObjectSnapshot::detachFiles --
//			スナップショットが保持するすべてのデータベースが
//			アタッチしているファイルをすべてでタッチする
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
//		???
//

void
ObjectSnapshot::
detachFiles()
{
	AutoRWLock l(m_cRWLock);

	if (m_mapDatabases)
		m_mapDatabases->apply(ApplyFunction0<Database>(&Database::detachFiles));
}

//	FUNCTION public
//	Schema::ObjectSnapshot::addDelayedClear --
//		キャッシュのクリアを後回しにしたデータベースとして登録する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value iDatabaseID_
//			登録するデータベースのID
//		bool bVolatile_
//			trueのときクリアされやすいように先頭に登録される
//			falseのときクリアされにくいように最後尾に登録される
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ObjectSnapshot::
addDelayedClear(Schema::Object::ID::Value iDatabaseID_, bool bVolatile_)
{
	// クリア候補リストに入れる
	_Cache::insert(new _Cache::_ListElement(getID(), iDatabaseID_), bVolatile_);
}

//	FUNCTION public
//	Schema::ObjectSnapshot::eraseDelayedClear --
//		キャッシュのクリアを後回しにしたデータベースとしての登録から除く
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value iDatabaseID_
//			登録から除くデータベースのID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ObjectSnapshot::
eraseDelayedClear(Schema::Object::ID::Value iDatabaseID_)
{
	// クリア候補リストから除く
	_Cache::erase(getID(), iDatabaseID_);
}

//	FUNCTION public
//	Schema::ObjectSnapshot::clearDatabaseCache --
//		キャッシュのクリアが後回しにされていたデータベースのキャッシュをクリアする
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

//static
void
ObjectSnapshot::
clearDatabaseCache()
{
	if (_Cache::_listHead) {
		ObjectSnapshot::ID snapshotID;
		Schema::Object::ID::Value databaseID;
		_Cache::_ListElement* next = 0;

		do {
			// _Cache::_latchがかかっている期間はリスト以外を操作してはいけない
			{
				Os::AutoCriticalSection m(_Cache::_latch);
				// ラッチの中で再度調べる
				_Cache::_ListElement* p = _Cache::_listHead;
				next = p->m_pNext;
				snapshotID = p->m_iSnapshotID;
				databaseID = p->m_iDatabaseID;

				// IDを取得したらリストから除いてラッチをはずす
				_Cache::erase(p);

				// 先頭を指すポインターを付け替える
				_Cache::_listHead = next;
				if (!next) _Cache::_listTail = 0;
			}

			// リストから得られるスナップショットのデータベースにキャッシュのクリアを実行する
			if (ObjectSnapshot* pSnapshot = ObjectSnapshot::get(snapshotID)) {
				pSnapshot->clearDatabaseCache(databaseID);
				if (!Manager::ObjectTree::Database::checkCacheSize())
					break;					// 消した結果閾値を下回ったら終了
			}
		} while (next);
	}
}

//	FUNCTION public
//	Schema::ObjectSnapshot::clearDatabaseCache --
//		指定されたIDをもつデータベースのキャッシュをクリアする
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value iDatabaseID_
//			キャッシュをクリアするデータベースのID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ObjectSnapshot::
clearDatabaseCache(Schema::Object::ID::Value iDatabaseID_)
{
	AutoRWLock l(m_cRWLock);

	if (Database* pDatabase = m_mapDatabases->get(iDatabaseID_).get())
		// データベースオブジェクト内に保持されている下位オブジェクトを解放する
		pDatabase->abandonCache();
}

//	FUNCTION public
//	Schema::ObjectSnapshot::closeDatabase --
//		データベースをcloseする
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value iDatabaseID_
//			closeするデータベースのID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ObjectSnapshot::
closeDatabase(Schema::Object::ID::Value iDatabaseID_)
{
	AutoRWLock l(m_cRWLock);

	if (Database* pDatabase = m_mapDatabases->get(iDatabaseID_).get())
		// データベースオブジェクトをcloseする
		pDatabase->close();
}

//	FUNCTION private
//	Schema::ObjectSnapshot::isDatabaseOpened -- 指定されたセッションからデータベースが取得されたか
//
//	NOTES
//
//	ARGUMENTS
//		Schema::SessionID iSessionID_
//			調べる対象のセッションID
//
//	RETURN
//		true .. 取得されていた
//		false.. 取得されていない
//
//	EXCEPTIONS

bool
ObjectSnapshot::
isDatabaseOpened(SessionID iSessionID_) const
{
	// 排他制御は呼び出し側が行う

	OpenMap::ConstIterator iterator = m_mapOpened.find(iSessionID_);
	return (iterator != m_mapOpened.end());
}

//	FUNCTION private
//	Schema::ObjectSnapshot::ObjectSnapshot -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		bool bIsNoVersion_
//			trueのとき版管理をしない(最新のシステム表であり再構成の対象になる)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

ObjectSnapshot::
ObjectSnapshot(bool bIsNoVersion_)
	: m_mapDatabases(0), m_bIsNoVersion(bIsNoVersion_),
	  m_cRWLock(), m_uID(_ID::_assign()), m_mapOpened(_openMapSize, _openMapEnableLink)
{ }

//	FUNCTION private
//	Schema::ObjectSnapshot::~ObjectSnapshot -- デストラクタ
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

ObjectSnapshot::
~ObjectSnapshot()
{
	erase(getID());
	clear();
}

//
// Copyright (c) 2000,2001,2002, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
