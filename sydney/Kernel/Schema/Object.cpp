// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Object.cpp -- スキーマオブジェクト関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "SyInclude.h"

#include "Schema/AutoRWLock.h"
#include "Schema/Database.h"
#include "Schema/Manager.h"
#include "Schema/Object.h"
#include "Schema/Message.h"
#include "Schema/Parameter.h"
#include "Schema/SystemTable.h"
#include "Schema/Utility.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerData.h"
#include "Common/Message.h"
#include "Common/NullData.h"
#include "Common/StringData.h"
#include "Common/UnsignedIntegerData.h"

#include "Exception/MetaDatabaseCorrupted.h"

#include "FileCommon/FileOption.h"

#include "LogicalFile/ObjectID.h"
#include "LogicalFile/FileID.h"

#include "ModArchive.h"
#include "ModSerial.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::Object::Object --
//		スキーマオブジェクトを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Category::Value	category
//			指定されたとき
//				生成するスキーマオブジェクトの種別
//			指定されないとき
//				Schema::Object::Category::Unknown が指定されたものとみなす
//		Schema::Object::Scope::Value	scope
//			指定されたとき
//				生成するスキーマオブジェクトのスコープ
//			指定されないとき
//				Schema::Object::Scope::Unknown が指定されたものとみなす
//		Schema::Object::Status::Value eStatus_
//			指定されたとき
//				生成するスキーマオブジェクトの永続化状態
//			指定されないとき
//				Schema::Object::Status::Unknownが指定されたものとみなす
//		Schema::Object::ID::Value	id
//			指定されたとき
//				生成するスキーマオブジェクトのスキーマオブジェクト ID
//			指定されないとき
//				Schema::Object::ID::Invalid が指定されたものとみなす
//		Schema::Object::ID::Value	parent
//			Schema::Object::ID::Invalid 以外の値
//				生成するスキーマオブジェクトの親のスキーマオブジェクト ID
//			指定されないとき、または Schema::Object::ID::Invalid
//				生成するスキーマオブジェクトには親はない
//		Schema::Object::ID::Value	database
//			Schema::Object::ID::Invalid 以外の値
//				生成するスキーマオブジェクトが属するデータベース ID
//			指定されないとき、または Schema::Object::ID::Invalid
//				生成するスキーマオブジェクトが属するデータベースはまだ設定されていない
//		Schema::Object::Name&	name
//			指定されたとき
//				生成するスキーマオブジェクトの名前
//			指定されないとき
//				Schema::Object::Name() が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Object::
Object(Schema::Object::Category::Value category,
	   Schema::Object::Scope::Value scope,
	   Schema::Object::Status::Value eStatus_,
	   Schema::Object::ID::Value id,
	   Schema::Object::ID::Value parent,
	   Schema::Object::ID::Value database,
	   const Schema::Object::Name& name)
	: _id(id),
	  _parent(parent),
	  _name(name),
	  _category(category),
	  _scope(scope),
	  m_iDatabaseID(database),
	  m_pDatabase(0),
	  m_eStatus(eStatus_),
	  m_pFileObjectID(0),
	  m_iTimestamp(0),
	  m_pArchiver(0),
	  m_cRWLock()
{ }

//	FUNCTION public
//	Schema::Object::~Object --
//		スキーマオブジェクトを表すクラスの仮想デストラクター
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

Object::
~Object()
{
	destruct();
}

#ifdef OBSOLETE // Objectの同一性はIDを使って行うのでObjectに対する比較演算子は使用しない
//	FUNCTION public
//	Schema::Object::operator == -- == 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object&		r
//			自分自身と比較するスキーマオブジェクト
//
//	RETURN
//		true
//			自分自身と与えられたスキーマオブジェクトは等しい
//		false
//			自分自身と等しくない
//
//	EXCEPTIONS
//		なし

bool
Object::
operator ==(const Object& r) const
{
	return _id == r._id;
}

//	FUNCTION public
//	Schema::Object::operator != -- != 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object&		r
//			自分自身と比較するスキーマオブジェクト
//
//	RETURN
//		true
//			自分自身と与えられたスキーマオブジェクトは等しくない
//		false
//			自分自身と等しい
//
//	EXCEPTIONS
//		なし

bool
Object::
operator !=(const Object& r) const
{
	return _id != r._id;
}

//	FUNCTION public
//	Schema::Object::hashCode -- ハッシュ値を計算する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたハッシュ値
//
//	EXCEPTIONS
//		なし

ModSize
Object::
hashCode() const
{
	return _id  % ~((ModSize) 0);
}

