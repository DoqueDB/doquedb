// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaContent.cpp --
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#include "Schema/AreaContent.h"
#include "Schema/Area.h"
#include "Schema/Database.h"
#include "Schema/File.h"
#include "Schema/Index.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/Meta.h"
#include "Schema/Parameter.h"
#include "Schema/SystemTable.h"
#include "Schema/Table.h"
#ifdef DEBUG
#include "Schema/Debug.h"
#endif

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"

#include "Exception/MetaDatabaseCorrupted.h"

#include "LogicalFile/ObjectID.h"

#include "FileCommon/FileOption.h"

#include "Trans/Transaction.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::AreaContent::AreaContent --
//		エリア格納関係を表すクラスのデフォルトコンストラクター
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

AreaContent::
AreaContent()
	: Object(Object::Category::AreaContent),
//	  m_iAreaID(Object::ID::Invalid),
	  m_iObjectID(Object::ID::Invalid),
	  m_eObjectCategory(Object::Category::Unknown),
	  m_eAreaCategory(AreaCategory::Default)
{ }

//	FUNCTION public
//	Schema::AreaContent::AreaContent --
//		エリア格納関係を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Area& cArea_
//			対象とするエリア
//		const Schema::Object& cObject_
//			対象とするオブジェクト
//		Schema::AreaCategory::Value eAreaCategory_
//			対象とする格納関係の種別
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

AreaContent::
AreaContent(Area& cArea_, const Object& cObject_, AreaCategory::Value eAreaCategory_)
	: Object(Object::Category::AreaContent, cArea_.getScope(),
			 Object::Status::Unknown,
			 ID::Invalid, cArea_.getID(), cArea_.getDatabaseID()),
// エリアIDは_parentを用いる
//	  m_iAreaID(cArea_.getID()),
	  m_iObjectID(cObject_.getID()),
	  m_eObjectCategory(cObject_.getCategory()),
	  m_eAreaCategory(eAreaCategory_)
{ }

//	FUNCTION public
//	Schema::AreaContent::~AreaContent --
//		エリア格納関係を表すクラスのデストラクター
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

AreaContent::
~AreaContent()
{
	destruct();
}

//	FUNCTION public
//	Schema::AreaContent::clear -- 列を表すクラスのメンバーをすべて初期化する
//
//	NOTES
//		親クラスのメンバーは初期化しない
//		下位オブジェクトのキャッシュからの抹消を行わないので
//		キャッシュに載っているオブジェクトに対してこのメソッドを呼ぶときには
//		あらかじめキャッシュから抹消する処理を行う必要がある
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
AreaContent::
clear()
{
//	m_iAreaID = Object::ID::Invalid;
	m_iObjectID = Object::ID::Invalid;
	m_eObjectCategory = Object::Category::Unknown;
	m_eAreaCategory = AreaCategory::Default;

	destruct();
}

//	FUNCTION public
//		Schema::AreaContent::getNewInstance -- オブジェクトを新たに取得する
//
//	NOTES
//
//	ARGUMENTS
//		const Common::DataArrayData& cData_
//			元になるデータ
//
//	RETURN
//		新規に作成されたオブジェクト
//
//	EXCEPTIONS

// static
AreaContent*
AreaContent::
getNewInstance(const Common::DataArrayData& cData_)
{
	ModAutoPointer<AreaContent> pObject = new AreaContent;
	pObject->unpack(cData_);
	return pObject.release();
}

//	FUNCTION public
//	Schema::AreaContent::create --
//		エリア格納関係を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Area& cArea_
//			対象とするエリア
//		const Schema::Object& cObject_
//			対象とするオブジェクト
//		Schema::AreaCategory::Value eAreaCategory_
//			対象とする格納関係の種別
//		Trans::Transaction& cTrans_
//			操作しようとしているトランザクション記述子
//
//	RETURN
//		生成したエリア格納関係を表すクラス
//
//	EXCEPTIONS
//		なし

// static
AreaContent::Pointer
AreaContent::
create(Area& cArea_, const Object& cObject_, AreaCategory::Value eAreaCategory_,
	   Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	Pointer pContent = new AreaContent(cArea_, cObject_, eAreaCategory_);
	; _SYDNEY_ASSERT(pContent.get());

	// MOUNT中に作成する場合があるのでIDを振る前に
	// Databaseオブジェクトをセットする必要がある
	; _SYDNEY_ASSERT(cArea_.getDatabase(cTrans_));
	pContent->setDatabase(cArea_.getDatabase(cTrans_));

	// IDをふり、状態を変える
	pContent->Object::create(cTrans_);

	// 対象とするエリアにこのクラスを追加する
	// ★注意★
	// エリア格納関係は単独で生成されないので
	// ここで追加してしまってよい
	(void) cArea_.addContent(pContent, cTrans_);

	// 生成されたエリア格納関係のオブジェクトを返す
	return pContent;
}

//	FUNCTION private
//	Schema::AreaContent::destruct --
//		エリア格納関係を表すクラスのデストラクター下位関数
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
AreaContent::
destruct()
{
	;
}

//	FUNCTION public
//	Schema::AreaContent::drop --
//		あるエリアとあるスキーマオブジェクトのエリア格納関係を表す
//		オブジェクトを破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Area& cArea_
//			このエリアに関するオブジェクトに関係する定義を破棄する
//		const Schema::Object& cObject_
//			このオブジェクトに関する定義を破棄する
//		Schema::AreaCategory::Value eAreaCategory_
//			対象とする格納関係の種別
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//		bool bRecovery_ = false
//			リカバリー処理でのDROPか
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
AreaContent::
drop(Area& cArea_, const Object& cObject_,
	 AreaCategory::Value eAreaCategory_, Trans::Transaction& cTrans_,
	 bool bRecovery_ /* = false */)
{
	if (AreaContent* pContent = cArea_.getContent(cObject_.getID(), eAreaCategory_, cTrans_))
		// 削除の処理を行う
		pContent->drop(cTrans_, bRecovery_);
}

//	FUNCTION public
//	Schema::AreaContent::drop -- エリア格納関係のスキーマ定義を破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//		bool bRecovery_ = false
//			リカバリー処理でのDROPか
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AreaContent::drop(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
	// 作成中ならエリアのマップから削除する
	Object::Status::Value eStat = getStatus();
	if ( eStat == Status::Created || eStat == Status::Mounted ) {
		if (Area* pArea = getArea(cTrans_)) {
			pArea->eraseContent(getID());
		}

	} else {
		// 状態を変える
		Object::drop(bRecovery_);
	}
}

//	FUNCTION public
//	Schema::AreaContent::undoDrop --
//		あるエリアとあるスキーマオブジェクトのエリア格納関係を表す
//		オブジェクトの破棄マークをクリアする
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Area& cArea_
//			このエリアに関するオブジェクトに関係する定義を破棄する
//		const Schema::Object& cObject_
//			このオブジェクトに関する定義を破棄する
//		Schema::AreaCategory::Value eAreaCategory_
//			対象とする格納関係の種別
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
AreaContent::
undoDrop(Area& cArea_, const Object& cObject_,
		 AreaCategory::Value eAreaCategory_, Trans::Transaction& cTrans_)
{
	if (AreaContent* pContent = cArea_.getContent(cObject_.getID(), eAreaCategory_, cTrans_))
		// 削除の処理を行う
		pContent->Object::undoDrop();
}

//	FUNCTION public
//	Schema::AreaContent::moveFile -- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const ModVector<ModUnicodeString>& vecPrevPath_
//			移動前のパス
//		bool bUndo_ = false
//			trueの場合エラー処理中なので重ねてのエラー処理はしない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AreaContent::
moveFile(Trans::Transaction& cTrans_,
		 const ModVector<ModUnicodeString>& vecPrevPath_,
		 const ModVector<ModUnicodeString>& vecPostPath_,
		 bool bUndo_, bool bRecovery_, bool bMount_)
{
	switch (getObjectCategory()) {
	case Category::Table:
	{
		// エリア種別がDefaultのもののみを処理する
		// MOUNT時およびREDOの処理ではファイルの移動は不要なのでシーケンスファイルの処理は行わない
		if (getAreaCategory() == AreaCategory::Default) {
			Area* pArea = getArea(cTrans_);
			; _SYDNEY_ASSERT(getDatabase(cTrans_));
			Table* pTable = Table::get(getObjectID(), getDatabase(cTrans_), cTrans_, true /* internal */);
			if (!pTable) {
				// AreaContentの値が不正なので例外
				SydErrorMessage << "Can't move table in area "
							   << pArea->getName()
							   << ". AreaContent may be corrupted."
							   << ModEndl;
				_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
			}

			if (!bMount_ && !bRecovery_) {
				// ファイルは以下のコードで移動するのでタプルIDの
				// シーケンスファイルだけ移動すればよい
				// ★注意★
				// シーケンスファイルは0番目のパスを常に使う
				pTable->moveSequence(cTrans_, vecPrevPath_[0], vecPostPath_[0],
									 pTable->getName(), pTable->getName(), bUndo_, bRecovery_);

			} else {
				// ファイルを移動しない場合は表のパス名キャッシュをクリアしておく
				pTable->clearPath();
			}
		}
		break;
	}
	case Category::File:
	{
		Area* pArea = getArea(cTrans_);
		; _SYDNEY_ASSERT(getDatabase(cTrans_));
		File* pFile = File::get(getObjectID(), getDatabase(cTrans_), cTrans_);
		// MOUNTのリカバリー中はエリアの移動のほうがFileオブジェクトを読み込むよりも
		// 先に行う必要があるため、Fileオブジェクトが見つからない可能性がある
		if (!(bRecovery_ && bMount_) && !pFile) {
			// AreaContentの内容が不正なので例外送出
			SydErrorMessage << "Can't move file in area "
							<< pArea->getName()
							<< ". AreaContent may be corrupted."
							<< ModEndl;
			_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
		}

		if (pFile) {
			pFile->moveArea(cTrans_, vecPrevPath_, vecPostPath_, bUndo_, bRecovery_, bMount_);
		}
		break;
	}
	default:
		break;
	}
}