//	FUNCTION public
//	Schema::Object::isInvalid -- 無効なスキーマオブジェクトか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			自分自身の表すスキーマオブジェクトは無効である
//		false
//			無効でない
//
//	EXCEPTIONS
//		なし

bool
Object::
isInvalid() const
{
	return _id == ID::Invalid || _category == Category::Unknown;
}
#endif

//	FUNCTION public
//	Schema::Object::getTimestamp -- タイムスタンプを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		オブジェクトのタイムスタンプ
//
//	EXCEPTIONS
//		なし

Schema::Object::Timestamp
Object::
getTimestamp() const
{
	return m_iTimestamp;
}

//	FUNCTION public
//	Schema::Object::getID --
//		スキーマオブジェクトのスキーマオブジェクト ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた自分自身のスキーマオブジェクト ID
//
//	EXCEPTIONS
//		なし

Object::ID::Value
Object::
getID() const
{
	return _id;
}

//	FUNCTION protected
//	Schema::Object::setID --
//		スキーマオブジェクトのスキーマオブジェクト ID を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	id
//			設定するスキーマオブジェクト ID
//
//	RETURN
//		設定したスキーマオブジェクト ID
//
//	EXCEPTIONS
//		なし

void
Object::
setID(Object::ID::Value id)
{
	_id = id;
}

//	FUNCTION public
//	Schema::Object::getParentID --
//		スキーマオブジェクトの親のスキーマオブジェクト ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた自分自身の親のスキーマオブジェクト ID
//
//	EXCEPTIONS
//		なし

Object::ID::Value
Object::
getParentID() const
{
	return _parent;
}

//	FUNCTION protected
//	Schema::Object::setParentID --
//		スキーマオブジェクトの親のスキーマオブジェクト ID を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	parent
//			設定するスキーマオブジェクト ID
//
//	RETURN
//		設定したスキーマオブジェクト ID
//
//	EXCEPTIONS
//		なし

void
Object::
setParentID(Object::ID::Value parent)
{
	_parent = parent;
}

//	FUNCTION public
//	Schema::Object::getName -- スキーマオブジェクトの名前を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた自分自身の名前
//
//	EXCEPTIONS
//		なし

const Object::Name&
Object::
getName() const
{
	return _name;
}

//	FUNCTION protected
//	Schema::Object::setName -- スキーマオブジェクトの名前を設定する
//
//	NOTES
//		引数を const Schema::Object::Name& にすると、
//		引数に ModUnicodeString を与えたとき、
//		内部で代入するときの都合 2 回コピーが起きてしまうので、
//		引数は const ModUnicodeString& にする
//
//	ARGUMENTS
//		ModUnicodeString&	name
//			設定する名前
//
//	RETURN
//		設定した名前
//
//	EXCEPTIONS

const Object::Name&
Object::
setName(const ModUnicodeString& name)
{
	return _name = name;
}

//	FUNCTION protected
//	Schema::Object::setName -- スキーマオブジェクトの名前を設定する
//
//	NOTES
//
//	ARGUMENTS
//		ModCharString&		name
//			設定する名前
//			マルチバイト文字が含まれているなら
//			ModOs::Process::setEncodingTypeされたコードで
//			エンコードされている必要がある
//
//	RETURN
//		設定した名前
//
//	EXCEPTIONS

const Object::Name&
Object::
setName(const ModCharString& name)
{
	return _name = Name(name.getString(), ModOs::Process::getEncodingType());
}

//	FUNCTION public
//	Schema::Object::getCategory -- スキーマオブジェクトの種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた自分自身の種別を表す値
//
//	EXCEPTIONS
//		なし

Object::Category::Value
Object::
getCategory() const
{
	return _category;
}

//	FUNCTION protected
//	Schema::Object::setCategory -- スキーマオブジェクトの種別を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Category::Value	category
//			設定する種別を表す値
//
//	RETURN
//		設定した種別を表す値
//
//	EXCEPTIONS
//		なし

Object::Category::Value
Object::
setCategory(Object::Category::Value category)
{
	return _category = category;
}

//	FUNCTION public
//	Schema::Object::getScope -- スキーマオブジェクトのスコープを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた自分自身のスコープを表す値
//
//	EXCEPTIONS
//		なし

Object::Scope::Value
Object::
getScope() const
{
	return _scope;
}

//	FUNCTION protected
//	Schema::Object::setScope -- スキーマオブジェクトのスコープを設定する
//
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Scope::Value	scope
//			設定するスコープを表す値
//
//	RETURN
//		設定したスコープを表す値
//
//	EXCEPTIONS
//		なし