//	FUNCTION public
//	Schema::AreaContent::moveArea -- エリア割り当て変更を格納関係に反映する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Area* pPrevArea_
//		Schema::Area* pPrevArea_
//			割り当て変更前後のエリアオブジェクト
//		const Schema::Object* pObject_
//			割り当て対象のオブジェクト
//		AreaCategory::Value eCategory_
//			割り当てるエリア種別
//		bool bUndo_ = false
//			trueの場合エラー処理中なので重ねてのエラー処理はしない
//		bool bRecovery_ = false
//			リカバリー処理での移動か
//		bool bRecovery_ = false
//			MOUNT処理での移動か
//
//	RETURN
//		移動した格納関係のオブジェクト
//
//	EXCEPTIONS

// static
AreaContent*
AreaContent::
moveArea(Trans::Transaction& cTrans_,
		 Area* pPrevArea_, Area* pPostArea_,
		 const Object* pObject_, AreaCategory::Value eCategory_,
		 bool bUndo_, bool bRecovery_, bool bMount_)
{
	AreaContent* pContent = 0;

	if (pPrevArea_) {
		// 変更前のエリアが有効な指定であればその対応関係を破棄する

		// AreaContent の取得
		pContent = pPrevArea_->getContent(pObject_->getID(), eCategory_, cTrans_);

		if (pContent) {
			pContent->drop(cTrans_, bRecovery_);

		} else if (!bRecovery_ && !bUndo_) {
			// 割り当てるときに同時に対応関係のクラスも作成しているので
			// 必ずあるはず
			// ただし、Recovery中やUndo中はないかもしれない

			// 格納関係のオブジェクトがないので例外送出
			SydErrorMessage
				<< "AreaContent not found between object = "
				<< pObject_->getName() << " and area = "
				<< pPrevArea_->getName() << ", area category = "
				<< eCategory_ << ModEndl;
			_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
		}

	}

	if (pPostArea_) {
		// 変更後のエリアが有効な指定であればその対応関係を作成する
		// ★注意★
		// 変更前のエリアも有効な場合は上記で求めたpContentを上書きすることになる

		// リカバリー中は変更後のContentがすでにある場合がある
		pContent = pPostArea_->getContent(pObject_->getID(), eCategory_, cTrans_);
		if (!pContent) {
			pContent = AreaContent::create(*pPostArea_, *pObject_, eCategory_, cTrans_).get();

			// 状態は「生成」である
			; _SYDNEY_ASSERT( pContent->getStatus() == Status::Created
							  || pContent->getStatus() == Status::Mounted );
		} else {
			// すでにある場合は破棄マークがついている場合があるので取り消す
			pContent->Object::undoDrop();
		}
	}

	return pContent;
}