Object::Scope::Value
Object::
setScope(Object::Scope::Value scope)
{
	return _scope = scope;
}

//	FUNCTION public
//	Schema::Object::getStatus -- スキーマオブジェクトの永続化状態を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた自分自身の永続化状態を表す値
//
//	EXCEPTIONS
//		なし

Object::Status::Value
Object::
getStatus() const
{
	return m_eStatus;
}

//	FUNCTION protected
//	Schema::Object::setStatus -- スキーマオブジェクトの永続化状態を設定する
//
//	NOTES
//		constオブジェクトに対しても変更がある
//
//	ARGUMENTS
//		Schema::Object::Status::Value	eStatus_
//			設定する永続化状態を表す値
//
//	RETURN
//		設定した永続化状態を表す値
//
//	EXCEPTIONS
//		なし

Object::Status::Value
Object::
setStatus(Object::Status::Value eStatus_) const
{
	return m_eStatus = eStatus_;
}

//	FUNCTION public
//	Schema::Object::isLessThan -- ソートに使う比較関数
//
//	NOTES
//
//	ARGUMENTS
//		const Object& cOther_
//
//	RETURN
//		true .. thisの方がソート順で先である
//
//	EXCEPTIONS
//		なし

bool
Object::
isLessThan(const Object& cOther_) const
{
	// デフォルトはIDの順に並べる
	return (getID() < cOther_.getID());
}

//	FUNCTION public
//	Schema::Object::getDatabaseID --
//		スキーマオブジェクトが属するデータベース ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたデータベース ID
//
//	EXCEPTIONS
//		なし

Object::ID::Value
Object::
getDatabaseID() const
{
	return m_iDatabaseID;
}

//	FUNCTION protected
//	Schema::Object::setDatabaseID --
//		スキーマオブジェクトが属するデータベース ID を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	iDatabaseID_
//			設定するデータベース ID
//
//	RETURN
//		設定したスキーマオブジェクト ID
//
//	EXCEPTIONS
//		なし

Object::ID::Value
Object::
setDatabaseID(Object::ID::Value iDatabaseID_)
{
	return m_iDatabaseID = iDatabaseID_;
}

//	FUNCTION public
//	Schema::Object::getFileObjectID --
//		オブジェクトをファイル中に格納するときのOIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		const LogicalFile::ObjectID*
//			ファイル中のOID
//
//	EXCEPTIONS

const LogicalFile::ObjectID*
Object::
getFileObjectID() const
{
	return m_pFileObjectID;
}

//	FUNCTION protected
//	Schema::Object::setTimestamp -- タイムスタンプを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Timestamp iTimestamp_
//			セットするタイムスタンプの値
//
//	RETURN
//		セットしたタイムスタンプの値
//
//	EXCEPTIONS
//		なし

Schema::Object::Timestamp
Object::
setTimestamp(Timestamp iTimestamp_)
{
	return m_iTimestamp = iTimestamp_;
}

//	FUNCTION protected
//	Schema::Object::addTimestamp -- タイムスタンプを進める
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		新しいタイムスタンプの値
//
//	EXCEPTIONS
//		なし

Schema::Object::Timestamp
Object::
addTimestamp()
{
	return ++m_iTimestamp;
}

//	FUNCTION public
//	Schema::Object::getRWLock -- 読み書きロックを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		このオブジェクトのメンバー操作の排他制御に使う読み書きロック
//
//	EXCEPTIONS
//		なし

Os::RWLock&
Object::
getRWLock() const
{
	return const_cast<Os::RWLock&>(m_cRWLock);
}

//	FUNCTION public
//	Schema::Object::clear --
//		スキーマオブジェクトを表すクラスのメンバーをすべて初期化する
//
//	NOTES
//		親クラスのメンバーは初期化しない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Schema::Object::clear()
{
	m_pFileObjectID = 0;
	_id = ID::Invalid;
	_parent = ID::Invalid;
	_name.clear();
	_category = Category::Unknown;
	_scope = Scope::Unknown;
	m_eStatus = Status::Unknown;
	m_iTimestamp = 0;
}

//	FUNCTION public
//	Schema::Object::setFileObjectID --
//		オブジェクトをファイル中に格納するときのOIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//		const LogicalFile::ObjectID& cID_
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Object::
setFileObjectID(const LogicalFile::ObjectID& cID_)
{
	if (m_pFileObjectID)
		delete m_pFileObjectID, m_pFileObjectID = 0;

	m_pFileObjectID = new LogicalFile::ObjectID(cID_);
}