//	FUNCTION public
//	Schema::AreaContent::undoMoveArea -- エリア割り当て変更を取り消す
//
//	NOTES
//		moveAreaを実行後、永続化する前に取り消し操作を行うときは
//		本関数を使う
//		永続化した後は逆向きのmoveAreaをbUndo_=trueにして
//		実行することで取り消し操作を行う
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Area* pPrevArea_
//		Schema::Area* pPrevArea_
//			割り当て変更前後のエリアオブジェクト
//		const Schema::Object* pObject_
//			割り当て対象のオブジェクト
//		Schema::AreaCategory::Value eCategory_
//			割り当てるエリア種別
//		Schema::AreaContent* pContent_
//			moveAreaで変更した格納関係
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
AreaContent::
undoMoveArea(Trans::Transaction& cTrans_,
			 Area* pPrevArea_, Area* pPostArea_,
			 const Object* pObject_, AreaCategory::Value eCategory_,
			 AreaContent* pContent_)
{
	switch (pContent_->getStatus()) {
	case Status::Created:
	case Status::Mounted:
	{
		// 格納関係の作成を取り消す
		AreaContent::drop(*pPostArea_, *pObject_, eCategory_, cTrans_);
		break;
	}
	case Status::Deleted:
	case Status::DeletedInRecovery:
	{
		// 削除マークを取り消す
		pContent_->Object::undoDrop();
		break;
	}
	default:
		// 上記以外の場合はこの関数にはこない
		; _SYDNEY_ASSERT(false);
		break;
	}

	// pPrevArea_とpPostArea_が同時に0以外である場合
	// pPrevArea_に対するAreaContentの破棄を取り消す必要がある

	if (pPrevArea_ && pPostArea_) {
		// AreaContent の取得
		AreaContent* pContent = pPrevArea_->getContent(pObject_->getID(), eCategory_, cTrans_);
		// リカバリー中にはAreaContentがない可能性がある
		if (pContent) {
			; _SYDNEY_ASSERT(pContent->getStatus() == Status::Deleted
							 || pContent->getStatus() == Status::DeletedInRecovery);
			pContent->Object::undoDrop();
		}
	}
}

//	FUNCTION public
//	Schema::AreaContent::doBeforePersist -- 永続化する前に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::AreaContent::Pointer& pContent_
//			永続化するエリア格納関係のオブジェクト
//		Schema::Object::Status::Value eStatus_
//			永続化する前のオブジェクトの状態
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
AreaContent::
doBeforePersist(const Pointer& pContent_, Status::Value eStatus_,
				bool bNeedToErase_,
				Trans::Transaction& cTrans_)
{
	// 何もしない
	;
}

//	FUNCTION public
//	Schema::AreaContent::doAfterPersist -- 永続化した後に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::AreaContent::Pointer& pContent_
//			永続化したエリア格納関係のオブジェクト
//		Schema::Object::Status::Value eStatus_
//			永続化する前のオブジェクトの状態
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
AreaContent::
doAfterPersist(const Pointer& pContent_, Status::Value eStatus_,
			   bool bNeedToErase_,
			   Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pContent_.get());

	// deleteされる可能性があるのでここでデータベースIDを取得しておく
	ID::Value dbID = pContent_->getDatabaseID();

	switch (eStatus_) {
	case Status::Created:
	case Status::Mounted:
	case Status::Changed:
	case Status::CreateCanceled:
	case Status::DeleteCanceled:
		break;

	case Status::Deleted:
	case Status::DeletedInRecovery:
	{
		// 変更が削除だったら登録の抹消も行う

		// 状態を「実際に削除された」にする
		
		pContent_->setStatus(Schema::Object::Status::ReallyDeleted);

		// 下位オブジェクトがあればそれを抹消してからdeleteする
		pContent_->reset();

		// エリアの登録から抹消すればdeleteされる
		
		Schema::Area* pArea = pContent_->getArea(cTrans_);
		; _SYDNEY_ASSERT(pArea);
		pArea->eraseContent(pContent_->getID());
		break;
	}
	default:

		// 何もしない

		return;
	}

	// システム表の状態を変える
	SystemTable::unsetStatus(dbID, Object::Category::AreaContent);
}

//static
void
AreaContent::
doAfterLoad(const Pointer& pContent_, Area& cArea_, Trans::Transaction& cTrans_)
{
	// エリアへ読み出したエリア格納関係を表すクラスを追加する
	cArea_.addContent(pContent_, cTrans_);
}

//	FUNCTION public
//	Schema::AreaContent::reset --
//		下位オブジェクトを抹消する
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
AreaContent::
reset()
{
	;
}
void
AreaContent::
reset(Database& cDatabase_)
{
	reset();
}

//	FUNCTION public
//	Schema::AreaContent::getAreaID --
//		対象とするエリアのオブジェクトIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Schema::Object::ID::Invalid以外
//			対象とするエリアのオブジェクトID
//		Schema::Object::ID::Invalid
//			対象とするエリアは存在しない
//
//	EXCEPTIONS
//		なし