//	FUNCTION public
//	Schema::Object::create -- 
//		状態を作成にする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//		ID::Value iID = ID::Invalid
//			Invalid以外が指定された場合、
//			新規に割り振るのではなくSequenceファイルの整合性を取る
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Object::create(Trans::Transaction& cTrans_, ID::Value iID_ /* = ID::Invalid */)
{
	//////////////////////////////
	// 現在の状態に関係なく処理する
	//////////////////////////////

	// 新たなIDをふる、または整合性を取る
	; _SYDNEY_ASSERT(getDatabaseID() == ID::Invalid || getDatabase(cTrans_));
	setID(ID::assign(cTrans_, getDatabase(cTrans_), iID_, ID::SystemTable));

	// 状態を「作成」にする
	setStatus(Status::Created);

	// システム表の状態を変える
	SystemTable::setStatus(getDatabaseID(), getCategory(), SystemTable::Status::Dirty);
}

//	FUNCTION public
//	Schema::Object::touch -- 
//		関係するオブジェクトの変更により自身を変更する
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
Object::
touch()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	switch (getStatus()) {
	case Status::Created:
	case Status::Changed:
	case Status::Deleted:
	case Status::DeletedInRecovery:
	case Status::ReallyDeleted:
	case Status::Mounted:
	case Status::CreateCanceled:
	case Status::DeleteCanceled:
		// 状態を変える必要はない
		break;
	case Status::Persistent:
		// 状態を「変更」にする
		setStatus(Status::Changed);
		; _SYDNEY_ASSERT(getFileObjectID()
						 || getScope() == Scope::SessionTemporary);

		// システム表の状態を変える
		SystemTable::setStatus(getDatabaseID(), getCategory(), SystemTable::Status::Dirty);

		// タイムスタンプを進める
		(void) addTimestamp();
		break;
	default:
		; _SYDNEY_ASSERT(false);
	}
}

//	FUNCTION public
//	Schema::Object::untouch -- 
//		状態を変更なしにする
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
Object::
untouch()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	switch (getStatus()) {
	case Status::Mounted:
	case Status::Changed:
		// 状態をなしにする
		setStatus(Status::Persistent);
		// システム表の状態を変える
		SystemTable::unsetStatus(getDatabaseID(), getCategory());
		break;
	case Status::Created:
	case Status::Deleted:
	case Status::DeletedInRecovery:
	case Status::ReallyDeleted:
	case Status::Persistent:
	case Status::CreateCanceled:
	case Status::DeleteCanceled:
		// 何もしない
		break;
	default:
		; _SYDNEY_ASSERT(false);
	}
}

//	FUNCTION public
//	Schema::Object::drop -- 
//		状態を破棄または削除の取り消しにする
//
//	NOTES
//
//	ARGUMENTS
//		bool bRecovery_ = false
//			リカバリー処理の中でのDROPである
//		bool bNoUnset_ = false
//			削除の取り消し時、unsetStatusをしない(システム表の永続化を行う)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Object::
drop(bool bRecovery_ /* = false */, bool bNoUnset_ /* = false */ )
{
	switch (getStatus()) {
	case Status::Created:
	case Status::Mounted:
	case Status::DeleteCanceled:
	{
		// 状態をなしにする
		if (bNoUnset_)
			setStatus(Status::CreateCanceled);
		else {
			setStatus(Status::Persistent);
			SystemTable::unsetStatus(getDatabaseID(), getCategory());
		}
		break;
	}
	case Status::Persistent:
	case Status::Changed:
	{
		// 状態を削除にする
		setStatus(bRecovery_ ? Status::DeletedInRecovery : Status::Deleted);
		// システム表の状態を変える
		SystemTable::setStatus(getDatabaseID(), getCategory(), SystemTable::Status::DirtyDeleted);
		break;
	}
	case Status::Unknown:			// 重複名オブジェクトが発見されたら Unknown o状態になる
	case Status::Deleted:
	case Status::DeletedInRecovery:
	case Status::ReallyDeleted:
	case Status::CreateCanceled:
		// 何もしない
		break;
	default:
		; _SYDNEY_ASSERT(false);
	}
}

//	FUNCTION public
//	Schema::Object::undoDrop -- 
//		破棄を取り消す
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
Object::
undoDrop()
{
	switch (getStatus()) {
	case Status::Deleted:
	case Status::DeletedInRecovery:
	{
		// 状態をなしにする
		setStatus(Status::Persistent);
		// システム表の状態を変える
		SystemTable::unsetStatus(getDatabaseID(), getCategory());
		break;
	}
	case Status::ReallyDeleted:
	{
		// 状態を「削除取り消し」にする
		setStatus(Status::DeleteCanceled);
		// システム表の状態を変える
		SystemTable::setStatus(getDatabaseID(), getCategory(), SystemTable::Status::Dirty);
		break;
	}
	case Status::Persistent:
	case Status::Created:
	case Status::Changed:
	case Status::Mounted:
	case Status::CreateCanceled:
	case Status::DeleteCanceled:
		// 何もしない
		break;
	default:
		; _SYDNEY_ASSERT(false);
	}
}

//	FUNCTION public
//	Schema::Object::getDatabase -- オブジェクトが存在するデータベースを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたデータベースを格納する領域の先頭アドレス
//		0
//			オブジェクトが存在するデータベースは存在しない
//
//	EXCEPTIONS

Database*
Object::
getDatabase(Trans::Transaction& cTrans_) const
{
	if (!m_pDatabase) {
		AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
		// 書き込みロックの中でもう一度調べる
		if (!m_pDatabase) {
			if (getScope() == Scope::SessionTemporary) {
				m_pDatabase = Database::getTemporary(cTrans_);
			} else {
				m_pDatabase = Database::get(getDatabaseID(), cTrans_);
			}
		}
	}
	return m_pDatabase;
}

//	FUNCTION public
//	Schema::Object::getDatabase -- オブジェクトアが存在するデータベースを表すクラスを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database* pDatabase_
//			設定するデータベースオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Object::
setDatabase(Database* pDatabase_)
{
	; _SYDNEY_ASSERT(pDatabase_);
	if (!m_pDatabase) {
		// クリティカルセクションの中でもう一度調べる
		AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
		if (!m_pDatabase) {
			setDatabaseID(pDatabase_->getID());
			m_pDatabase = pDatabase_;
		}
	}
}

//	FUNCTION public
//	Schema::Object::serialize -- 
//		スキーマオブジェクトを表すクラスのシリアライザー
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive&			archiver
//			シリアル化先(または元)のアーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Object::
serialize(ModArchive& archiver)
{
	if (archiver.isStore()) {

		; _SYDNEY_ASSERT(getScope() != Scope::Permanent
						 || getStatus() == Status::Persistent);

		// ファイル上のオブジェクトID

		bool hasFileID = (m_pFileObjectID)?1:0;
		archiver << hasFileID;
		if (hasFileID) archiver << *m_pFileObjectID;

		// スキーマオブジェクト ID
		archiver << _id;

		// 親のスキーマオブジェクト ID
		archiver << _parent;

		// オブジェクトの名前
		archiver << _name;

		// オブジェクトの種別
		{
		int tmp = _category;
		archiver << tmp;
		}

		// スコープ
		{
		int tmp = _scope;
		archiver << tmp;
		}
		// データベースID
		archiver << m_iDatabaseID;

		// 状態
		{
		int tmp = m_eStatus;
		archiver << tmp;
		}

		// タイムスタンプ
		archiver << m_iTimestamp;
		
	} else {

		// メンバーをすべて初期化しておく
		clear();

		// ファイル上のオブジェクトID

		bool hasFileID;
		archiver >> hasFileID;
		if (hasFileID) {
			LogicalFile::ObjectID cID;
			archiver >> cID;
			m_pFileObjectID = new LogicalFile::ObjectID(cID);
		}

		// スキーマオブジェクト ID
		archiver >> _id;

		// 親のスキーマオブジェクト ID
		archiver >> _parent;

		// オブジェクトの名前
		archiver >> _name;

		// オブジェクトの種別
		{
		int tmp;
		archiver >> tmp;
		_category = static_cast<Category::Value>(tmp);
		}

		// スコープ
		{
		int tmp;
		archiver >> tmp;
		_scope = static_cast<Scope::Value>(tmp);
		}
		// データベースID
		archiver >> m_iDatabaseID;

		// 状態
		{
		int tmp;
		archiver >> tmp;
		m_eStatus = static_cast<Status::Value>(tmp);
		}

		// タイムスタンプ
		archiver >> m_iTimestamp;
	}
}

////////////////////////////////////////////////////////////
// メタデータベースのための定義
////////////////////////////////////////////////////////////

namespace
{
#define _DEFINE0(_type_) \
	Meta::Definition<Object>(Meta::MemberType::_type_)
#define _DEFINE2(_type_, _get_, _set_) \
	Meta::Definition<Object>(Meta::MemberType::_type_, &Object::_get_, &Object::_set_)