Object::ID::Value
AreaContent::
getAreaID() const
{
	return getParentID();
}

//	FUNCTION public
//	Schema::AreaContent::setAreaID --
//		対象とするエリアのオブジェクトIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			設定する対象のエリアのオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
AreaContent::
setAreaID(ID::Value iID_)
{
	setParentID(iID_);
}

//	FUNCTION public
//	Schema::AreaContent::getObjectID --
//		対象とするオブジェクトのオブジェクトIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Schema::Object::ID::Invalid以外
//			対象とするオブジェクトのオブジェクトID
//		Schema::Object::ID::Invalid
//			対象とするオブジェクトは存在しない
//
//	EXCEPTIONS
//		なし

Object::ID::Value
AreaContent::
getObjectID() const
{
	return m_iObjectID;
}

//	FUNCTION public
//	Schema::AreaContent::setObjectID --
//		対象とするオブジェクトのオブジェクトIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value
//			オブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
AreaContent::
setObjectID(ID::Value id_)
{
	m_iObjectID = id_;
}

//	FUNCTION public
//	Schema::AreaContent::getObjectCategory --
//		対象とするオブジェクトの種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Schema::Object::Category::Unknown以外
//			対象とするオブジェクトの種別
//		Schema::Object::Category::Unknown
//			対象とするオブジェクトは存在しない
//
//	EXCEPTIONS
//		なし

Object::Category::Value
AreaContent::
getObjectCategory() const
{
	return m_eObjectCategory;
}

//	FUNCTION public
//	Schema::AreaContent::setObjectCategory --
//		対象とするオブジェクトの種別を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Category::Value
//			設定する種別
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
AreaContent::
setObjectCategory(Object::Category::Value category_)
{
	m_eObjectCategory = category_;
}

//	FUNCTION public
//	Schema::AreaContent::getAreaCategory --
//		対象とする格納関係の表すエリアの種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		対象とする格納関係の種別
//
//	EXCEPTIONS
//		なし

AreaCategory::Value
AreaContent::
getAreaCategory() const
{
	return m_eAreaCategory;
}

//	FUNCTION public
//	Schema::AreaContent::setAreaCategory --
//		対象とする格納関係の表すエリアの種別を設定する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		対象とする格納関係の種別
//
//	EXCEPTIONS
//		なし

void
AreaContent::
setAreaCategory(AreaCategory::Value category_)
{
	m_eAreaCategory = category_;
}

//	FUNCTION public
//	Schema::AreaContent::getArea -- 
//		対象となるエリアのオブジェクトを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0以外
//			対象となるエリアのオブジェクト
//		0
//			対象となるエリアは存在しない
//
//	EXCEPTIONS

Area*
AreaContent::getArea(Trans::Transaction& cTrans_) const
{
	; _SYDNEY_ASSERT(getDatabase(cTrans_));
	return Area::get(getParentID(), getDatabase(cTrans_), cTrans_);
}

//	FUNCTION public
//	Schema::AreaContent::serialize -- 
//		エリア格納関係を表すクラスのシリアライザー
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
AreaContent::
serialize(ModArchive& archiver)
{
	// まず、スキーマオブジェクト固有の情報をシリアル化する

	Object::serialize(archiver);

	if (archiver.isStore()) {

		// 対象オブジェクトのID
		archiver << m_iObjectID;

		// 対象オブジェクトの種別
		{
		int tmp = m_eObjectCategory;
		archiver << tmp;
		}

		// 対象エリアの種別
		{
		int tmp = m_eAreaCategory;
		archiver << tmp;
		}

	} else {

		// メンバーをすべて初期化しておく

		clear();

		// 対象オブジェクトのID
		archiver >> m_iObjectID;

		// 対象オブジェクトの種別
		{
		int tmp;
		archiver >> tmp;
		m_eObjectCategory = static_cast<Category::Value>(tmp);
		}

		// 対象エリアの種別
		{
		int tmp;
		archiver >> tmp;
		m_eAreaCategory = static_cast<AreaCategory::Value>(tmp);
		}
	}
}

//	FUNCTION public
//	Schema::AreaContent::getClassID -- このクラスのクラス ID を得る
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

int
AreaContent::
getClassID() const
{
	return Externalizable::Category::AreaContent +
		Common::Externalizable::SchemaClasses;
}

////////////////////////////////////////////////////////////
// メタデータベースのための定義
////////////////////////////////////////////////////////////