	// 他のものと異なり、この配列はMemberTypeが添え字となる
	Meta::Definition<Object> _vecDefinition[] =
	{
		_DEFINE0(Unknown),
		_DEFINE0(FileOID),
		_DEFINE2(ObjectID, getID, setID),
		_DEFINE2(ParentID, getParentID, setParentID),
		_DEFINE0(Name),
		_DEFINE0(Timestamp),
	};
#undef _DEFINE0
#undef _DEFINE2
}

//	FUNCTION protected
//	Schema::Object::pack --
//		スキーマオブジェクトの内容をレコードファイルに格納するために
//		DataArrayDataにする
//
//	NOTES
//		サブクラスで定義する。ここではアサートを起こす。
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Common::DataArrayData*
//			変換されたデータ。呼び出し側でdeleteをする必要がある
//
//	EXCEPTIONS

Common::DataArrayData*
Object::pack() const
{
	// 永続的オブジェクト以外から呼ばれるはずがない
	; _SYDNEY_ASSERT(getScope() == Scope::Permanent);

	int iFieldNumber = getMetaFieldNumber();

	ModAutoPointer<Common::DataArrayData> pResult = new Common::DataArrayData();
	pResult->reserve(iFieldNumber);

	// 更新のときは0番のフィールドを除く
	int i = (getStatus() != Object::Status::Changed) ? 0 : 1;

	for (; i < iFieldNumber; ++i)
		pResult->pushBack(packMetaField(i));

	// BinaryDataは不要なので破棄する
	delete m_pArchiver, m_pArchiver = 0;

	return pResult.release();
}

//	FUNCTION protected
//	Schema::Object::unpack --
//		DataArrayDataをスキーマオブジェクトの内容に反映させる
//
//	NOTES
//		サブクラスで定義する。ここではアサートを起こす。
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Common::DataArrayData& cData_
//			packされたデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Object::
unpack(const Common::DataArrayData& cData_)
{
	using namespace Common;

	int iFieldNumber = getMetaFieldNumber();

	if (cData_.getCount() != iFieldNumber) {
		// 例外送出
		SydErrorMessage
			<< "unpack failed. The number of fields don't match. ("
			<< cData_.getCount() << " != " << iFieldNumber << ")"
			<< ModEndl;
		_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
	}

	// unpackされるのは永続的なオブジェクトに決まっている
	setScope(Scope::Permanent);

	for (int i = 0; i < iFieldNumber; ++i) {
		const Data::Pointer& pElement = cData_.getElement(i);
		; _SYDNEY_ASSERT(pElement->getType() == Meta::getFieldType(getMetaMemberType(i))
						 || pElement->isNull());

		if (!unpackMetaField(pElement.get(), i)) {
			// 例外送出
			SydErrorMessage
				<< "unpack failed. Object=" << getCategory()
				<< " Can't get field ID=" << i
				<< ModEndl;
			_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
		}
	}

	// BinaryDataは不要なので破棄する
	delete m_pArchiver, m_pArchiver = 0;
}

//	FUNCTION public
//	Schema::Object::setFieldInfo -- システム表のフィールド情報をFileIDに設定する
//
//	NOTES
//
//	ARGUMENTS
//		LogicalFile::FileID& cFileID_
//			設定するFileID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Object::
setFieldInfo(LogicalFile::FileID& cFileID_)
{
	int iFieldNumber = getMetaFieldNumber();
	int i = 0;
	for (; i < iFieldNumber; ++i) {
		Meta::MemberType::Value eType = getMetaMemberType(i);

		// 各フィールドの型
		cFileID_.setInteger(_SYDNEY_SCHEMA_FORMAT_KEY(
								FileCommon::FileOption::FieldType::Key, i),
							Meta::getFieldType(eType));
		if (Meta::getFieldType(eType) == Common::DataType::Array) {
			// 配列フィールドの要素の型
			cFileID_.setInteger(_SYDNEY_SCHEMA_FORMAT_KEY(
									  FileCommon::FileOption::ElementType::Key, i),
								Meta::getFieldElementType(eType));
		}
	}

	// オブジェクトを構成するフィールド数
	cFileID_.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
							FileCommon::FileOption::FieldNumber::Key),
						i);
}

//	FUNCTION protected
//	Schema::Object::packMetaField --
//		スキーマオブジェクトの内容をレコードファイルに格納するために
//		DataArrayDataにする
//
//	NOTES
//
//	ARGUMENTS
//		int iMemberID_
//			データベース表のフィールドに対応する数値
//
//	RETURN
//		Common::Data::Pointer
//			変換したデータ
//
//	EXCEPTIONS