// メタデータベースにおける「エリア格納関係」表の構造は以下のとおり
// create table AreaContent (
//		ID			id,
//		area		id,
//		objID		id,
//		objCategory int,	-- Category::Value
//		areaCategory int,	-- AreaCategory::Value
//		time	timestamp
// )

namespace
{
#define _DEFINE0(_type_) \
	Meta::Definition<AreaContent>(Meta::MemberType::_type_)
#define _DEFINE2(_type_, _get_, _set_) \
	Meta::Definition<AreaContent>(Meta::MemberType::_type_, &AreaContent::_get_, &AreaContent::_set_)

	Meta::Definition<AreaContent> _vecDefinition[] =
	{
		_DEFINE0(FileOID),		// FileOID
		_DEFINE0(ObjectID),		// ID
		_DEFINE2(ID, getAreaID, setAreaID), // AreaID
		_DEFINE2(ID, getObjectID, setObjectID), // ObjectID
		_DEFINE0(Integer),		// ObjectCategory
		_DEFINE0(Integer),		// AreaCategory
		_DEFINE0(Timestamp),	// Timestamp
	};
#undef _DEFINE0
#undef _DEFINE2
}

//	FUNCTION public
//	Schema::AreaContent::getMetaFieldNumber --
//		スキーマオブジェクトを格納するファイルのフィールド数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	フィールドの数
//
//	EXCEPTIONS

int
AreaContent::
getMetaFieldNumber() const
{
	return static_cast<int>(Meta::AreaContent::MemberNum);
}

//	FUNCTION public
//	Schema::AreaContent::getMetaFieldDefinition --
//		スキーマオブジェクトを格納するファイルのフィールド定義を得る
//
//	NOTES
//
//	ARGUMENTS
//		int iMemberID_
//			フィールドのメンバーを識別する番号
//
//	RETURN
//	フィールドの数
//
//	EXCEPTIONS

Meta::MemberType::Value
AreaContent::
getMetaMemberType(int iMemberID_) const
{
	return _vecDefinition[iMemberID_].m_eType;
}

//	FUNCTION public
//	Schema::AreaContent::packMetaField --
//		スキーマオブジェクトの内容をレコードファイルに格納するため
//		DataArrayDataにする
//
//	NOTES
//
//	ARGUMENTS
//		int iMemberID_
//			データベース表のフィールドに対応する数値
//
//	RETURN
//		0以外...正しく変換された
//		0    ...変換に失敗した
//
//	EXCEPTIONS

Common::Data::Pointer
AreaContent::
packMetaField(int iMemberID_) const
{
	Meta::Definition<AreaContent>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Objectの情報を使う
		return Object::packMetaField(iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::ID:
		{
			return pack((this->*(cDef.m_funcGet._id))());
		}
	case Meta::MemberType::Integer:
		{
			switch (iMemberID_) {
			case Meta::AreaContent::ObjectCategory:
				{
					return pack(getObjectCategory());
				}
			case Meta::AreaContent::AreaCategory:
				{
					return pack(getAreaCategory());
				}
			}
			break;
		}
	default:
		// これ以外の型はないはず
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

//	FUNCTION private
//	Schema::AreaContent::unpackMetaField --
//		DataArrayDataをスキーマオブジェクトの内容に反映させる
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data* pData_
//			内容を反映するData
//		int iMemberID_
//			データベース表のフィールドに対応する数値
//
//	RETURN
//		true...正しく変換された
//		false..変換に失敗した
//
//	EXCEPTIONS

bool
AreaContent::
unpackMetaField(const Common::Data* pData_, int iMemberID_)
{
	Meta::Definition<AreaContent>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Objectの情報を使う
		return Object::unpackMetaField(pData_, iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::ID:
		{
			ID::Value id;
			if (unpack(pData_, id)) {
				(this->*(cDef.m_funcSet._id))(id);
				return true;
			}
			break;
		}
	case Meta::MemberType::Integer:
		{
			switch (iMemberID_) {
			case Meta::AreaContent::ObjectCategory:
				{
					Object::Category::Value value;
					if (unpack(pData_, value)) {
						setObjectCategory(value);
						return true;
					}
					break;
				}
			case Meta::AreaContent::AreaCategory:
				{
					AreaCategory::Value value;
					if (unpack(pData_, value)) {
						setAreaCategory(value);
						return true;
					}
					break;
				}
			}
			break;
		}
	default:
		// これ以外の型はないはず
		break;
	}
	return false;
}

//
//	Copyright (c) 2000, 2001, 2002, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