// virtual
Common::Data::Pointer
Object::
packMetaField(int iMemberID_) const
{
	Meta::MemberType::Value eType = getMetaMemberType(iMemberID_);
	switch (eType) {
	case Meta::MemberType::FileOID:
		{
			switch (getStatus()) {
			case Status::Created:
			case Status::Mounted:
			case Status::DeleteCanceled:
				// createのときは空のOIDを返す
				return new LogicalFile::ObjectID();
			default:
				return Common::Data::Pointer(getFileObjectID());
			}
		}
	case Meta::MemberType::ObjectID:
	case Meta::MemberType::ParentID:
		{
			return pack((this->*(_vecDefinition[eType].m_funcGet._id))());
		}
	case Meta::MemberType::Name:
		{
			return pack(getName());
		}
	case Meta::MemberType::Timestamp:
		{
			return pack(getTimestamp());
		}
	default:
		// Objectでは処理できない
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

//	FUNCTION protected
//	Schema::Object::unpackMetaField --
//		DataArrayDataをスキーマオブジェクトの内容に反映させる
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data* pData_
//			内容を反映するData
//		int iMemberID_
//			データベース表のフィールドに対応する数値
//		Schema::Meta::MemberType::Value eType_
//			メンバーの型
//
//	RETURN
//		true...正しく変換された
//		false..変換に失敗した
//
//	EXCEPTIONS

// virtual
bool
Object::
unpackMetaField(const Common::Data* pData_, int iMemberID_)
{
	bool bResult = false;

	Meta::MemberType::Value eType = getMetaMemberType(iMemberID_);
	switch (eType) {
	case Meta::MemberType::FileOID:
		{
			if (pData_ && pData_->getType() == Meta::getFieldType(eType)) {
				const LogicalFile::ObjectID* pFileObjectID =
					_SYDNEY_DYNAMIC_CAST(const LogicalFile::ObjectID*, pData_);
				; _SYDNEY_ASSERT(pFileObjectID);

				setFileObjectID(*pFileObjectID);
				bResult = true;
			}
			break;
		}
	case Meta::MemberType::ObjectID:
	case Meta::MemberType::ParentID:
		{
			ID::Value id;
			if (bResult = unpack(pData_, id))
				(this->*(_vecDefinition[eType].m_funcSet._id))(id);
			break;
		}
	case Meta::MemberType::Name:
		{
			ModUnicodeString name;
			if (bResult = unpack(pData_, name))
				setName(name);
			break;
		}
	case Meta::MemberType::Timestamp:
		{
			unsigned int iTimestamp;
			if (bResult = unpack(pData_, iTimestamp))
				(void) setTimestamp(iTimestamp);
			break;
		}
	default:
		// Objectでは処理できない
		break;
	}
	return bResult;
}

//	FUNCTION protected
//	Schema::Object::pack --
//		データをシステム表に書き込む形式にする
//
//	NOTES
//
//	ARGUMENTS
//		変換するデータ
//
//	RETURN
//	変換されたデータ
//
//	EXCEPTIONS

//static
Common::Data::Pointer
Object::
pack(int iValue_)
{
	return new Common::IntegerData(iValue_);
}

//static
Common::Data::Pointer
Object::
pack(unsigned int iValue_)
{
	return new Common::UnsignedIntegerData(iValue_);
}

//static
Common::Data::Pointer
Object::
pack(const ModVector<unsigned int>& vecValue_)
{
	if (ModSize n = vecValue_.getSize()) {
		ModAutoPointer<Common::DataArrayData> pData = new Common::DataArrayData();
		pData->reserve(n);
		for (ModSize i = 0; i < n; ++i) {
			pData->pushBack(pack((vecValue_)[i]));
		}
		return pData.release();
	}
	return Common::NullData::getInstance();
}

//static
Common::Data::Pointer
Object::
pack(const ModUnicodeString& cValue_)
{
	if (cValue_.getLength())
		return new Common::StringData(cValue_);

	return Common::NullData::getInstance();
}

//static
Common::Data::Pointer
Object::
pack(const ModVector<ModUnicodeString>& vecValue_)
{
	if (ModSize iSize = vecValue_.getSize()) {
		ModAutoPointer<Common::DataArrayData> pData = new Common::DataArrayData;
		pData->reserve(iSize);
		for (ModSize i = 0; i < iSize; i++) {
			pData->pushBack(pack(vecValue_[i]));
		}
		return pData.release();
	}
	return Common::NullData::getInstance();
}

//static
Common::Data::Pointer
Object::
pack(Category::Value eCategory_)
{
	return pack(static_cast<int>(eCategory_));
}

//static
Common::Data::Pointer
Object::
pack(AreaCategory::Value eCategory_)
{
	return pack(static_cast<int>(eCategory_));
}

//	FUNCTION protected
//	Schema::Object::unpack --
//		システム表に書き込まれているデータからメンバー値を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data* pElement_
//			変換するデータ
//		<変換先の型>& value_
//			変換した値
//
//	RETURN
//		true .. 変換に成功した
//		false.. 変換に失敗した
//
//	EXCEPTIONS

//static
bool
Object::
unpack(const Common::Data* pElement_, int& iValue_)
{
	if (pElement_
		&& pElement_->getType() == Common::DataType::Integer) {
		iValue_ = _SYDNEY_DYNAMIC_CAST(const Common::IntegerData&, *pElement_).getValue();
		return true;
	}
	return false;
}

//static
bool
Object::
unpack(const Common::Data* pElement_, unsigned int& iValue_)
{
	if (pElement_
		&& pElement_->getType() == Common::DataType::UnsignedInteger) {
		iValue_ = _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&, *pElement_).getValue();
		return true;
	}
	return false;
}

//static
bool
Object::
unpack(const Common::Data* pData_, ModVector<unsigned int>& vecValue_)
{
	if (pData_ && pData_->isNull()) {
		
		// 配列を初期化する
		vecValue_.clear();

		return true;

	} else if (pData_ && pData_->getType() == Common::DataType::Array
		&& pData_->getElementType() == Common::DataType::Data) {
		
		// 配列を初期化しておく
		vecValue_.clear();

		const Common::DataArrayData* pArrayData =
			_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData_);
		; _SYDNEY_ASSERT(pArrayData);

		ModSize n = pArrayData->getCount();
		vecValue_.reserve(n);

		ModSize i = 0;
		for (; i < n; ++i) {
			unsigned int value;
			if (unpack(pArrayData->getElement(i).get(), value))
				vecValue_.pushBack(value);
			else
				break;
		}
		return (i == n);
	}
	return false;
}

//static
bool
Object::
unpack(const Common::Data* pElement_, ModUnicodeString& cValue_)
{
	if (pElement_) {
		if (pElement_->isNull()) {
			cValue_.clear();
			return true;
		} else if (pElement_->getType() == Common::DataType::String) {
			cValue_ = _SYDNEY_DYNAMIC_CAST(const Common::StringData&, *pElement_).getValue();
			return true;
		}
	}
	return false;
}

//static
bool
Object::
unpack(const Common::Data* pData_, ModVector<ModUnicodeString>& vecValue_)
{
	if (pData_ && pData_->isNull()) {
		
		// 配列を初期化する
		vecValue_.clear();

		return true;

	} else if (pData_ && pData_->getType() == Common::DataType::Array
			   && pData_->getElementType() == Common::DataType::Data) {

		const Common::DataArrayData* pArrayData =
			_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData_);
		; _SYDNEY_ASSERT(pArrayData);

		ModSize n = pArrayData->getCount();
		vecValue_.reserve(n);

		ModSize i = 0;
		for (; i < n; ++i) {
			ModUnicodeString path;
			if (unpack(pArrayData->getElement(i).get(), path))
				vecValue_.pushBack(path);
			else
				break;
		}
		return (i == n);
	}
	return false;
}

//static
bool
Object::
unpack(const Common::Data* pData_, Category::Value& eCategory_)
{
	int value;
	if (unpack(pData_, value)) {
		if (value >= 0 && value < Category::ValueNum) {
			eCategory_ = static_cast<Category::Value>(value);
			return true;
		}
	}
	return false;
}

//static
bool
Object::
unpack(const Common::Data* pData_, AreaCategory::Value& eCategory_)
{
	int value;
	if (unpack(pData_, value)) {
		if (value >= 0 && value < AreaCategory::ValueNum) {
			eCategory_ = static_cast<AreaCategory::Value>(value);
			return true;
		}
	}
	return false;
}

//	FUNCTION protected
//	Schema::Object::getArchiver --
//		pack、unpackでBinaryDataを作るのに使うクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Schema::Utility::BinaryData&
//
//	EXCEPTIONS

Utility::BinaryData&
Object::
getArchiver() const
{
	if (!m_pArchiver)
		m_pArchiver = new Utility::BinaryData;
	return *m_pArchiver;
}

//	FUNCTION private
//	Schema::Object::destruct --
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
Object::
destruct()
{
	delete m_pFileObjectID, m_pFileObjectID = 0;
	delete m_pArchiver, m_pArchiver = 0;
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
